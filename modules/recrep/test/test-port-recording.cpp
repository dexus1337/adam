#include <gtest/gtest.h>
#include "data/port-types/port-output-recording.hpp"
#include "data/port-types/port-input-replay.hpp"
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

    void read_and_verify(port_input_replay& port, int expected_buffers)
    {
        int read_count = 0;
        buffer* buf = nullptr;
        while (port.read(buf))
        {
            if (buf)
            {
                std::string text = "Test packet " + std::to_string(read_count);
                EXPECT_STREQ(buf->data_as<const char>(), text.c_str());
                buf->release();
                read_count++;
            }
        }
        EXPECT_EQ(read_count, expected_buffers);
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
    write_test_data(rec_port, 10, 1000000000ull, 1000000ull); // start 1s, interval 1ms
    ASSERT_TRUE(rec_port.stop());

    bool found = false;
    for (const auto& entry : std::filesystem::directory_iterator(temp_dir))
    {
        if (entry.path().extension() == ".pcap") found = true;
    }
    ASSERT_TRUE(found);

    port_input_replay rep_port("rep_pcap_single");
    rep_port.set_controller(&controller::get());

    auto i_params = rep_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    i_params->get<configuration_parameter_string>("data_format"_ct)->set_value("pcap"_ct);
    i_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("single_file"_ct);
    
    std::string file_path;
    for (const auto& entry : std::filesystem::directory_iterator(temp_dir))
    {
        if (entry.path().extension() == ".pcap") file_path = entry.path().string();
    }
    i_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(file_path));
    i_params->get<configuration_parameter_double>("speed"_ct)->set_value(0.0);

    ASSERT_TRUE(rep_port.start());
    read_and_verify(rep_port, 10);
    ASSERT_TRUE(rep_port.stop());
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

    port_input_replay rep_port("rep_rff_single");
    rep_port.set_controller(&controller::get());

    auto i_params = rep_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    i_params->get<configuration_parameter_string>("data_format"_ct)->set_value("rff"_ct);
    i_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("single_file"_ct);
    
    std::string file_path;
    for (const auto& entry : std::filesystem::directory_iterator(temp_dir))
    {
        if (entry.path().extension() == ".rff") file_path = entry.path().string();
    }
    i_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(file_path));
    i_params->get<configuration_parameter_double>("speed"_ct)->set_value(0.0);

    ASSERT_TRUE(rep_port.start());
    read_and_verify(rep_port, 10);
    ASSERT_TRUE(rep_port.stop());
}

TEST_F(recording_test, chunked_by_size)
{
    port_output_recording rec_port("rec_chunk_size");
    rec_port.set_controller(&controller::get());
    
    auto r_params = rec_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    r_params->get<configuration_parameter_string>("data_format"_ct)->set_value("pcap"_ct);
    r_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("chunked"_ct);
    r_params->get<configuration_parameter_string>("chunk_mode"_ct)->set_value("size"_ct);
    r_params->get<configuration_parameter_integer>("chunk_size"_ct)->set_value(1); // 1 MB
    r_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(temp_dir));

    ASSERT_TRUE(rec_port.start());
    
    int num_buffers = 20;
    int data_size = 100000; // ~100KB per buffer
    
    std::vector<char> large_data(data_size, 'A');
    for (int i = 0; i < num_buffers; ++i)
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
        if (entry.path().extension() == ".pcap") file_count++;
    }
    EXPECT_GE(file_count, 2);

    port_input_replay rep_port("rep_chunk_size");
    rep_port.set_controller(&controller::get());
    auto i_params = rep_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    i_params->get<configuration_parameter_string>("data_format"_ct)->set_value("pcap"_ct);
    i_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("directory"_ct);
    i_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(temp_dir));
    i_params->get<configuration_parameter_double>("speed"_ct)->set_value(0.0);

    ASSERT_TRUE(rep_port.start());
    
    int read_count = 0;
    buffer* buf = nullptr;
    while (rep_port.read(buf))
    {
        if (buf)
        {
            EXPECT_EQ(buf->get_size(), data_size);
            buf->release();
            read_count++;
        }
    }
    EXPECT_EQ(read_count, num_buffers);
    
    ASSERT_TRUE(rep_port.stop());
}

TEST_F(recording_test, chunked_by_time)
{
    port_output_recording rec_port("rec_chunk_time");
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
        if (entry.path().extension() == ".pcap") file_count++;
    }
    EXPECT_GE(file_count, 3); // 3-4 chunks

    port_input_replay rep_port("rep_chunk_time");
    rep_port.set_controller(&controller::get());
    auto i_params = rep_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    i_params->get<configuration_parameter_string>("data_format"_ct)->set_value("pcap"_ct);
    i_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("directory"_ct);
    i_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(temp_dir));
    i_params->get<configuration_parameter_double>("speed"_ct)->set_value(0.0);

    ASSERT_TRUE(rep_port.start());
    read_and_verify(rep_port, 5);
    ASSERT_TRUE(rep_port.stop());
}
