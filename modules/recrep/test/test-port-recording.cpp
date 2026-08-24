#include <gtest/gtest.h>
#include "data/port-types/port-output-recording.hpp"
#include "data/port-types/port-input-replay.hpp"
#include "data/inspector.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "controller/controller.hpp"
#include "configuration/parameters/configuration-parameter-list.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-double.hpp"

#include <thread>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <vector>
#include <mutex>
#include <condition_variable>

using namespace adam;
using namespace adam::modules::recrep;

class recording_test : public ::testing::Test
{
protected:
    void SetUp() override
    {
        buffer_manager::get().initialize();
        std::error_code ec;
        std::filesystem::remove_all(temp_dir, ec);
        std::filesystem::create_directories(temp_dir, ec);
    }

    void TearDown() override
    {
        buffer_manager::get().destroy();
        std::error_code ec;
        std::filesystem::remove_all(temp_dir, ec);
    }

    std::string temp_dir = "recording_test_temp";

    struct received_buffer_info
    {
        std::string payload;
        uint64_t timestamp = 0;
        uint32_t size = 0;
    };

    class test_buffer_collector
    {
    public:
        test_buffer_collector(port& p)
            : m_port(p),
              m_inspector(std::make_shared<data_inspector>())
        {
            m_inspector->create(p.get_name());
            m_port.add_inspector(m_inspector);
            m_inspector->start_inspecting([this](buffer* buf)
            {
                if (!buf)
                {
                    return;
                }

                std::lock_guard lock(m_mutex);
                m_received.push_back({
                    std::string(buf->get_begin_as<const char>(), buf->get_size()),
                    buf->get_timestamp(),
                    buf->get_size()
                });
                m_cv.notify_all();
            });
        }

        ~test_buffer_collector()
        {
            m_port.remove_inspector(m_inspector);
            m_inspector->destroy();
        }

        std::vector<received_buffer_info> wait_for_buffers(size_t count, std::chrono::milliseconds timeout = std::chrono::milliseconds(2000))
        {
            std::unique_lock lock(m_mutex);
            m_cv.wait_for(lock, timeout, [this, count]()
            {
                return m_received.size() >= count;
            });
            return m_received;
        }

    private:
        port& m_port;
        std::shared_ptr<data_inspector> m_inspector;
        std::mutex m_mutex;
        std::condition_variable m_cv;
        std::vector<received_buffer_info> m_received;
    };

    void write_test_data(port_output_recording& port, int num_buffers, uint64_t start_time, uint64_t interval)
    {
        for (int i = 0; i < num_buffers; ++i)
        {
            buffer* buf = buffer_manager::get().request_buffer(64);
            ASSERT_NE(buf, nullptr);
            std::string text = "Test packet " + std::to_string(i);
            std::memcpy(buf->data_as<char>(), text.c_str(), text.size() + 1);
            buf->set_size(static_cast<uint32_t>(text.size() + 1));
            buf->set_timestamp(start_time + i * interval);
            EXPECT_TRUE(port.write(buf));
            buf->release();
        }
    }

    void read_and_verify(port_input_replay& port, size_t expected_buffers, std::chrono::milliseconds timeout = std::chrono::milliseconds(2000))
    {
        test_buffer_collector collector(port);
        ASSERT_TRUE(port.start());
        auto received = collector.wait_for_buffers(expected_buffers, timeout);
        EXPECT_TRUE(port.stop());

        ASSERT_EQ(received.size(), expected_buffers);
        for (size_t i = 0; i < expected_buffers; ++i)
        {
            std::string expected_text = "Test packet " + std::to_string(i);
            EXPECT_STREQ(received[i].payload.c_str(), expected_text.c_str());
        }
    }
};

