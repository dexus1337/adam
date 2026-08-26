#include <gtest/gtest.h>
#include "commander/messages/message-structs.hpp"
#include "data/connection.hpp"
#include "data/port.hpp"
#include "data/processor.hpp"
#include "data/format.hpp"
#include "module/module.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "memory/buffer/buffer.hpp"
#include "controller/controller-cmd-dispatcher.hpp"
#include "controller/registry.hpp"
#include "data/inspector.hpp"
#include "data/parser.hpp"
#include "data/encoder.hpp"
#include <atomic>
#include <filesystem>
#include "configuration/parameters/configuration-parameter-boolean.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"


using namespace adam::string_hashed_ct_literals;

static std::atomic<int> g_parse_count_A{0};
static std::atomic<int> g_parse_count_B{0};

class mock_parser : public adam::parser
{
public:
    mock_parser(const adam::string_hashed& name = "mock_parser"_ct) : adam::parser(name) {}
    bool parse(adam::buffer* buf, adam::buffer*& internal_data) override
    {
        internal_data = buf;
        if (internal_data)
            internal_data->add_ref();
        return true;
    }
};

class counting_mock_parser_A : public adam::parser
{
public:
    counting_mock_parser_A(const adam::string_hashed& name = "counting_mock_parser_A"_ct) : adam::parser(name) {}
    bool parse(adam::buffer* buf, adam::buffer*& internal_data) override
    {
        g_parse_count_A++;
        internal_data = buf;
        if (internal_data)
            internal_data->add_ref();
        return true;
    }
};

class counting_mock_parser_B : public adam::parser
{
public:
    counting_mock_parser_B(const adam::string_hashed& name = "counting_mock_parser_B"_ct) : adam::parser(name) {}
    bool parse(adam::buffer* buf, adam::buffer*& internal_data) override
    {
        g_parse_count_B++;
        internal_data = buf;
        if (internal_data)
            internal_data->add_ref();
        return true;
    }
};

class mock_encoder : public adam::encoder
{
public:
    mock_encoder(const adam::string_hashed& name = "mock_encoder"_ct) : adam::encoder(name) {}
    bool encode(adam::buffer*& buf, adam::buffer* internal_data) override
    {
        buf = internal_data;
        if (buf)
            buf->add_ref();
        return true;
    }
};

static adam::default_factory<adam::parser, mock_parser> mock_parser_factory;
static adam::default_factory<adam::parser, counting_mock_parser_A> counting_mock_parser_A_factory;
static adam::default_factory<adam::parser, counting_mock_parser_B> counting_mock_parser_B_factory;
static adam::default_factory<adam::encoder, mock_encoder> mock_encoder_factory;

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
            while (is_running())
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
        g_parse_count_A = 0;
        g_parse_count_B = 0;
        adam::buffer_manager::get().initialize();
    }

    void TearDown() override
    {
        adam::buffer_manager::get().destroy();
    }
};

TEST_F(connection_test, connection_set_formats)
{
    adam::data_format formatA("formatA"_ct, &mock_parser_factory, &mock_encoder_factory);
    adam::data_format formatB("formatB"_ct, &mock_parser_factory, &mock_encoder_factory);

    adam::connection conn("conn"_ct);

    // Default formats should be transparent
    EXPECT_EQ(conn.get_input_format()->get_name(), "transparent"_ct);
    EXPECT_EQ(conn.get_output_format()->get_name(), "transparent"_ct);
    EXPECT_EQ(conn.get_parser(), nullptr);
    EXPECT_EQ(conn.get_encoder(), nullptr);

    // Set formats and check
    conn.set_input_format(&formatA);
    conn.set_output_format(&formatB);

    EXPECT_EQ(conn.get_input_format(), &formatA);
    EXPECT_EQ(conn.get_output_format(), &formatB);
    EXPECT_NE(conn.get_parser(), nullptr);
    EXPECT_NE(conn.get_encoder(), nullptr);
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

    EXPECT_EQ(stats_1->total_buffers_recieved, 1u);
    EXPECT_EQ(stats_1->total_buffers_forwarded, 1u);
    EXPECT_EQ(stats_2->total_buffers_recieved, 1u);
    EXPECT_EQ(stats_2->total_buffers_forwarded, 1u);

    buf->release();
    in_port.stop();
    out_port_1.stop();
    out_port_2.stop();
}

