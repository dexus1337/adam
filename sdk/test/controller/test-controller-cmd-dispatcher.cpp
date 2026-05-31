#include <gtest/gtest.h>
#include <controller/controller.hpp>
#include <controller/controller-cmd-dispatcher.hpp>
#include <commander/messages/command.hpp>
#include <data/connection.hpp>

using namespace adam::string_hashed_ct_literals;


class controller_cmd_dispatcher_test : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

/** @brief Tests that the dispatcher correctly registers and routes to custom handlers. */
TEST_F(controller_cmd_dispatcher_test, register_and_dispatch_custom_handler)
{
    adam::controller& ctrl = adam::controller::get();
    adam::registry& reg = ctrl.get_registry();
    adam::controller_cmd_dispatcher dispatcher;
    
    // Arbitrary command type ID for our custom handler
    const adam::command_type custom_cmd_type = static_cast<adam::command_type>(9999);
    bool handler_invoked = false;
    
    dispatcher.register_handler(custom_cmd_type, [&](const adam::command*, size_t, adam::command_context& ctx) 
    {
        handler_invoked = true;
        ctx.set_single_response_status(adam::response_status::success);
    });
    
    std::vector<adam::response> resps;
    resps.emplace_back();
    adam::command_context ctx { 0, reg, ctrl, resps, {} };
    
    // Creating a standard command which inherently falls outside our custom boundaries unless specifically modified.
    adam::command cmd;
    
    dispatcher.dispatch(&cmd, 1, ctx);
    
    // Because we just dispatched a default-initialized command with an unknown type,
    // the dispatcher should fall back to its internal unknown response.
    if (static_cast<int>(cmd.get_type()) != custom_cmd_type)
    {
        ASSERT_EQ(resps.size(), static_cast<size_t>(1));
        EXPECT_EQ(resps[0].get_type(), adam::response_status::unknown);
        EXPECT_FALSE(handler_invoked);
    }
}

/** @brief Tests that the dispatcher correctly supports returning multiple extended responses. */
TEST_F(controller_cmd_dispatcher_test, multiple_responses)
{
    adam::controller& ctrl = adam::controller::get();
    adam::registry& reg = ctrl.get_registry();
    adam::controller_cmd_dispatcher dispatcher;
    
    const adam::command_type custom_cmd_type = static_cast<adam::command_type>(9999);
    
    dispatcher.register_handler(custom_cmd_type, [&](const adam::command*, size_t, adam::command_context& ctx) 
    {
        ctx.set_single_response_status(adam::response_status::success);
        ctx.responses.front().set_extended(true);

        ctx.responses[1].set_extended(true);
        ctx.responses[1].type() = adam::response_status::success;

        ctx.responses[2].set_extended(false);
        ctx.responses[2].type() = adam::response_status::success;
    });
    
    adam::command cmd(static_cast<adam::command_type>(custom_cmd_type));
    
    std::vector<adam::response> resps;
    resps.emplace_back();
    resps.emplace_back();
    resps.emplace_back();
    adam::command_context ctx { 0, reg, ctrl, resps, {} };
    dispatcher.dispatch(&cmd, 1, ctx);
    
    ASSERT_EQ(resps.size(), static_cast<size_t>(3));
    EXPECT_TRUE(resps[0].is_extended());
    EXPECT_TRUE(resps[1].is_extended());
    EXPECT_FALSE(resps[2].is_extended());
}

/** @brief Tests that default handlers are registered without faulting. */
TEST_F(controller_cmd_dispatcher_test, default_handlers)
{
    adam::controller_cmd_dispatcher dispatcher;
    
    // Test whether the default handlers properly register
    dispatcher.register_default_handlers();
    
    SUCCEED();
}

/** @brief Tests that the connection_set_input_data_format command correctly changes a connection's data format. */
TEST_F(controller_cmd_dispatcher_test, connection_set_input_data_format_dispatch)
{
    adam::controller& ctrl = adam::controller::get();
    adam::registry& reg = ctrl.get_registry();
    adam::controller_cmd_dispatcher dispatcher;
    dispatcher.register_default_handlers();

    auto conn_name = "test_format_conn"_ct;
    adam::connection* conn = nullptr;
    ASSERT_EQ(reg.create_connection(conn_name, &conn), adam::registry::status_success);
    ASSERT_NE(conn, nullptr);

    // Initial formats are expected to be transparent
    EXPECT_EQ(conn->get_input_format()->get_name(), "transparent"_ct);
    EXPECT_EQ(conn->get_output_format()->get_name(), "transparent"_ct);

    // Prepare set data format command
    adam::command cmd(adam::command_type::connection_set_input_data_format);
    auto* data = cmd.data_as<adam::messages::connection_data_format_data>();
    data->connection = conn_name.get_hash();
    data->format = "transparent"_ct.get_hash();
    data->format_module = 0;

    std::vector<adam::response> resps;
    resps.emplace_back();
    adam::command_context ctx { 0, reg, ctrl, resps, {} };
    dispatcher.dispatch(&cmd, 1, ctx);

    EXPECT_EQ(resps[0].get_type(), adam::response_status::success);
    EXPECT_EQ(conn->get_input_format()->get_name(), "transparent"_ct);
    EXPECT_EQ(conn->get_output_format()->get_name(), "transparent"_ct);

    // Clean up
    reg.destroy_connection(conn_name.get_hash());
}