TEST_F(recording_test, format_pcap_single)
{
    port_output_recording rec_port("rec_pcap_single");
    rec_port.set_controller(&controller::get());
    
    auto r_params = rec_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    r_params->get<configuration_parameter_string>("data_format"_ct)->set_value("pcap"_ct);
    r_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("single"_ct);
    r_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(temp_dir));

    ASSERT_TRUE(rec_port.start());
    write_test_data(rec_port, 10, 1000000000ull, 1000000ull);
    ASSERT_TRUE(rec_port.stop());

    bool found = false;
    std::string file_path;
    for (const auto& entry : std::filesystem::directory_iterator(temp_dir))
    {
        if (entry.path().extension() == ".pcap")
        {
            found = true;
            file_path = entry.path().string();
        }
    }
    ASSERT_TRUE(found);

    port_input_replay rep_port("rep_pcap_single");
    rep_port.set_controller(&controller::get());

    auto i_params = rep_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    i_params->get<configuration_parameter_string>("data_format"_ct)->set_value("pcap"_ct);
    i_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("single_file"_ct);
    i_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(file_path));
    i_params->get<configuration_parameter_double>("speed"_ct)->set_value(0.0);

    read_and_verify(rep_port, 10);
}

TEST_F(recording_test, format_rff_single)
{
    port_output_recording rec_port("rec_rff_single");
    rec_port.set_controller(&controller::get());
    
    auto r_params = rec_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    r_params->get<configuration_parameter_string>("data_format"_ct)->set_value("rff"_ct);
    r_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("single"_ct);
    r_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(temp_dir));

    ASSERT_TRUE(rec_port.start());
    write_test_data(rec_port, 10, 1000000000ull, 1000000ull);
    ASSERT_TRUE(rec_port.stop());

    bool found = false;
    std::string file_path;
    for (const auto& entry : std::filesystem::directory_iterator(temp_dir))
    {
        if (entry.path().extension() == ".rff")
        {
            found = true;
            file_path = entry.path().string();
        }
    }
    ASSERT_TRUE(found);

    port_input_replay rep_port("rep_rff_single");
    rep_port.set_controller(&controller::get());

    auto i_params = rep_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    i_params->get<configuration_parameter_string>("data_format"_ct)->set_value("rff"_ct);
    i_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("single_file"_ct);
    i_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(file_path));
    i_params->get<configuration_parameter_double>("speed"_ct)->set_value(0.0);

    read_and_verify(rep_port, 10);
}

TEST_F(recording_test, chunked_by_size_pcap)
{
    port_output_recording rec_port("rec_chunk_size_pcap");
    rec_port.set_controller(&controller::get());
    
    auto r_params = rec_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    r_params->get<configuration_parameter_string>("data_format"_ct)->set_value("pcap"_ct);
    r_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("chunked"_ct);
    r_params->get<configuration_parameter_string>("chunk_mode"_ct)->set_value("size"_ct);
    r_params->get<configuration_parameter_integer>("chunk_size"_ct)->set_value(1); // 1 MB
    r_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(temp_dir));

    ASSERT_TRUE(rec_port.start());
    
    size_t num_buffers = 20;
    uint32_t data_size = 100000; // ~100KB per buffer
    
    std::vector<char> large_data(data_size, 'A');
    for (size_t i = 0; i < num_buffers; ++i)
    {
        buffer* buf = buffer_manager::get().request_buffer(data_size);
        ASSERT_NE(buf, nullptr);
        std::memcpy(buf->data_as<char>(), large_data.data(), data_size);
        buf->set_size(data_size);
        buf->set_timestamp(1000000000ull + i * 1000000ull);
        EXPECT_TRUE(rec_port.write(buf));
        buf->release();
    }
    ASSERT_TRUE(rec_port.stop());

    int file_count = 0;
    for (const auto& entry : std::filesystem::directory_iterator(temp_dir))
    {
        if (entry.path().extension() == ".pcap")
        {
            file_count++;
        }
    }
    EXPECT_GE(file_count, 2);

    port_input_replay rep_port("rep_chunk_size_pcap");
    rep_port.set_controller(&controller::get());
    auto i_params = rep_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    i_params->get<configuration_parameter_string>("data_format"_ct)->set_value("pcap"_ct);
    i_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("directory"_ct);
    i_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(temp_dir));
    i_params->get<configuration_parameter_double>("speed"_ct)->set_value(0.0);

    test_buffer_collector collector(rep_port);
    ASSERT_TRUE(rep_port.start());
    auto received = collector.wait_for_buffers(num_buffers);
    ASSERT_TRUE(rep_port.stop());

    ASSERT_EQ(received.size(), num_buffers);
    for (size_t i = 0; i < num_buffers; ++i)
    {
        EXPECT_EQ(received[i].size, data_size);
    }
}

