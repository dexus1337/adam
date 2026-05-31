#include <gtest/gtest.h>
#include "data/connection.hpp"
#include "data/port/port.hpp"
#include "data/port/port-internal.hpp"
#include "data/processor.hpp"
#include "data/format.hpp"
#include "module/module.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "memory/buffer/buffer.hpp"
#include "controller/controller.hpp"
#include "controller/controller-cmd-dispatcher.hpp"
#include "controller/registry.hpp"

using namespace adam::string_hashed_ct_literals;

namespace adam::test
{
    class test_port : public adam::port
    {
    public:
        test_port(const adam::string_hashed& name) : adam::port(name) {}
        
        const adam::string_hashed_ct& get_type_name() const override { static adam::string_hashed_ct type = "test"; return type; }
        direction get_direction() const override { return direction_inout; }


        void worker() override
        {
            while (is_active())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
        }
    };

    class test_processor : public adam::data_processor
    {
    public:
        test_processor(const adam::string_hashed& name, const adam::data_format* in_fmt, const adam::data_format* out_fmt)
            : adam::data_processor(name)
        {
            m_format_input = in_fmt;
            m_format_output = out_fmt;
        }

        bool handle_data(adam::buffer*& buffer) override
        {
            (void)buffer;
            return true;
        }
    };
}

class connection_test : public ::testing::Test
{
protected:
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
    adam::test::test_port in_port("in_port"_ct);
    in_port.start();

    adam::test::test_port out_port_1("out_port_1"_ct);
    out_port_1.start();

    adam::test::test_port out_port_2("out_port_2"_ct);
    out_port_2.start();

    adam::connection conn("conn"_ct);
    conn.ports_input().push_back(&in_port);
    conn.ports_output().push_back(&out_port_1);
    conn.ports_output().push_back(&out_port_2);

    EXPECT_TRUE(conn.check_valid_chain());

    adam::buffer* buf = adam::buffer_manager::get().request_buffer(512);
    buf->set_size(10);

    EXPECT_TRUE(conn.handle_data(buf));

    auto* stats_1 = out_port_1.get_statistic_buffer()->data_as<adam::port::statistic_info>();
    auto* stats_2 = out_port_2.get_statistic_buffer()->data_as<adam::port::statistic_info>();

    EXPECT_EQ(stats_1->total_buffers_handled, 1u);
    EXPECT_EQ(stats_2->total_buffers_handled, 1u);

    buf->release();
    in_port.stop();
    out_port_1.stop();
    out_port_2.stop();
}

TEST_F(connection_test, connection_data_forwarding_with_processor)
{
    adam::data_format formatA("formatA");
    adam::data_format formatB("formatB");

    adam::test::test_port in_port("in_port"_ct);
    in_port.start();

    adam::test::test_processor proc("proc"_ct, &formatA, &formatB);

    adam::test::test_port out_port("out_port"_ct);
    out_port.start();

    adam::connection conn("conn"_ct);
    conn.ports_input().push_back(&in_port);
    conn.processors().push_back(&proc);
    conn.ports_output().push_back(&out_port);

    EXPECT_TRUE(conn.check_valid_chain());

    adam::buffer* buf = adam::buffer_manager::get().request_buffer(512);
    buf->set_size(10);

    EXPECT_TRUE(conn.handle_data(buf));

    auto* stats = out_port.get_statistic_buffer()->data_as<adam::port::statistic_info>();
    EXPECT_EQ(stats->total_buffers_handled, 1u);

    buf->release();
    in_port.stop();
    out_port.stop();
}
