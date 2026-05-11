#include <gtest/gtest.h>

#include "commander/commander.hpp"
#include "controller/controller.hpp"
#include "data/port/port-output-internal.hpp"
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
        // 1. Initialize buffer memory pools
        adam::buffer_manager::get().initialize();

        // 2. Start the controller in asynchronous mode so it processes IPC queues in the background
        ASSERT_TRUE(adam::controller::get().run(true));
    }

    void TearDown() override
    {
        adam::controller::get().destroy();
        adam::buffer_manager::get().destroy();
    }
};

TEST_F(commander_inspector_test, lifecycle_and_data_transfer)
{
    adam::string_hashed port_name("adam::test_inspector_port");

    // 1. Create a data port and register it in the controller context
    auto port_instance = std::make_unique<adam::port_output_internal>(port_name);
    adam::port_output_internal* test_port = port_instance.get();
    
    adam::controller::get().get_registry().ports().emplace(port_name, std::move(port_instance));

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
    adam::response resp = cmd.request_inspector_create(port_name, callback, inspector);
    
    ASSERT_EQ(resp.get_type(), adam::response_status::success);
    ASSERT_NE(inspector, nullptr);

    // Give the IPC system and thread a short moment to establish the active listening loop
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // 5. Request a buffer, write test data, and pass it through the controller's port
    adam::buffer* buf = adam::buffer_manager::get().request_buffer(sizeof(int));
    ASSERT_NE(buf, nullptr);
    
    int sent_value = 1337;
    *static_cast<int*>(buf->data()) = sent_value;
    buf->set_size(sizeof(int));
    
    EXPECT_EQ(buf->get_ref_count(), 1u);
    
    // Actually push the data into the port
    test_port->handle_data(buf);

    // Wait for the inspector thread to pop and invoke the callback (it will block)
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // The queue should have successfully transferred the reference to the inspector.
    EXPECT_EQ(buf->get_ref_count(), 2u);

    // Release our local ownership of the buffer
    buf->release();

    EXPECT_EQ(buf->get_ref_count(), 1u);

    // 6. Let the callback proceed and finish
    pause_callback.store(false);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_EQ(received_count.load(), 1);
    EXPECT_EQ(received_value.load(), sent_value);
    EXPECT_EQ(buf->get_ref_count(), 0u);

    // 7. Tear down the inspector gracefully
    resp = cmd.request_inspector_destroy(inspector);
    EXPECT_EQ(resp.get_type(), adam::response_status::success);

    EXPECT_TRUE(cmd.destroy());
}