TEST_F(connection_test, connection_data_forwarding_with_processor)
{
    adam::data_format formatA("formatA"_ct, &mock_parser_factory);
    adam::data_format formatB("formatB"_ct, nullptr, &mock_encoder_factory);

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
    EXPECT_EQ(stats->total_buffers_recieved, 1u);
    EXPECT_EQ(stats->total_buffers_forwarded, 1u);

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

TEST_F(connection_test, connection_input_inspector_receives_cloned_buffer_isolated_from_processors)
{
    class mutating_filter_processor : public adam::processor
    {
    public:
        mutating_filter_processor(const adam::string_hashed& name) : adam::processor(name) {}
        const adam::string_hashed_ct& get_type_name() const override { static adam::string_hashed_ct type = "mutating_filter"_ct; return type; }

        bool handle_data(adam::buffer*& buf) override
        {
            if (!buf) return false;
            // Mutate in-place to simulate in-place filtering
            std::memset(buf->begin(), 0xFF, buf->get_size());
            // Filter out / drop
            return false;
        }
    };

    test_port in_port("in_port"_ct);
    in_port.start();

    test_port out_port("out_port"_ct);
    out_port.start();

    mutating_filter_processor proc("mut_filter"_ct);

    adam::connection conn("conn"_ct);
    conn.ports_input().push_back(&in_port);
    conn.processors().push_back(&proc);
    conn.ports_output().push_back(&out_port);

    EXPECT_TRUE(conn.check_valid_chain());
    EXPECT_TRUE(conn.start());

    std::atomic<int> callback_count = 0;
    std::vector<uint8_t> captured_data;
    auto inspector = std::make_shared<adam::data_inspector>();
    EXPECT_TRUE(inspector->create("conn"_ct.get_hash() ^ adam::string_hashed_ct("input").get_hash()));
    EXPECT_TRUE(inspector->start_inspecting([&callback_count, &captured_data](adam::buffer* inspected_buf)
    {
        if (inspected_buf)
        {
            const auto* data = inspected_buf->get_begin_as<uint8_t>();
            captured_data.assign(data, data + inspected_buf->get_size());
        }
        callback_count++;
    }));

    conn.inspectors_input().push_back(inspector);

    adam::buffer* buf = adam::buffer_manager::get().request_buffer(512);
    uint8_t initial_bytes[] = { 0x11, 0x22, 0x33, 0x44 };
    std::memcpy(buf->begin(), initial_bytes, sizeof(initial_bytes));
    buf->set_size(sizeof(initial_bytes));

    // Connection should return false because processor dropped the frame
    EXPECT_FALSE(conn.handle_data(buf));

    // Allow inspector thread to process
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    EXPECT_EQ(callback_count.load(), 1);
    ASSERT_EQ(captured_data.size(), sizeof(initial_bytes));
    // Verify inspector received the original unmutated data (not 0xFF)
    EXPECT_EQ(std::memcmp(captured_data.data(), initial_bytes, sizeof(initial_bytes)), 0);

    // Output port must not have received anything
    auto* stats = out_port.get_state_buffer()->data_as<adam::port::state_buffer_data>();
    EXPECT_EQ(stats->total_buffers_recieved, 0u);

    buf->release();
    inspector->destroy();
    in_port.stop();
    out_port.stop();
}

TEST_F(connection_test, connection_format_runtime_change_dataflow)
{
    adam::data_format formatA("formatA"_ct, &counting_mock_parser_A_factory, &mock_encoder_factory);
    adam::data_format formatB("formatB"_ct, &counting_mock_parser_B_factory, &mock_encoder_factory);

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
    EXPECT_EQ(g_parse_count_A.load(), 1);
    EXPECT_EQ(g_parse_count_B.load(), 0);

    // Verify output port received the buffer
    auto* stats = out_port.get_state_buffer()->data_as<adam::port::state_buffer_data>();
    EXPECT_EQ(stats->total_buffers_recieved, 1u);
    EXPECT_EQ(stats->total_buffers_forwarded, 1u);

    // 2. Change format at runtime to formatB
    conn.set_input_format(&formatB);
    conn.set_output_format(&formatB);

    // Send second data packet
    EXPECT_TRUE(in_port.handle_data(buf, adam::data_direction_in));

    // Verify formatB parser was invoked now, and formatA parser count stayed at 1
    EXPECT_EQ(g_parse_count_A.load(), 1);
    EXPECT_EQ(g_parse_count_B.load(), 1);

    // Verify output port received the second buffer
    EXPECT_EQ(stats->total_buffers_recieved, 2u);
    EXPECT_EQ(stats->total_buffers_forwarded, 2u);

    buf->release();
    in_port.stop();
    out_port.stop();
}

TEST_F(connection_test, connection_same_port_multiple_formats)
{
    adam::data_format formatA("formatA"_ct, &counting_mock_parser_A_factory, &mock_encoder_factory);
    adam::data_format formatB("formatB"_ct, &counting_mock_parser_B_factory, &mock_encoder_factory);

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
    EXPECT_EQ(g_parse_count_A.load(), 1);
    EXPECT_EQ(g_parse_count_B.load(), 1);

    // Verify both output ports received the data
    auto* stats_A = out_port_A.get_state_buffer()->data_as<adam::port::state_buffer_data>();
    auto* stats_B = out_port_B.get_state_buffer()->data_as<adam::port::state_buffer_data>();
    EXPECT_EQ(stats_A->total_buffers_recieved, 1u);
    EXPECT_EQ(stats_A->total_buffers_forwarded, 1u);
    EXPECT_EQ(stats_B->total_buffers_recieved, 1u);
    EXPECT_EQ(stats_B->total_buffers_forwarded, 1u);

    buf->release();
    in_port.stop();
    out_port_A.stop();
    out_port_B.stop();
}

TEST_F(connection_test, connection_format_parameter_wiping_on_format_change)
{
    // Define a parser with custom user parameters
    class param_mock_parser : public adam::parser
    {
    public:
        static const adam::configuration_parameter_list& get_custom_parameters()
        {
            static const adam::configuration_parameter_list params = []()
            {
                adam::configuration_parameter_list p;
                auto user_params = std::make_unique<adam::configuration_parameter_list_sorted>("user_parameters"_ct);
                user_params->add(std::make_unique<adam::configuration_parameter_boolean>("custom_flag"_ct, true));
                p.add(std::move(user_params));
                return p;
            }();
            return params;
        }

        param_mock_parser(const adam::string_hashed& name = "param_mock_parser"_ct)
            : adam::parser(name, get_custom_parameters())
        {
        }

        bool parse(adam::buffer* buf, adam::buffer*& internal_data) override
        {
            internal_data = buf;
            return true;
        }
    };

    static adam::default_factory<adam::parser, param_mock_parser> param_mock_parser_factory;
    adam::data_format format_with_params("format_with_params"_ct, &param_mock_parser_factory);
    adam::data_format format_no_params("format_no_params"_ct, &mock_parser_factory);

    adam::connection conn("conn_param_test"_ct);

    // Initial: transparent (no format params)
    EXPECT_EQ(conn.get_parameters().get("input_format_user_parameters"_ct), nullptr);

    // Switch to format_with_params
    conn.set_input_format(&format_with_params);
    auto* user_params = conn.get_parameters().get("input_format_user_parameters"_ct);
    ASSERT_NE(user_params, nullptr);
    auto* list = dynamic_cast<adam::configuration_parameter_list*>(user_params);
    ASSERT_NE(list, nullptr);
    EXPECT_NE(list->get("custom_flag"_ct), nullptr);

    // Switch to format_no_params -> old parameters must be wiped!
    conn.set_input_format(&format_no_params);
    EXPECT_EQ(conn.get_parameters().get("input_format_user_parameters"_ct), nullptr);
}

TEST_F(connection_test, connection_format_parameter_isolation_between_connections)
{
    class param_mock_parser : public adam::parser
    {
    public:
        static const adam::configuration_parameter_list& get_custom_parameters()
        {
            static const adam::configuration_parameter_list params = []()
            {
                adam::configuration_parameter_list p;
                auto user_params = std::make_unique<adam::configuration_parameter_list_sorted>("user_parameters"_ct);
                user_params->add(std::make_unique<adam::configuration_parameter_boolean>("custom_flag"_ct, true));
                p.add(std::move(user_params));
                return p;
            }();
            return params;
        }

        param_mock_parser(const adam::string_hashed& name = "param_mock_parser"_ct)
            : adam::parser(name, get_custom_parameters())
        {
        }

        bool parse(adam::buffer* buf, adam::buffer*& internal_data) override
        {
            internal_data = buf;
            return true;
        }
    };

    static adam::default_factory<adam::parser, param_mock_parser> param_mock_parser_factory;
    adam::data_format format_with_params("format_with_params"_ct, &param_mock_parser_factory);

    adam::connection conn_A("conn_A"_ct);
    adam::connection conn_B("conn_B"_ct);

    conn_A.set_input_format(&format_with_params);
    conn_B.set_input_format(&format_with_params);

    auto* params_A = conn_A.get_parameter<adam::configuration_parameter_list_sorted>("input_format_user_parameters"_ct);
    auto* params_B = conn_B.get_parameter<adam::configuration_parameter_list_sorted>("input_format_user_parameters"_ct);

    ASSERT_NE(params_A, nullptr);
    ASSERT_NE(params_B, nullptr);
    ASSERT_NE(params_A, params_B);

    auto* flag_A = params_A->get<adam::configuration_parameter_boolean>("custom_flag"_ct);
    auto* flag_B = params_B->get<adam::configuration_parameter_boolean>("custom_flag"_ct);

    ASSERT_NE(flag_A, nullptr);
    ASSERT_NE(flag_B, nullptr);

    // Modify conn_A's parameter value
    flag_A->set_value(false);

    // conn_B's parameter must remain unchanged
    EXPECT_FALSE(flag_A->get_value());
    EXPECT_TRUE(flag_B->get_value());
}

TEST_F(connection_test, connection_output_format_parameter_wiping_on_format_change)
{
    class param_mock_encoder : public adam::encoder
    {
    public:
        static const adam::configuration_parameter_list& get_custom_parameters()
        {
            static const adam::configuration_parameter_list params = []()
            {
                adam::configuration_parameter_list p;
                auto user_params = std::make_unique<adam::configuration_parameter_list_sorted>("user_parameters"_ct);
                user_params->add(std::make_unique<adam::configuration_parameter_integer>("custom_int"_ct, 42));
                p.add(std::move(user_params));
                return p;
            }();
            return params;
        }

        param_mock_encoder(const adam::string_hashed& name = "param_mock_encoder"_ct)
            : adam::encoder(name, get_custom_parameters())
        {
        }

        bool encode(adam::buffer*& buf, adam::buffer* internal_data) override
        {
            buf = internal_data;
            if (buf)
            {
                buf->add_ref();
            }
            return true;
        }
    };

    static adam::default_factory<adam::encoder, param_mock_encoder> param_mock_encoder_factory;
    adam::data_format format_with_enc_params("format_with_enc_params"_ct, nullptr, &param_mock_encoder_factory);
    adam::data_format format_no_params("format_no_params"_ct, &mock_parser_factory);

    adam::connection conn("conn_enc_param_test"_ct);

    // Initial: transparent
    EXPECT_EQ(conn.get_parameters().get("output_format_user_parameters"_ct), nullptr);

    // Set output format with encoder user parameters
    conn.set_output_format(&format_with_enc_params);
    auto* user_params = conn.get_parameters().get("output_format_user_parameters"_ct);
    ASSERT_NE(user_params, nullptr);
    auto* list = dynamic_cast<adam::configuration_parameter_list*>(user_params);
    ASSERT_NE(list, nullptr);
    EXPECT_NE(list->get("custom_int"_ct), nullptr);

    // Reset to format without encoder -> wiped
    conn.set_output_format(&format_no_params);
    EXPECT_EQ(conn.get_parameters().get("output_format_user_parameters"_ct), nullptr);

    // Set again and reset to nullptr -> wiped
    conn.set_output_format(&format_with_enc_params);
    EXPECT_NE(conn.get_parameters().get("output_format_user_parameters"_ct), nullptr);
    conn.set_output_format(nullptr);
    EXPECT_EQ(conn.get_parameters().get("output_format_user_parameters"_ct), nullptr);
}

TEST_F(connection_test, connection_format_parameter_persistence)
{
    class param_mock_parser : public adam::parser
    {
    public:
        static const adam::configuration_parameter_list& get_custom_parameters()
        {
            static const adam::configuration_parameter_list params = []()
            {
                adam::configuration_parameter_list p;
                auto user_params = std::make_unique<adam::configuration_parameter_list_sorted>("user_parameters"_ct);
                user_params->add(std::make_unique<adam::configuration_parameter_boolean>("custom_flag"_ct, true));
                p.add(std::move(user_params));
                return p;
            }();
            return params;
        }

        param_mock_parser(const adam::string_hashed& name = "param_mock_parser"_ct)
            : adam::parser(name, get_custom_parameters())
        {
        }

        bool parse(adam::buffer* buf, adam::buffer*& internal_data) override
        {
            internal_data = buf;
            return true;
        }
    };

    class param_mock_encoder : public adam::encoder
    {
    public:
        static const adam::configuration_parameter_list& get_custom_parameters()
        {
            static const adam::configuration_parameter_list params = []()
            {
                adam::configuration_parameter_list p;
                auto user_params = std::make_unique<adam::configuration_parameter_list_sorted>("user_parameters"_ct);
                user_params->add(std::make_unique<adam::configuration_parameter_integer>("custom_int"_ct, 42));
                p.add(std::move(user_params));
                return p;
            }();
            return params;
        }

        param_mock_encoder(const adam::string_hashed& name = "param_mock_encoder"_ct)
            : adam::encoder(name, get_custom_parameters())
        {
        }

        bool encode(adam::buffer*& buf, adam::buffer* internal_data) override
        {
            buf = internal_data;
            if (buf)
            {
                buf->add_ref();
            }
            return true;
        }
    };

    static adam::default_factory<adam::parser, param_mock_parser> param_mock_parser_factory;
    static adam::default_factory<adam::encoder, param_mock_encoder> param_mock_encoder_factory;
    adam::data_format format_with_params("format_with_params"_ct, &param_mock_parser_factory, &param_mock_encoder_factory);

    static constexpr const char* config_file_path = "test_connection_param_persistence.bin";
    std::filesystem::remove(config_file_path);

    // 1. Configure conn_1 and modify parameters
    {
        adam::connection conn_1("conn_persist_test"_ct);
        conn_1.set_input_format(&format_with_params);
        conn_1.set_output_format(&format_with_params);

        auto* in_params = conn_1.get_parameter<adam::configuration_parameter_list_sorted>("input_format_user_parameters"_ct);
        auto* out_params = conn_1.get_parameter<adam::configuration_parameter_list_sorted>("output_format_user_parameters"_ct);
        ASSERT_NE(in_params, nullptr);
        ASSERT_NE(out_params, nullptr);

        auto* flag = in_params->get<adam::configuration_parameter_boolean>("custom_flag"_ct);
        auto* integer_val = out_params->get<adam::configuration_parameter_integer>("custom_int"_ct);
        ASSERT_NE(flag, nullptr);
        ASSERT_NE(integer_val, nullptr);

        flag->set_value(false);
        integer_val->set_value(999);

        EXPECT_TRUE(conn_1.save(config_file_path));
    }

    // 2. Load into conn_2 and verify values are restored
    {
        adam::connection conn_2("conn_persist_test"_ct);
        conn_2.set_input_format(&format_with_params);
        conn_2.set_output_format(&format_with_params);

        EXPECT_TRUE(conn_2.load(config_file_path));

        auto* in_params = conn_2.get_parameter<adam::configuration_parameter_list_sorted>("input_format_user_parameters"_ct);
        auto* out_params = conn_2.get_parameter<adam::configuration_parameter_list_sorted>("output_format_user_parameters"_ct);
        ASSERT_NE(in_params, nullptr);
        ASSERT_NE(out_params, nullptr);

        auto* flag = in_params->get<adam::configuration_parameter_boolean>("custom_flag"_ct);
        auto* integer_val = out_params->get<adam::configuration_parameter_integer>("custom_int"_ct);
        ASSERT_NE(flag, nullptr);
        ASSERT_NE(integer_val, nullptr);

        EXPECT_FALSE(flag->get_value());
        EXPECT_EQ(integer_val->get_value(), 999);
    }

    // 3. Set formats first then load into conn_3 (simulating registry load flow)
    {
        adam::connection conn_3("conn_persist_test"_ct);
        conn_3.set_input_format(&format_with_params);
        conn_3.set_output_format(&format_with_params);

        EXPECT_TRUE(conn_3.load(config_file_path));

        if (auto* parser = conn_3.get_parser())
        {
            if (auto* user_params = conn_3.get_parameter<adam::configuration_parameter_list_sorted>("input_format_user_parameters"_ct))
            {
                if (auto* parser_params = parser->get_parameter<adam::configuration_parameter_list_sorted>("user_parameters"_ct))
                    parser_params->copy_values_from(user_params);
            }
        }

        if (auto* encoder = conn_3.get_encoder())
        {
            if (auto* user_params = conn_3.get_parameter<adam::configuration_parameter_list_sorted>("output_format_user_parameters"_ct))
            {
                if (auto* encoder_params = encoder->get_parameter<adam::configuration_parameter_list_sorted>("user_parameters"_ct))
                    encoder_params->copy_values_from(user_params);
            }
        }

        auto* in_params = conn_3.get_parameter<adam::configuration_parameter_list_sorted>("input_format_user_parameters"_ct);
        auto* out_params = conn_3.get_parameter<adam::configuration_parameter_list_sorted>("output_format_user_parameters"_ct);
        ASSERT_NE(in_params, nullptr);
        ASSERT_NE(out_params, nullptr);

        auto* flag = in_params->get<adam::configuration_parameter_boolean>("custom_flag"_ct);
        auto* integer_val = out_params->get<adam::configuration_parameter_integer>("custom_int"_ct);
        ASSERT_NE(flag, nullptr);
        ASSERT_NE(integer_val, nullptr);

        EXPECT_FALSE(flag->get_value());
        EXPECT_EQ(integer_val->get_value(), 999);

        ASSERT_NE(conn_3.get_parser(), nullptr);
        ASSERT_NE(conn_3.get_encoder(), nullptr);
        auto* parser_user_params = conn_3.get_parser()->get_parameter<adam::configuration_parameter_list_sorted>("user_parameters"_ct);
        auto* encoder_user_params = conn_3.get_encoder()->get_parameter<adam::configuration_parameter_list_sorted>("user_parameters"_ct);
        ASSERT_NE(parser_user_params, nullptr);
        ASSERT_NE(encoder_user_params, nullptr);
        auto* parser_flag = parser_user_params->get<adam::configuration_parameter_boolean>("custom_flag"_ct);
        auto* encoder_int = encoder_user_params->get<adam::configuration_parameter_integer>("custom_int"_ct);
        ASSERT_NE(parser_flag, nullptr);
        ASSERT_NE(encoder_int, nullptr);
        EXPECT_FALSE(parser_flag->get_value());
        EXPECT_EQ(encoder_int->get_value(), 999);
    }

    std::filesystem::remove(config_file_path);
}

TEST_F(connection_test, connection_chain_validation_topologies)
{
    test_port in_port("in_port"_ct);
    test_port out_port("out_port"_ct);

    adam::data_format format_A1("formatA"_ct);
    adam::data_format format_A2("formatA"_ct);
    adam::data_format format_B("formatB"_ct);
    adam::data_format format_C("formatC"_ct);

    test_processor conv_AB("conv_AB"_ct, &format_A1, &format_B);
    test_processor conv_BC("conv_BC"_ct, &format_B, &format_C);
    test_processor filter_A("filter_A"_ct, &format_A2, &format_A1);

    adam::connection conn("chain_test_conn"_ct);

    // 1. Missing input or output ports -> invalid
    EXPECT_FALSE(conn.check_valid_chain());

    conn.ports_input().push_back(&in_port);
    EXPECT_FALSE(conn.check_valid_chain());

    conn.ports_output().push_back(&out_port);

    // 2. Default transparent -> input and output match -> valid
    EXPECT_TRUE(conn.check_valid_chain());

    // 3. Direct matching formats across distinct instances with same name -> valid
    conn.set_input_format(&format_A1);
    conn.set_output_format(&format_A2);
    EXPECT_TRUE(conn.check_valid_chain());

    // 4. Format mismatch with no processor -> invalid
    conn.set_output_format(&format_B);
    EXPECT_FALSE(conn.check_valid_chain());

    // 5. With converter A -> B -> valid
    conn.processors().push_back(&conv_AB);
    EXPECT_TRUE(conn.check_valid_chain());

    // 6. With chained converters A -> B and B -> C, output C -> valid
    conn.processors().push_back(&conv_BC);
    conn.set_output_format(&format_C);
    EXPECT_TRUE(conn.check_valid_chain());

    // 7. Output expected is B but chain produces C -> invalid
    conn.set_output_format(&format_B);
    EXPECT_FALSE(conn.check_valid_chain());

    // 8. With filter A -> A prepended before converter A -> B -> valid
    conn.processors().clear();
    conn.processors().push_back(&filter_A);
    conn.processors().push_back(&conv_AB);
    conn.set_input_format(&format_A1);
    conn.set_output_format(&format_B);
    EXPECT_TRUE(conn.check_valid_chain());
}