TEST_F(recording_test, chunked_by_size_rff)
{
    port_output_recording rec_port("rec_chunk_size_rff");
    rec_port.set_controller(&controller::get());
    
    auto r_params = rec_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    r_params->get<configuration_parameter_string>("data_format"_ct)->set_value("rff"_ct);
    r_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("chunked"_ct);
    r_params->get<configuration_parameter_string>("chunk_mode"_ct)->set_value("size"_ct);
    r_params->get<configuration_parameter_integer>("chunk_size"_ct)->set_value(1); // 1 MB
    r_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(temp_dir));

    ASSERT_TRUE(rec_port.start());
    
    size_t num_buffers = 20;
    uint32_t data_size = 60000; // ~60KB per buffer (RFF max block size is 65535)
    
    std::vector<char> large_data(data_size, 'B');
    for (size_t i = 0; i < num_buffers; ++i)
    {
        buffer* buf = buffer_manager::get().request_buffer(data_size);
        ASSERT_NE(buf, nullptr);
        std::memcpy(buf->data_as<char>(), large_data.data(), data_size);
        buf->set_size(data_size);
        buf->set_timestamp(1000000000ull + i * 1000000ull);
        EXPECT_TRUE(rec_port.write(buf));
        buf->release();
    }
    ASSERT_TRUE(rec_port.stop());

    int file_count = 0;
    for (const auto& entry : std::filesystem::directory_iterator(temp_dir))
    {
        if (entry.path().extension() == ".rff")
        {
            file_count++;
        }
    }
    EXPECT_GE(file_count, 2);

    port_input_replay rep_port("rep_chunk_size_rff");
    rep_port.set_controller(&controller::get());
    auto i_params = rep_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    i_params->get<configuration_parameter_string>("data_format"_ct)->set_value("rff"_ct);
    i_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("directory"_ct);
    i_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(temp_dir));
    i_params->get<configuration_parameter_double>("speed"_ct)->set_value(0.0);

    test_buffer_collector collector(rep_port);
    ASSERT_TRUE(rep_port.start());
    auto received = collector.wait_for_buffers(num_buffers);
    ASSERT_TRUE(rep_port.stop());

    ASSERT_EQ(received.size(), num_buffers);
    for (size_t i = 0; i < num_buffers; ++i)
    {
        EXPECT_EQ(received[i].size, data_size);
    }
}

TEST_F(recording_test, chunked_by_time_pcap)
{
    port_output_recording rec_port("rec_chunk_time_pcap");
    rec_port.set_controller(&controller::get());
    
    auto r_params = rec_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    r_params->get<configuration_parameter_string>("data_format"_ct)->set_value("pcap"_ct);
    r_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("chunked"_ct);
    r_params->get<configuration_parameter_string>("chunk_mode"_ct)->set_value("time"_ct);
    r_params->get<configuration_parameter_integer>("chunk_duration"_ct)->set_value(1); // 1 second
    r_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(temp_dir));

    ASSERT_TRUE(rec_port.start());
    
    uint64_t start_time = 1000000000ull;
    for (int i = 0; i < 5; ++i)
    {
        buffer* buf = buffer_manager::get().request_buffer(64);
        ASSERT_NE(buf, nullptr);
        std::string text = "Test packet " + std::to_string(i);
        std::memcpy(buf->data_as<char>(), text.c_str(), text.size() + 1);
        buf->set_size(static_cast<uint32_t>(text.size() + 1));
        buf->set_timestamp(start_time + i * 750000000ull); 
        EXPECT_TRUE(rec_port.write(buf));
        buf->release();
    }
    ASSERT_TRUE(rec_port.stop());

    int file_count = 0;
    for (const auto& entry : std::filesystem::directory_iterator(temp_dir))
    {
        if (entry.path().extension() == ".pcap")
        {
            file_count++;
        }
    }
    EXPECT_GE(file_count, 3); // 3-4 chunks

    port_input_replay rep_port("rep_chunk_time_pcap");
    rep_port.set_controller(&controller::get());
    auto i_params = rep_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    i_params->get<configuration_parameter_string>("data_format"_ct)->set_value("pcap"_ct);
    i_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("directory"_ct);
    i_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(temp_dir));
    i_params->get<configuration_parameter_double>("speed"_ct)->set_value(0.0);

    read_and_verify(rep_port, 5);
}

