#include <gtest/gtest.h>
#include <commander/commander.hpp>
#include <controller/controller.hpp>
#include <commander/messages/command.hpp>
#include <commander/messages/event.hpp>
#include <version/version.hpp>

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

/** @brief Tests initial data synchronization of modules between controller and commander. */
TEST_F(commander_test, initial_data_module_sync)
{
    adam::controller& ctrl = adam::controller::get();
    
    // Override the initial data handler to return some mock modules
    ctrl.dispatcher().register_handler(static_cast<int>(adam::command_type::acquire_initial_data), [](const adam::command*, size_t, adam::command_context& ctx)
    {
        ctx.set_single_response_status(adam::response_status::success);
        auto* data = ctx.responses.front().data_as<adam::command::initial_data_header>();
        data->lang_info.lang = adam::language_english;
        
        data->mod_info.available_modules = 1;
        data->mod_info.unavailable_modules = 1;
        data->mod_info.loaded_modules = 1;
        
        // available module
        ctx.responses.front().set_extended(true);
        ctx.responses.emplace_back();
        auto* mod_info1 = ctx.responses[1].data_as<adam::module::basic_info>();
        mod_info1->setup(adam::module::basic_info::available, "mock_avail", "/mock/path/avail.so", adam::make_version(1, 0, 0));
        
        // unavailable module
        ctx.responses[1].set_extended(true);
        ctx.responses.emplace_back();
        auto* mod_info2 = ctx.responses[2].data_as<adam::module::basic_info>();
        mod_info2->setup(adam::module::basic_info::unavailable, "mock_unavail", "/mock/path/unavail.so", adam::make_version(2, 0, 0));
        
        // loaded module
        ctx.responses[2].set_extended(true);
        ctx.responses.emplace_back();
        auto* mod_info3 = ctx.responses[3].data_as<adam::module::basic_info>();
        mod_info3->setup(adam::module::basic_info::loaded, "mock_loaded", "/mock/path/loaded.so", adam::make_version(3, 0, 0));
    });

    adam::commander cmdr;
    ASSERT_TRUE(cmdr.connect()); // Implicitly requests initial data and populates caches

    EXPECT_EQ(cmdr.get_available_modules().size(), 1u);
    EXPECT_TRUE(cmdr.get_available_modules().contains(adam::string_hashed("mock_avail")));
    EXPECT_EQ(cmdr.get_available_modules().at(adam::string_hashed("mock_avail")).first, adam::make_version(1, 0, 0));
    EXPECT_EQ(cmdr.get_available_modules().at(adam::string_hashed("mock_avail")).second, adam::string_hashed("/mock/path/avail.so"));

    EXPECT_EQ(cmdr.get_unavailable_modules().size(), 1u);
    EXPECT_TRUE(cmdr.get_unavailable_modules().contains(adam::string_hashed("mock_unavail")));

    EXPECT_EQ(cmdr.get_loaded_modules().size(), 1u);
    EXPECT_TRUE(cmdr.get_loaded_modules().contains(adam::string_hashed("mock_loaded")));

    // Restore default handler to not break other tests on the shared controller singleton
    ctrl.dispatcher().register_default_handlers();
    EXPECT_TRUE(cmdr.destroy());
}

/** @brief Tests event synchronization of new modules getting added in the commander caches. */
TEST_F(commander_test, module_events_sync)
{
    adam::commander cmdr;
    ASSERT_TRUE(cmdr.connect());

    // Initially caches are empty (assuming no modules loaded in the test environment)
    size_t initial_available = cmdr.get_available_modules().size();
    size_t initial_unavailable = cmdr.get_unavailable_modules().size();
    size_t initial_loaded = cmdr.get_loaded_modules().size();

    // 1. Broadcast module_available event
    adam::event evt_avail(adam::event_type::module_available);
    auto* mod_info1 = evt_avail.data_as<adam::module::basic_info>();
    mod_info1->setup(adam::module::basic_info::available, "evt_mock_avail", "/mock/path/evt_avail.so", adam::make_version(1, 0, 0));

    adam::controller::get().broadcast_event(evt_avail);

    // 2. Broadcast module_unavailable event
    adam::event evt_unavail(adam::event_type::module_unavailable);
    auto* mod_info2 = evt_unavail.data_as<adam::module::basic_info>();
    mod_info2->setup(adam::module::basic_info::unavailable, "evt_mock_unavail", "/mock/path/evt_unavail.so", adam::make_version(1, 0, 0));

    adam::controller::get().broadcast_event(evt_unavail);

    // 3. Broadcast module_loaded event (this also tests that it dynamically moves from available to loaded cache)
    adam::event evt_avail2(adam::event_type::module_available);
    auto* mod_info_avail2 = evt_avail2.data_as<adam::module::basic_info>();
    mod_info_avail2->setup(adam::module::basic_info::available, "evt_mock_to_load", "/mock/path/evt_to_load.so", adam::make_version(1, 0, 0));

    adam::controller::get().broadcast_event(evt_avail2);
    
    adam::event evt_loaded(adam::event_type::module_loaded);
    auto* mod_info3 = evt_loaded.data_as<adam::module::basic_info>();
    mod_info3->setup(adam::module::basic_info::loaded, "evt_mock_to_load", "/mock/path/evt_to_load.so", adam::make_version(1, 0, 0));

    adam::controller::get().broadcast_event(evt_loaded);

    // Give events time to propagate over the IPC queue and be dispatched by the thread
    auto start = std::chrono::steady_clock::now();
    while (cmdr.get_available_modules().size() == initial_available && std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    EXPECT_EQ(cmdr.get_available_modules().size(), initial_available + 1);
    EXPECT_TRUE(cmdr.get_available_modules().contains(adam::string_hashed("evt_mock_avail")));
    
    EXPECT_EQ(cmdr.get_unavailable_modules().size(), initial_unavailable + 1);
    EXPECT_TRUE(cmdr.get_unavailable_modules().contains(adam::string_hashed("evt_mock_unavail")));

    EXPECT_EQ(cmdr.get_loaded_modules().size(), initial_loaded + 1);
    EXPECT_TRUE(cmdr.get_loaded_modules().contains(adam::string_hashed("evt_mock_to_load")));
    EXPECT_FALSE(cmdr.get_available_modules().contains(adam::string_hashed("evt_mock_to_load")));

    EXPECT_TRUE(cmdr.destroy());
}

/** @brief Tests the full flow of requesting a language change and receiving the updated state via event. */
TEST_F(commander_test, request_language_change_flow)
{
    adam::controller::get().set_language(adam::language_english); // Ensure we start from a known language state
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

/** @brief Tests the behavior when a shutdown event is received from the controller. 
TEST_F(commander_test, shutdown_event_flow)
{
    adam::commander cmdr;
    ASSERT_TRUE(cmdr.connect());
    EXPECT_TRUE(cmdr.is_active());

    // Broadcast the shutdown event
    adam::event evt(adam::event_type::shutdown);
    adam::controller::get().broadcast_event(evt);

    // Wait for the event to propagate and the commander to destroy itself
    auto start = std::chrono::steady_clock::now();
    while (cmdr.is_active() && std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Verify the commander is no longer active
    EXPECT_FALSE(cmdr.is_active());
    EXPECT_TRUE(cmdr.destroy());
}*/