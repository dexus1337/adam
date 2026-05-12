#include <gtest/gtest.h>
#include <commander/commander.hpp>
#include <controller/controller.hpp>

class commander_test : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Boot the controller asynchronously before each test
        adam::controller::get().run(true);
    }

    void TearDown() override
    {
        // Tear down the master queue and clean up shared memory
        adam::controller::get().destroy();
    }
};

/** @brief Tests commander is_active after a successful connection. */
TEST_F(commander_test, is_active_on_success)
{
    adam::commander cmdr;
    
    EXPECT_FALSE(cmdr.is_active());
    
    // Connect to the controller through the master queue
    EXPECT_TRUE(cmdr.connect());
    EXPECT_TRUE(cmdr.is_active());
    
    // Disconnect and clean up
    EXPECT_TRUE(cmdr.destroy());
    EXPECT_FALSE(cmdr.is_active());
}

/** @brief Tests if a single commander can receive an event broadcasted by the controller. */
TEST_F(commander_test, event_broadcast_and_receive)
{
    adam::commander cmdr;
    
    std::atomic<bool> event_received{false};
    adam::event_type received_type = adam::event_type::invalid;

    cmdr.dispatcher().register_handler(static_cast<int>(adam::event_type::language_changed), [&](const adam::event& e, adam::event_context&) {
        received_type = e.get_type();
        event_received = true;
    });

    ASSERT_TRUE(cmdr.connect());

    adam::event evt(adam::event_type::language_changed);
    adam::controller::get().broadcast_event(evt);

    // Wait for event to propagate across the IPC queue
    auto start = std::chrono::steady_clock::now();
    while (!event_received && std::chrono::steady_clock::now() - start < std::chrono::seconds(2))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    EXPECT_TRUE(event_received.load());
    EXPECT_EQ(received_type, adam::event_type::language_changed);

    EXPECT_TRUE(cmdr.destroy());
}

/** @brief Tests the full flow of requesting a language change and receiving the updated state via event. */
TEST_F(commander_test, request_language_change_flow)
{
    adam::commander cmdr;
    ASSERT_TRUE(cmdr.connect());

    // Default language should be english after connect (initial data received)
    EXPECT_EQ(cmdr.get_language(), adam::language_english);

    // Request language change
    adam::response_status status = cmdr.request_language_change(adam::language_german);
    EXPECT_EQ(status, adam::response_status::success);

    // Wait for the event to propagate and update the commander's language state
    auto start = std::chrono::steady_clock::now();
    while (cmdr.get_language() != adam::language_german && std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Verify the event updated the internal state successfully
    EXPECT_EQ(cmdr.get_language(), adam::language_german);

    EXPECT_TRUE(cmdr.destroy());
}