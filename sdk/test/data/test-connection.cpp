#include <gtest/gtest.h>
#include "commander/messages/message-structs.hpp"
#include "data/connection.hpp"
#include "data/port.hpp"
#include "data/port-types/port-internal.hpp"
#include "data/processor.hpp"
#include "data/format.hpp"
#include "module/module.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "memory/buffer/buffer.hpp"
#include "controller/controller.hpp"
#include "controller/controller-cmd-dispatcher.hpp"
#include "controller/registry.hpp"
#include "data/inspector.hpp"
#include "data/parser.hpp"
#include "data/encoder.hpp"
#include <atomic>


using namespace adam::string_hashed_ct_literals;

class mock_parser : public adam::parser
{
public:
    mock_parser() : adam::parser() {}
    bool parse(adam::buffer* buf, adam::buffer*& internal_data) override
    {
        internal_data = buf;
        if (internal_data)
            internal_data->add_ref();
        return true;
    }
};

class counting_mock_parser : public adam::parser
{
public:
    std::atomic<int> parse_count = 0;
    counting_mock_parser() : adam::parser() {}
    bool parse(adam::buffer* buf, adam::buffer*& internal_data) override
    {
        parse_count++;
        internal_data = buf;
        if (internal_data)
            internal_data->add_ref();
        return true;
    }
};

class mock_encoder : public adam::encoder
{
public:
    mock_encoder() : adam::encoder() {}
    bool encode(adam::buffer*& buf, adam::buffer* internal_data) override
    {
        buf = internal_data;
        if (buf)
            buf->add_ref();
        return true;
    }
};


class connection_test : public ::testing::Test
{
protected:
    class test_port : public adam::port
    {
    public:
        test_port(const adam::string_hashed& name) : adam::port(name) { m_b_threaded = false; }
        
        const adam::string_hashed_ct& get_type_name() const override { static adam::string_hashed_ct type = "test"_ct; return type; }
        direction get_direction() const override { return direction_inout; }

        bool read(adam::buffer*& buff) override { (void)buff; return false;}
        bool write(adam::buffer* buff) override { (void)buff; return true;}
        
        void worker() override
        {
            while (is_started())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
        }
    };

    class test_processor : public adam::processor
    {
    public:
        test_processor(const adam::string_hashed& name, const adam::data_format* in_fmt, const adam::data_format* out_fmt)
            : adam::processor(name)
        {
            m_format_input = in_fmt;
            m_format_output = out_fmt;
        }

        const adam::string_hashed_ct& get_type_name() const override { static adam::string_hashed_ct type = "test"; return type; }