TEST_F(recording_test, chunked_by_time_rff)
{
    port_output_recording rec_port("rec_chunk_time_rff");
    rec_port.set_controller(&controller::get());
    
    auto r_params = rec_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    r_params->get<configuration_parameter_string>("data_format"_ct)->set_value("rff"_ct);
    r_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("chunked"_ct);
    r_params->get<configuration_parameter_string>("chunk_mode"_ct)->set_value("time"_ct);
    r_params->get<configuration_parameter_integer>("chunk_duration"_ct)->set_value(1); // 1 second
    r_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(temp_dir));

    ASSERT_TRUE(rec_port.start());
    
    uint64_t start_time = 1000000000ull;
    for (int i = 0; i < 5; ++i)
    {
        buffer* buf = buffer_manager::get().request_buffer(64);
        ASSERT_NE(buf, nullptr);
        std::string text = "Test packet " + std::to_string(i);
        std::memcpy(buf->data_as<char>(), text.c_str(), text.size() + 1);
        buf->set_size(static_cast<uint32_t>(text.size() + 1));
        buf->set_timestamp(start_time + i * 750000000ull); 
        EXPECT_TRUE(rec_port.write(buf));
        buf->release();
    }
    ASSERT_TRUE(rec_port.stop());

    int file_count = 0;
    for (const auto& entry : std::filesystem::directory_iterator(temp_dir))
    {
        if (entry.path().extension() == ".rff")
        {
            file_count++;
        }
    }
    EXPECT_GE(file_count, 3); // 3-4 chunks

    port_input_replay rep_port("rep_chunk_time_rff");
    rep_port.set_controller(&controller::get());
    auto i_params = rep_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    i_params->get<configuration_parameter_string>("data_format"_ct)->set_value("rff"_ct);
    i_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("directory"_ct);
    i_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(temp_dir));
    i_params->get<configuration_parameter_double>("speed"_ct)->set_value(0.0);

    read_and_verify(rep_port, 5);
}

TEST_F(recording_test, buffer_start_offset_pcap)
{
    port_output_recording rec_port("rec_start_offset_pcap");
    rec_port.set_controller(&controller::get());
    
    auto r_params = rec_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    r_params->get<configuration_parameter_string>("data_format"_ct)->set_value("pcap"_ct);
    r_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("single"_ct);
    r_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(temp_dir));

    ASSERT_TRUE(rec_port.start());

    buffer* buf = buffer_manager::get().request_buffer(64);
    ASSERT_NE(buf, nullptr);

    std::memcpy(buf->data_as<char>(), "GARBAGE_HEADER", 14);
    std::string text = "Payload data after offset";
    std::memcpy(buf->data_as<char>() + 14, text.c_str(), text.size() + 1);
    buf->set_start_pos(14);
    buf->set_size(static_cast<uint32_t>(text.size() + 1));
    buf->set_timestamp(1000000000ull);
    EXPECT_TRUE(rec_port.write(buf));
    buf->release();

    ASSERT_TRUE(rec_port.stop());

    port_input_replay rep_port("rep_start_offset_pcap");
    rep_port.set_controller(&controller::get());

    auto i_params = rep_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    i_params->get<configuration_parameter_string>("data_format"_ct)->set_value("pcap"_ct);
    i_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("single_file"_ct);

    std::string file_path;
    for (const auto& entry : std::filesystem::directory_iterator(temp_dir))
    {
        if (entry.path().extension() == ".pcap")
        {
            file_path = entry.path().string();
        }
    }
    ASSERT_FALSE(file_path.empty());
    i_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(file_path));
    i_params->get<configuration_parameter_double>("speed"_ct)->set_value(0.0);

    test_buffer_collector collector(rep_port);
    ASSERT_TRUE(rep_port.start());
    auto received = collector.wait_for_buffers(1);
    ASSERT_TRUE(rep_port.stop());

    ASSERT_EQ(received.size(), 1u);
    EXPECT_STREQ(received[0].payload.c_str(), "Payload data after offset");
}

