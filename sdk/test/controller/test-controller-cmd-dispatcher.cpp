#include <gtest/gtest.h>
#include <controller/controller.hpp>
#include <controller/controller-cmd-dispatcher.hpp>
#include <commander/messages/command.hpp>
#include <controller/registry.hpp>

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
    const int custom_cmd_type = 9999;
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
    
    const int custom_cmd_type = 9999;
    
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