#include <gtest/gtest.h>
#include "data/processor.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "memory/buffer/buffer.hpp"
#include "configuration/parameters/configuration-parameter-boolean.hpp"

#include <thread>
#include <chrono>
#include <atomic>

using namespace adam::string_hashed_ct_literals;

class processor_test : public ::testing::Test
{
protected:
    class test_processor : public adam::processor
    {
    public:
        test_processor(const adam::string_hashed& name) : adam::processor(name) {}
        
        const adam::string_hashed_ct& get_type_name() const override { static adam::string_hashed_ct type = "test"_ct; return type; }

        bool handle_data(adam::buffer*& buff) override 
        { 
            if (!buff) return false;
            handled_buffers++;
            return true;
        }
        
        std::atomic<int> handled_buffers{0};
    };

    void SetUp() override
    {
        adam::buffer_manager::get().initialize();
    }

    void TearDown() override
    {
        adam::buffer_manager::get().destroy();
    }
};

/** @brief Tests the base parameters and memory states directly after the processor is constructed. */
TEST_F(processor_test, initial_state)
{
    test_processor p("test_processor"_ct);
    
    // Processors have transparent format by default
    EXPECT_NE(p.get_input_format(), nullptr);
    EXPECT_NE(p.get_output_format(), nullptr);
}

/** @brief Tests that basic_info serialization sets fields correctly. */
TEST_F(processor_test, basic_info_setup)
{
    test_processor p("test_processor"_ct);
    adam::processor::basic_info info;
    
    adam::buffer_handle handle;
    info.setup(p.get_name(), p.get_type_name(), 0, false, handle);
    
    EXPECT_EQ(std::string(info.name), "test_processor");
    EXPECT_EQ(info.type, "test"_ct.get_hash());
    EXPECT_FALSE(info.is_unavailable);
}

/** @brief Tests handling payload buffers correctly forwards to subclass handle_data. */
TEST_F(processor_test, handle_data)
{
    test_processor p("test_processor"_ct);
    
    adam::buffer* buf = adam::buffer_manager::get().request_buffer(1024);
    buf->set_size(123);
    
    EXPECT_TRUE(p.handle_data(buf));
    EXPECT_EQ(p.handled_buffers.load(), 1);
    
    buf->release();
}