TEST_F(recording_test, buffer_start_offset_rff)
{
    port_output_recording rec_port("rec_start_offset_rff");
    rec_port.set_controller(&controller::get());
    
    auto r_params = rec_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    r_params->get<configuration_parameter_string>("data_format"_ct)->set_value("rff"_ct);
    r_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("single"_ct);
    r_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(temp_dir));

    ASSERT_TRUE(rec_port.start());

    buffer* buf = buffer_manager::get().request_buffer(64);
    ASSERT_NE(buf, nullptr);

    std::memcpy(buf->data_as<char>(), "GARBAGE_HEADER", 14);
    std::string text = "RFF payload after offset";
    std::memcpy(buf->data_as<char>() + 14, text.c_str(), text.size() + 1);
    buf->set_start_pos(14);
    buf->set_size(static_cast<uint32_t>(text.size() + 1));
    buf->set_timestamp(1000000000ull);
    EXPECT_TRUE(rec_port.write(buf));
    buf->release();

    ASSERT_TRUE(rec_port.stop());

    port_input_replay rep_port("rep_start_offset_rff");
    rep_port.set_controller(&controller::get());

    auto i_params = rep_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    i_params->get<configuration_parameter_string>("data_format"_ct)->set_value("rff"_ct);
    i_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("single_file"_ct);

    std::string file_path;
    for (const auto& entry : std::filesystem::directory_iterator(temp_dir))
    {
        if (entry.path().extension() == ".rff")
        {
            file_path = entry.path().string();
        }
    }
    ASSERT_FALSE(file_path.empty());
    i_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(file_path));
    i_params->get<configuration_parameter_double>("speed"_ct)->set_value(0.0);

    test_buffer_collector collector(rep_port);
    ASSERT_TRUE(rep_port.start());
    auto received = collector.wait_for_buffers(1);
    ASSERT_TRUE(rep_port.stop());

    ASSERT_EQ(received.size(), 1u);
    EXPECT_STREQ(received[0].payload.c_str(), "RFF payload after offset");
}

TEST_F(recording_test, null_or_empty_buffer_write)
{
    port_output_recording rec_port("rec_null_buffer");
    rec_port.set_controller(&controller::get());

    auto r_params = rec_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    r_params->get<configuration_parameter_string>("data_format"_ct)->set_value("pcap"_ct);
    r_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("single"_ct);
    r_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(temp_dir));

    // Writing before start should return false
    EXPECT_FALSE(rec_port.write(nullptr));

    ASSERT_TRUE(rec_port.start());
    // Writing nullptr when started should return false safely
    EXPECT_FALSE(rec_port.write(nullptr));
    ASSERT_TRUE(rec_port.stop());
}

TEST_F(recording_test, invalid_path_start)
{
    port_output_recording rec_port("rec_invalid_path");
    rec_port.set_controller(&controller::get());

    auto r_params = rec_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    r_params->get<configuration_parameter_string>("data_format"_ct)->set_value("pcap"_ct);
    r_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("single"_ct);
    r_params->get<configuration_parameter_string>("path"_ct)->set_value("invalid_dir/non_existent_subdir/nested_dir"_ct);

    // Starting with an impossible path should fail gracefully
    EXPECT_FALSE(rec_port.start());
}
