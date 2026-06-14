#include <gtest/gtest.h>

#include "commander/commander.hpp"
#include "controller/controller.hpp"
#include "data/port-types/port-internal.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "memory/buffer/buffer.hpp"
#include "data/inspector.hpp"

#include <thread>
#include <chrono>
#include <atomic>

class commander_inspector_test : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // 0. Ensure the buffer manager is initialized
        adam::buffer_manager::get().initialize();

        // 1. Ensure a completely clean state for the controller's registry across tests
        adam::controller::get().get_registry().clear();

        // 2. Start the controller in asynchronous mode so it processes IPC queues in the background
        ASSERT_TRUE(adam::controller::get().run(true));
    }

    void TearDown() override
    {
        // 3. Clear the registry to safely release any lingering ports while the buffer manager is still active
        adam::controller::get().get_registry().clear();
        adam::controller::get().destroy();
        
        // 4. Force a complete wipe of the buffer memory pools AFTER all background threads have exited and dumped their caches
        adam::buffer_manager::get().destroy();
    }
};

using namespace adam::string_hashed_ct_literals;

TEST_F(commander_inspector_test, lifecycle_and_data_transfer)
{
    adam::string_hashed port_name("adam::test_inspector_port");

    // 1. Create a data port and register it in the controller context
    adam::port* test_port = nullptr;
    adam::registry::status stat = adam::controller::get().get_registry().create_port(port_name, adam::port_internal::type_name(), "essential"_ct.get_hash(), &test_port);
    
    ASSERT_EQ(stat, adam::registry::status_success);
    ASSERT_NE(test_port, nullptr);
    
    ASSERT_TRUE(test_port->start());

    // 2. Connect the external commander
    adam::commander cmd;
    ASSERT_TRUE(cmd.connect());

    std::atomic<int> received_value{0};
    std::atomic<int> received_count{0};
    std::atomic<bool> pause_callback{true};

    // 3. Define the listener callback
    auto callback = [&](adam::buffer* buf) 
    {
        while (pause_callback.load())
        {
            std::this_thread::yield();
        }

        if (buf && buf->get_size() >= sizeof(int))
        {
            received_value = *static_cast<int*>(buf->data());
            received_count++;
        }
    };

    // 4. Request the commander to create its remote instance of the data inspector
    adam::data_inspector* inspector = nullptr;
    adam::response_status resp = cmd.request_inspector_create(port_name, callback, inspector);
    
    ASSERT_EQ(resp, adam::response_status::success);
    ASSERT_NE(inspector, nullptr);

    // Give the IPC system and thread a short moment to establish the active listening loop
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // 5. Request a buffer, write test data, and pass it through the controller's port
    adam::buffer* buf = adam::buffer_manager::get().request_buffer(sizeof(int));
    ASSERT_NE(buf, nullptr);
    buf->set_timestamp();
    
    int sent_value = 1337;
    *static_cast<int*>(buf->data()) = sent_value;
    buf->set_size(sizeof(int));
    
    EXPECT_EQ(buf->get_ref_count(), 1u);
    
    // Actually push the data into the port
    test_port->handle_data(buf, adam::data_direction_in);

    // Wait for the inspector thread to pop and invoke the callback (it will block)
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // The queue should have successfully transferred the reference to the inspector.
    EXPECT_EQ(buf->get_ref_count(), 2u);

    // Release our local ownership of the buffer
    buf->release();

    EXPECT_EQ(buf->get_ref_count(), 1u);

    // 6. Let the callback proceed and finish
    pause_callback.store(false);
    
    auto start = std::chrono::steady_clock::now();
    while (received_count.load() == 0 && std::chrono::steady_clock::now() - start < std::chrono::milliseconds(1000))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    EXPECT_EQ(received_count.load(), 1);
    EXPECT_EQ(received_value.load(), sent_value);

    // 7. Tear down the inspector gracefully
    resp = cmd.request_inspector_destroy(inspector);
    EXPECT_EQ(resp, adam::response_status::success);

    EXPECT_TRUE(cmd.destroy());
    
    EXPECT_TRUE(test_port->stop());
}