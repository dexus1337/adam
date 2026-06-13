#include <gtest/gtest.h>
#include "data/port.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "memory/buffer/buffer.hpp"
#include "configuration/parameters/configuration-parameter-boolean.hpp"

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
            while (is_started())
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
    EXPECT_EQ(stats->total_buffers_handled, 1u);
    EXPECT_EQ(stats->total_bytes_handled, 123u);
    
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
    EXPECT_EQ(stats->total_buffers_handled, 0u);
    EXPECT_EQ(stats->total_bytes_handled, 0u);
    
    buf->release();
}