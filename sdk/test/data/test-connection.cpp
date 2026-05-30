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
        
        const adam::string_hashed_ct& get_type_name() const override { static adam::string_hashed_ct type = "test"_ct; return type; }
        direction get_direction() const override { return direction_inout; }
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

TEST_F(connection_test, bypass_format_compatibility_forwarding)
{
    adam::data_format formatA("formatA");
    adam::data_format formatB("formatB");

    adam::test::test_port in_port("in_port"_ct);
    in_port.set_data_format(&formatA);
    in_port.start();

    adam::test::test_port out_port_compatible("out_port_comp"_ct);
    out_port_compatible.set_data_format(&formatA);
    out_port_compatible.start();

    adam::test::test_port out_port_incompatible("out_port_incomp"_ct);
    out_port_incompatible.set_data_format(&formatB);
    out_port_incompatible.start();

    adam::connection conn("conn"_ct);
    conn.ports_input().push_back(&in_port);
    conn.ports_output().push_back(&out_port_compatible);
    conn.ports_output().push_back(&out_port_incompatible);

    EXPECT_TRUE(conn.has_valid_chain());

    adam::buffer* buf = adam::buffer_manager::get().request_buffer(512);
    buf->set_size(10);

    EXPECT_TRUE(conn.handle_data(buf, &in_port));

    auto* stats_comp = out_port_compatible.get_statistic_buffer()->data_as<adam::port::statistic_info>();
    auto* stats_incomp = out_port_incompatible.get_statistic_buffer()->data_as<adam::port::statistic_info>();

    EXPECT_EQ(stats_comp->total_buffers_handled, 1u);
    EXPECT_EQ(stats_incomp->total_buffers_handled, 0u);

    buf->release();
    in_port.stop();
    out_port_compatible.stop();
    out_port_incompatible.stop();
}

TEST_F(connection_test, processor_format_compatibility_forwarding)
{
    adam::data_format formatA("formatA");
    adam::data_format formatB("formatB");
    adam::data_format formatC("formatC");

    adam::test::test_port in_port("in_port"_ct);
    in_port.set_data_format(&formatA);
    in_port.start();

    adam::test::test_processor proc("proc"_ct, &formatA, &formatB);

    adam::test::test_port out_port_compatible("out_port_comp"_ct);
    out_port_compatible.set_data_format(&formatB);
    out_port_compatible.start();

    adam::test::test_port out_port_incompatible("out_port_incomp"_ct);
    out_port_incompatible.set_data_format(&formatC);
    out_port_incompatible.start();

    adam::connection conn("conn"_ct);
    conn.ports_input().push_back(&in_port);
    conn.processors().push_back(&proc);
    conn.ports_output().push_back(&out_port_compatible);
    conn.ports_output().push_back(&out_port_incompatible);

    EXPECT_TRUE(conn.has_valid_chain());

    adam::buffer* buf = adam::buffer_manager::get().request_buffer(512);
    buf->set_size(10);

    EXPECT_TRUE(conn.handle_data(buf, &in_port));

    auto* stats_comp = out_port_compatible.get_statistic_buffer()->data_as<adam::port::statistic_info>();
    auto* stats_incomp = out_port_incompatible.get_statistic_buffer()->data_as<adam::port::statistic_info>();

    EXPECT_EQ(stats_comp->total_buffers_handled, 1u);
    EXPECT_EQ(stats_incomp->total_buffers_handled, 0u);

    buf->release();
    in_port.stop();
    out_port_compatible.stop();
    out_port_incompatible.stop();
}

TEST_F(connection_test, incompatible_chain_stops_connection)
{
    adam::controller& ctrl = adam::controller::get();
    adam::registry& reg = ctrl.get_registry();

    // 1. Scan and load asterix module
    reg.add_module_path("out/build/linux-x64-debug/bin/modules"_ct);
    reg.modules().scan_for_modules();
    const adam::module* mod = nullptr;
    ASSERT_TRUE(reg.modules().load_module("asterix"_ct, &mod));
    ASSERT_NE(mod, nullptr);

    // 2. Register formatB into the loaded asterix module's formats map
    adam::data_format formatB("formatB");
    auto& formats_map = const_cast<adam::registry::data_format_map&>(mod->get_data_formats());
    formats_map.emplace(formatB.get_name().get_hash(), &formatB);

    // Get the asterix format
    auto format_it = mod->get_data_formats().find(adam::string_hashed("asterix").get_hash());
    ASSERT_NE(format_it, mod->get_data_formats().end());
    const adam::data_format* formatAsterix = format_it->second;

    adam::port* in_port = nullptr;
    ASSERT_EQ(reg.create_port("in_port"_ct, adam::port_internal::type_name(), 0, 0, 0, &in_port), adam::registry::status_success);
    in_port->set_data_format(formatAsterix);

    adam::port* out_port = nullptr;
    ASSERT_EQ(reg.create_port("out_port"_ct, adam::port_internal::type_name(), 0, 0, 0, &out_port), adam::registry::status_success);
    out_port->set_data_format(formatAsterix);

    adam::connection* conn = nullptr;
    ASSERT_EQ(reg.create_connection("conn"_ct, &conn), adam::registry::status_success);

    // Associate mutually
    conn->ports_input().push_back(in_port);
    conn->ports_output().push_back(out_port);
    in_port->in_connections().push_back(conn);
    out_port->out_connections().push_back(conn);

    EXPECT_TRUE(conn->start());
    EXPECT_TRUE(conn->is_active());
    EXPECT_TRUE(conn->has_valid_chain());

    std::vector<adam::response> resps;
    resps.emplace_back();
    adam::command_context ctx { 0, reg, ctrl, resps, {} };

    // Change out_port format to formatB
    adam::command cmd(adam::command_type::port_set_data_format);
    auto* data = cmd.data_as<adam::messages::port_data_format_data>();
    data->port = out_port->get_name().get_hash();
    data->format = formatB.get_name().get_hash();
    data->format_module = 0;

    ctrl.dispatcher().dispatch(&cmd, 1, ctx);

    ASSERT_EQ(resps.size(), 1u);
    EXPECT_EQ(resps[0].get_type(), adam::response_status::success);
    EXPECT_EQ(out_port->get_data_format()->get_name(), "formatB"_ct);

    EXPECT_FALSE(conn->is_active());

    reg.clear();
}
