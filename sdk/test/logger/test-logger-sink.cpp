#include <gtest/gtest.h>
#include <logger/logger-sink.hpp>
#include <controller/controller.hpp>

/** @brief Tests logger sink command queue lifecycle */
TEST(logger_sink, command_queue_lifecycle)
{
    // Ensure the controller is running asynchronously so the master queue can respond
    adam::controller& ctrl = adam::controller::get();
    ASSERT_TRUE(ctrl.run(true));
    
    adam::logger_sink log_sink;
    
    
    // Connect to the controller through the master queue
    EXPECT_TRUE(log_sink.connect());
    
    // Disconnect and clean up
    EXPECT_TRUE(log_sink.destroy());

    // Clean up controller for subsequent tests
    EXPECT_TRUE(ctrl.destroy());
}