        bool handle_data(adam::buffer*& buf) override
        {
            (void)buf;
            return true;
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

TEST_F(connection_test, connection_set_formats)
{
    adam::data_format formatA("formatA");
    adam::data_format formatB("formatB");

    adam::connection conn("conn"_ct);

    // Default formats should be transparent
    EXPECT_EQ(conn.get_input_format()->get_name(), "transparent"_ct);
    EXPECT_EQ(conn.get_output_format()->get_name(), "transparent"_ct);

    // Set formats and check
    conn.set_input_format(&formatA);
    conn.set_output_format(&formatB);

    EXPECT_EQ(conn.get_input_format(), &formatA);
    EXPECT_EQ(conn.get_output_format(), &formatB);
}

TEST_F(connection_test, connection_data_forwarding_multiple_outputs)
{
    test_port in_port("in_port"_ct);
    in_port.start();

    test_port out_port_1("out_port_1"_ct);
    out_port_1.start();

    test_port out_port_2("out_port_2"_ct);
    out_port_2.start();

    adam::connection conn("conn"_ct);
    conn.ports_input().push_back(&in_port);
    conn.ports_output().push_back(&out_port_1);
    conn.ports_output().push_back(&out_port_2);

    EXPECT_TRUE(conn.check_valid_chain());
    EXPECT_TRUE(conn.start());

    adam::buffer* buf = adam::buffer_manager::get().request_buffer(512);
    buf->set_size(10);

    EXPECT_TRUE(conn.handle_data(buf));

    auto* stats_1 = out_port_1.get_state_buffer()->data_as<adam::port::state_buffer_data>();
    auto* stats_2 = out_port_2.get_state_buffer()->data_as<adam::port::state_buffer_data>();

    EXPECT_EQ(stats_1->total_buffers_handled, 1u);
    EXPECT_EQ(stats_2->total_buffers_handled, 1u);

    buf->release();
    in_port.stop();
    out_port_1.stop();
    out_port_2.stop();
}

TEST_F(connection_test, connection_data_forwarding_with_processor)
{
    mock_parser parserA;
    mock_encoder encoderB;
    adam::data_format formatA("formatA", &parserA);
    adam::data_format formatB("formatB", nullptr, &encoderB);

    test_port in_port("in_port"_ct);
    in_port.start();

    test_processor proc("proc"_ct, &formatA, &formatB);

    test_port out_port("out_port"_ct);
    out_port.start();

    adam::connection conn("conn"_ct);
    conn.ports_input().push_back(&in_port);
    conn.processors().push_back(&proc);
    conn.ports_output().push_back(&out_port);
    conn.set_input_format(&formatA);
    conn.set_output_format(&formatB);

    EXPECT_TRUE(conn.check_valid_chain());
    EXPECT_TRUE(conn.start());

    adam::buffer* buf = adam::buffer_manager::get().request_buffer(512);
    buf->set_size(10);

    EXPECT_TRUE(conn.handle_data(buf));

    auto* stats = out_port.get_state_buffer()->data_as<adam::port::state_buffer_data>();
    EXPECT_EQ(stats->total_buffers_handled, 1u);

    buf->release();
    in_port.stop();
    out_port.stop();
}

TEST_F(connection_test, connection_input_inspector_receives_data)
{
    test_port in_port("in_port"_ct);
    in_port.start();

    test_port out_port("out_port"_ct);
    out_port.start();

    adam::connection conn("conn"_ct);
    conn.ports_input().push_back(&in_port);
    conn.ports_output().push_back(&out_port);

    EXPECT_TRUE(conn.check_valid_chain());
    EXPECT_TRUE(conn.start());

    std::atomic<int> callback_count = 0;
    auto inspector = std::make_shared<adam::data_inspector>();
    EXPECT_TRUE(inspector->create("conn"_ct.get_hash() ^ adam::string_hashed_ct("input").get_hash()));
    EXPECT_TRUE(inspector->start_inspecting([&callback_count](adam::buffer*) {
        callback_count++;
    }));

    conn.inspectors_input().push_back(inspector);

    adam::buffer* buf = adam::buffer_manager::get().request_buffer(512);
    buf->set_size(10);

    EXPECT_TRUE(conn.handle_data(buf));

    // Allow inspector thread to process
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    EXPECT_EQ(callback_count.load(), 1);

    buf->release();
    inspector->destroy();
    in_port.stop();
    out_port.stop();
}

TEST_F(connection_test, connection_output_inspector_receives_data)
{
    test_port in_port("in_port"_ct);
    in_port.start();

    test_port out_port("out_port"_ct);
    out_port.start();

    adam::connection conn("conn"_ct);
    conn.ports_input().push_back(&in_port);
    conn.ports_output().push_back(&out_port);

    EXPECT_TRUE(conn.check_valid_chain());
    EXPECT_TRUE(conn.start());

    std::atomic<int> callback_count = 0;
    auto inspector = std::make_shared<adam::data_inspector>();
    EXPECT_TRUE(inspector->create("conn"_ct.get_hash() ^ adam::string_hashed_ct("output").get_hash()));
    EXPECT_TRUE(inspector->start_inspecting([&callback_count](adam::buffer*) {
        callback_count++;
    }));

    conn.inspectors_output().push_back(inspector);

    adam::buffer* buf = adam::buffer_manager::get().request_buffer(512);
    buf->set_size(10);

    EXPECT_TRUE(conn.handle_data(buf));

    // Allow inspector thread to process
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    EXPECT_EQ(callback_count.load(), 1);

    buf->release();
    inspector->destroy();
    in_port.stop();
    out_port.stop();
}

TEST_F(connection_test, connection_format_runtime_change_dataflow)
{
    counting_mock_parser parserA;
    counting_mock_parser parserB;
    mock_encoder encoderA;
    mock_encoder encoderB;

    adam::data_format formatA("formatA", &parserA, &encoderA);
    adam::data_format formatB("formatB", &parserB, &encoderB);

    test_port in_port("in_port"_ct);
    in_port.start();

    test_port out_port("out_port"_ct);
    out_port.start();

    adam::connection conn("conn"_ct);

    // Wire them up
    conn.ports_input().push_back(&in_port);
    in_port.add_as_connection_input(&conn);

    conn.ports_output().push_back(&out_port);
    out_port.add_as_connection_output(&conn);

    // 1. Initial format set to formatA
    conn.set_input_format(&formatA);
    conn.set_output_format(&formatA);

    EXPECT_TRUE(conn.check_valid_chain());
    EXPECT_TRUE(conn.start());

    // Send first data packet
    adam::buffer* buf = adam::buffer_manager::get().request_buffer(512);
    buf->set_size(10);

    // Call handle_data directly on the port to trigger full parsing and forwarding pipeline
    EXPECT_TRUE(in_port.handle_data(buf, adam::data_direction_in));

    // Verify formatA parser was invoked, and formatB parser was not
    EXPECT_EQ(parserA.parse_count.load(), 1);
    EXPECT_EQ(parserB.parse_count.load(), 0);

    // Verify output port received the buffer
    auto* stats = out_port.get_state_buffer()->data_as<adam::port::state_buffer_data>();
    EXPECT_EQ(stats->total_buffers_handled, 1u);

    // 2. Change format at runtime to formatB
    conn.set_input_format(&formatB);
    conn.set_output_format(&formatB);

    // Send second data packet
    EXPECT_TRUE(in_port.handle_data(buf, adam::data_direction_in));

    // Verify formatB parser was invoked now, and formatA parser count stayed at 1
    EXPECT_EQ(parserA.parse_count.load(), 1);
    EXPECT_EQ(parserB.parse_count.load(), 1);

    // Verify output port received the second buffer
    EXPECT_EQ(stats->total_buffers_handled, 2u);

    buf->release();
    in_port.stop();
    out_port.stop();
}

TEST_F(connection_test, connection_same_port_multiple_formats)
{
    counting_mock_parser parserA;
    counting_mock_parser parserB;
    mock_encoder encoderA;
    mock_encoder encoderB;

    adam::data_format formatA("formatA", &parserA, &encoderA);
    adam::data_format formatB("formatB", &parserB, &encoderB);

    test_port in_port("in_port"_ct);
    in_port.start();

    test_port out_port_A("out_port_A"_ct);
    out_port_A.start();

    test_port out_port_B("out_port_B"_ct);
    out_port_B.start();

    adam::connection conn_A("conn_A"_ct);
    adam::connection conn_B("conn_B"_ct);

    // Wire up conn_A (in_port -> out_port_A, formatA)
    conn_A.ports_input().push_back(&in_port);
    in_port.add_as_connection_input(&conn_A);
    conn_A.ports_output().push_back(&out_port_A);
    out_port_A.add_as_connection_output(&conn_A);
    conn_A.set_input_format(&formatA);
    conn_A.set_output_format(&formatA);

    // Wire up conn_B (in_port -> out_port_B, formatB)
    conn_B.ports_input().push_back(&in_port);
    in_port.add_as_connection_input(&conn_B);
    conn_B.ports_output().push_back(&out_port_B);
    out_port_B.add_as_connection_output(&conn_B);
    conn_B.set_input_format(&formatB);
    conn_B.set_output_format(&formatB);

    EXPECT_TRUE(conn_A.check_valid_chain());
    EXPECT_TRUE(conn_B.check_valid_chain());
    EXPECT_TRUE(conn_A.start());
    EXPECT_TRUE(conn_B.start());

    // Send a single buffer
    adam::buffer* buf = adam::buffer_manager::get().request_buffer(512);
    buf->set_size(10);

    EXPECT_TRUE(in_port.handle_data(buf, adam::data_direction_in));

    // Verify both parsers were invoked exactly once
    EXPECT_EQ(parserA.parse_count.load(), 1);
    EXPECT_EQ(parserB.parse_count.load(), 1);

    // Verify both output ports received the data
    auto* stats_A = out_port_A.get_state_buffer()->data_as<adam::port::state_buffer_data>();
    auto* stats_B = out_port_B.get_state_buffer()->data_as<adam::port::state_buffer_data>();
    EXPECT_EQ(stats_A->total_buffers_handled, 1u);
    EXPECT_EQ(stats_B->total_buffers_handled, 1u);

    buf->release();
    in_port.stop();
    out_port_A.stop();
    out_port_B.stop();
}
