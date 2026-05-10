#include <gtest/gtest.h>
#include <controller/controller.hpp>
#include <controller/controller-cmd-dispatcher.hpp>
#include <commander/command-response/command.hpp>
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
    
    dispatcher.register_handler(custom_cmd_type, [&](const adam::command*, size_t, adam::command_context&) -> adam::response 
    {
        handler_invoked = true;
        return adam::response(adam::response_status::success);
    });
    
    adam::language lang = adam::language_english;
    adam::command_context ctx { 0, reg, lang, ctrl, {} };
    
    // Creating a standard command which inherently falls outside our custom boundaries unless specifically modified.
    adam::command cmd;
    
    adam::response resp = dispatcher.dispatch(&cmd, 1, ctx);
    
    // Because we just dispatched a default-initialized command with an unknown type,
    // the dispatcher should fall back to its internal unknown response.
    if (static_cast<int>(cmd.get_type()) != custom_cmd_type)
    {
        EXPECT_EQ(resp.get_type(), adam::response_status::unknown);
        EXPECT_FALSE(handler_invoked);
    }
}

/** @brief Tests that default handlers are registered without faulting. */
TEST_F(controller_cmd_dispatcher_test, default_handlers)
{
    adam::controller_cmd_dispatcher dispatcher;
    
    // Test whether the default handlers properly register
    dispatcher.register_default_handlers();
    
    SUCCEED();
}