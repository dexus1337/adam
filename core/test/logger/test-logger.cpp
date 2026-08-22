#include <gtest/gtest.h>
#include <logger/logger.hpp>
#include <controller/controller.hpp>
#include <memory/buffer/buffer-manager.hpp>
#include <thread>

class logger_test : public ::testing::Test
{
protected:
    void SetUp() override
    {
        adam::buffer_manager::get().initialize();
        adam::controller::get().get_registry().clear();
    }

    void TearDown() override
    {
        adam::controller::get().get_registry().clear();
        adam::controller::get().destroy();
        adam::buffer_manager::get().destroy();
    }
};

/** @brief Tests logger command queue lifecycle */
TEST_F(logger_test, command_queue_lifecycle)
{
    // Ensure the controller is running asynchronously so the master queue can respond
    adam::controller& ctrl = adam::controller::get();
    ASSERT_TRUE(ctrl.run(true));
    
    adam::logger logg;
    
    EXPECT_FALSE(logg.is_active());
    
    // Connect to the controller through the master queue
    EXPECT_TRUE(logg.connect());
    EXPECT_TRUE(logg.is_active());
    
    // Disconnect and clean up
    EXPECT_TRUE(logg.destroy());
    EXPECT_FALSE(logg.is_active());
}

/** @brief Tests logger is_active when master queue request fails */
TEST_F(logger_test, is_active_on_failure)
{
    // Ensure the controller is NOT running
    adam::controller& ctrl = adam::controller::get();
    if (ctrl.is_active())
        ctrl.destroy();

    adam::logger logg;
    
    EXPECT_FALSE(logg.is_active());
    EXPECT_FALSE(logg.connect()); // Should fail as controller is not running
    EXPECT_FALSE(logg.is_active());
}