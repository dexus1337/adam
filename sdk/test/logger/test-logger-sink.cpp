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
    
    // Theres one log in the queue in debug so yeah remove that
    debug_statement
    (
        adam::log syslog;
        EXPECT_TRUE(log_sink.queue().pop(syslog, 100))
    );

    adam::log send1(adam::log::error, "logger_sink test error");
    adam::log send2(adam::log::trace, "logger_sink test trace");

    // Send 2 different logs
    ctrl.log(send1);
    ctrl.log(send2);

    // They should now be in the queue for our sink
    EXPECT_FALSE(log_sink.queue().is_empty());

    adam::log recv1;
    adam::log recv2;

    // We should only be able to pop 2
    EXPECT_TRUE(log_sink.queue().pop(recv1, 100));
    EXPECT_TRUE(log_sink.queue().pop(recv2, 100));
    
    EXPECT_TRUE(log_sink.queue().is_empty());

    // And they should be exactly equal
    EXPECT_EQ(send1.get_level(), recv1.get_level());
    EXPECT_EQ(send1.get_timestamp(), recv1.get_timestamp());
    EXPECT_STREQ(send1.get_text(), recv1.get_text());

    EXPECT_EQ(send2.get_level(), recv2.get_level());
    EXPECT_EQ(send2.get_timestamp(), recv2.get_timestamp());
    EXPECT_STREQ(send2.get_text(), recv2.get_text());

    // Disconnect and clean up
    EXPECT_TRUE(log_sink.destroy());

    // Clean up controller for subsequent tests
    EXPECT_TRUE(ctrl.destroy());
}