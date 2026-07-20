#include <gtest/gtest.h>
#include "data/port.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "memory/buffer/buffer.hpp"
#include "configuration/parameters/configuration-parameter-boolean.hpp"
#include "data/parser.hpp"
#include "data/connection.hpp"

#include <thread>
#include <chrono>
#include <atomic>

using namespace adam::string_hashed_ct_literals;

class port_test : public ::testing::Test
{
protected:
    class test_port : public adam::port
    {
    public:
        test_port(const adam::string_hashed& name) : adam::port(name) {}
        
        const adam::string_hashed_ct& get_type_name() const override { static adam::string_hashed_ct type = "test"_ct; return type; }
        direction get_direction() const override { return direction_inout; }
        
        void set_threaded(bool threaded) { m_b_threaded = threaded; }
        bool get_threaded() const { return m_b_threaded; }

        bool read(adam::buffer*& buff) override { (void)buff; return false;}
        bool write(adam::buffer* buff) override { (void)buff; return false;}
        
        std::atomic<int> worker_loops{0};
        
    protected:
        void worker() override
        {
            set_state(state_running);
            while (is_running())
            {
                worker_loops++;
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
        }
    };

    void SetUp() override
    {
        adam::buffer_manager::get().initialize();
    }

    void TearDown() override
    {
        adam::buffer_manager::get().destroy();
    }
};

/** @brief Tests the base parameters and memory states directly after the port is constructed. */
TEST_F(port_test, initial_state)
{
    test_port p("test_port"_ct);
    
    EXPECT_FALSE(p.is_started());
    EXPECT_EQ(p.get_direction(), adam::port::direction_inout);
    EXPECT_NE(p.get_state_buffer(), nullptr);
    EXPECT_EQ(p.get_threaded(), true); // Default value from constructor
}

/** @brief Tests starting and stopping the port without spinning up the dedicated worker thread. */
TEST_F(port_test, start_stop_unthreaded)
{
    test_port p("test_port"_ct);
    p.set_threaded(false);
    
    EXPECT_TRUE(p.start());
    EXPECT_TRUE(p.is_started());
    
    EXPECT_TRUE(p.stop());
    EXPECT_FALSE(p.is_started());
}

/** @brief Tests starting and stopping the port along with its threaded worker loops. */
TEST_F(port_test, start_stop_threaded)
{
    test_port p("test_port"_ct);
    
    EXPECT_TRUE(p.start());
    EXPECT_TRUE(p.is_started());
    
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    
    // The worker thread should have executed at least a few times
    EXPECT_GT(p.worker_loops.load(), 0);
    
    EXPECT_TRUE(p.stop());
    EXPECT_FALSE(p.is_started());
}

/** @brief Tests handling payload buffers when the port is active, and verifies updating base statistics. */
TEST_F(port_test, handle_data_statistics)
{
    test_port p("test_port"_ct);
    p.set_threaded(false);
    p.start();
    
    adam::buffer* buf = adam::buffer_manager::get().request_buffer(1024);
    
    buf->set_size(123);
    p.handle_data(buf, adam::data_direction_in);
    
    auto* stats = p.get_state_buffer()->data_as<adam::port::state_buffer_data>();
    EXPECT_EQ(stats->total_buffers_recieved, 1u);
    EXPECT_EQ(stats->total_bytes_recieved, 123u);
    EXPECT_EQ(stats->total_buffers_forwarded, 1u);
    EXPECT_EQ(stats->total_bytes_forwarded, 123u);
    EXPECT_EQ(stats->total_buffers_discarded, 0u);
    EXPECT_EQ(stats->total_bytes_discarded, 0u);
    
    // Now test a discarded message (test_port::write returns false)
    buf->set_size(45);
    EXPECT_FALSE(p.handle_data(buf, adam::data_direction_out));
    
    EXPECT_EQ(stats->total_buffers_recieved, 2u);
    EXPECT_EQ(stats->total_bytes_recieved, 123u + 45u);
    EXPECT_EQ(stats->total_buffers_forwarded, 1u);
    EXPECT_EQ(stats->total_bytes_forwarded, 123u);
    EXPECT_EQ(stats->total_buffers_discarded, 1u);
    EXPECT_EQ(stats->total_bytes_discarded, 45u);

    buf->release();
    p.stop();
}

/** @brief Tests that inactive ports should automatically deny accepting payloads and prevent updates to stats. */
TEST_F(port_test, inactive_drops_data)
{
    test_port p("test_port"_ct);
    p.set_threaded(false);
    
    // Do not call start(), so is_started() remains false
    
    adam::buffer* buf = adam::buffer_manager::get().request_buffer(1024);
    buf->set_size(123);
    
    EXPECT_FALSE(p.handle_data(buf, adam::data_direction_in));
    
    auto* stats = p.get_state_buffer()->data_as<adam::port::state_buffer_data>();
    EXPECT_EQ(stats->total_buffers_recieved, 0u);
    EXPECT_EQ(stats->total_bytes_recieved, 0u);
    EXPECT_EQ(stats->total_buffers_forwarded, 0u);
    EXPECT_EQ(stats->total_bytes_forwarded, 0u);
    EXPECT_EQ(stats->total_buffers_discarded, 0u);
    EXPECT_EQ(stats->total_bytes_discarded, 0u);
    
    buf->release();
}

class bench_parser : public adam::parser
{
public:
    bool parse(adam::buffer* buf, adam::buffer*& internal_data) override
    {
        internal_data = buf;
        if (internal_data)
            internal_data->add_ref();
        return true;
    }
};

TEST_F(port_test, benchmark_handle_data)
{
    test_port p("bench_port"_ct);
    p.set_threaded(false);
    p.start();

    test_port out_port1("out_port1"_ct);
    test_port out_port2("out_port2"_ct);

    bench_parser parser1;
    bench_parser parser2;
    adam::data_format format1("format1", &parser1);
    adam::data_format format2("format2", &parser2);

    adam::connection conn1("conn1"_ct);
    conn1.ports_input().push_back(&p);
    conn1.ports_output().push_back(&out_port1);
    conn1.set_input_format(&format1);
    conn1.set_output_format(&format1);
    p.add_as_connection_input(&conn1);

    adam::connection conn2("conn2"_ct);
    conn2.ports_input().push_back(&p);
    conn2.ports_output().push_back(&out_port2);
    conn2.set_input_format(&format2);
    conn2.set_output_format(&format2);
    p.add_as_connection_input(&conn2);

    adam::buffer* buf = adam::buffer_manager::get().request_buffer(64);
    buf->set_size(10);

    const int iterations = 1000000;
    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i)
    {
        p.handle_data(buf, adam::data_direction_in);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();

    std::cout << "[          ] Benchmark (handle_data): " << iterations << " iterations." << std::endl;
    std::cout << "[          ] Total duration: " << duration << " us" << std::endl;
    std::cout << "[          ] Average time per iteration: " << (static_cast<double>(duration) / iterations) * 1000.0 << " ns" << std::endl;
    buf->release();
    p.stop();
}

TEST_F(port_test, benchmark_handle_data_three_formats)
{
    test_port p("bench_port_3"_ct);
    p.set_threaded(false);
    p.start();

    test_port out_port1("out_port1"_ct);
    test_port out_port2("out_port2"_ct);
    test_port out_port3("out_port3"_ct);

    bench_parser parser1;
    bench_parser parser2;
    bench_parser parser3;
    adam::data_format format1("format1", &parser1);
    adam::data_format format2("format2", &parser2);
    adam::data_format format3("format3", &parser3);

    adam::connection conn1("conn1"_ct);
    conn1.ports_input().push_back(&p);
    conn1.ports_output().push_back(&out_port1);
    conn1.set_input_format(&format1);
    conn1.set_output_format(&format1);
    p.add_as_connection_input(&conn1);

    adam::connection conn2("conn2"_ct);
    conn2.ports_input().push_back(&p);
    conn2.ports_output().push_back(&out_port2);
    conn2.set_input_format(&format2);
    conn2.set_output_format(&format2);
    p.add_as_connection_input(&conn2);

    adam::connection conn3("conn3"_ct);
    conn3.ports_input().push_back(&p);
    conn3.ports_output().push_back(&out_port3);
    conn3.set_input_format(&format3);
    conn3.set_output_format(&format3);
    p.add_as_connection_input(&conn3);

    adam::buffer* buf = adam::buffer_manager::get().request_buffer(64);
    buf->set_size(10);

    const int iterations = 1000000;
    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i)
    {
        p.handle_data(buf, adam::data_direction_in);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();

    std::cout << "[          ] Benchmark (handle_data - 3 formats): " << iterations << " iterations." << std::endl;
    std::cout << "[          ] Total duration: " << duration << " us" << std::endl;
    std::cout << "[          ] Average time per iteration: " << (static_cast<double>(duration) / iterations) * 1000.0 << " ns" << std::endl;
    buf->release();
    p.stop();
}