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
#include <algorithm>

using namespace adam;
using namespace adam::modules::recrep;

class replay_test : public ::testing::Test
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

    std::string temp_dir = "replay_test_temp";

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

TEST_F(replay_test, format_any)
{
    // Write PCAP
    {
        port_output_recording rec_port("rec_any_pcap");
        rec_port.set_controller(&controller::get());
        auto r_params = rec_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
        r_params->get<configuration_parameter_string>("data_format"_ct)->set_value("pcap"_ct);
        r_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("single"_ct);
        r_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(temp_dir));
        ASSERT_TRUE(rec_port.start());
        write_test_data(rec_port, 5, 1000000000ull, 1000000ull);
        ASSERT_TRUE(rec_port.stop());
    }

    // Write RFF
    {
        port_output_recording rec_port("rec_any_rff");
        rec_port.set_controller(&controller::get());
        auto r_params = rec_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
        r_params->get<configuration_parameter_string>("data_format"_ct)->set_value("rff"_ct);
        r_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("single"_ct);
        r_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(temp_dir));
        ASSERT_TRUE(rec_port.start());
        write_test_data(rec_port, 5, 2000000000ull, 1000000ull);
        ASSERT_TRUE(rec_port.stop());
    }

    std::string pcap_file, rff_file;
    for (const auto& entry : std::filesystem::directory_iterator(temp_dir))
    {
        if (entry.path().extension() == ".pcap") pcap_file = entry.path().string();
        if (entry.path().extension() == ".rff") rff_file = entry.path().string();
    }

    // Read Any - PCAP
    {
        port_input_replay rep_port("rep_any_pcap");
        rep_port.set_controller(&controller::get());
        auto i_params = rep_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
        i_params->get<configuration_parameter_string>("data_format"_ct)->set_value("any"_ct);
        i_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("single_file"_ct);
        i_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(pcap_file));
        i_params->get<configuration_parameter_double>("speed"_ct)->set_value(0.0);
        ASSERT_TRUE(rep_port.start());
        read_and_verify(rep_port, 5);
        ASSERT_TRUE(rep_port.stop());
    }

    // Read Any - RFF
    {
        port_input_replay rep_port("rep_any_rff");
        rep_port.set_controller(&controller::get());
        auto i_params = rep_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
        i_params->get<configuration_parameter_string>("data_format"_ct)->set_value("any"_ct);
        i_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("single_file"_ct);
        i_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(rff_file));
        i_params->get<configuration_parameter_double>("speed"_ct)->set_value(0.0);
        ASSERT_TRUE(rep_port.start());
        read_and_verify(rep_port, 5);
        ASSERT_TRUE(rep_port.stop());
    }
}

TEST_F(replay_test, mode_multiple_files)
{
    // Write two PCAP files
    port_output_recording rec1("rec1_mult"), rec2("rec2_mult");
    rec1.set_controller(&controller::get());
    rec2.set_controller(&controller::get());

    auto r1 = rec1.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    r1->get<configuration_parameter_string>("data_format"_ct)->set_value("pcap"_ct);
    r1->get<configuration_parameter_string>("file_mode"_ct)->set_value("single"_ct);
    r1->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(temp_dir));
    
    auto r2 = rec2.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    r2->get<configuration_parameter_string>("data_format"_ct)->set_value("pcap"_ct);
    r2->get<configuration_parameter_string>("file_mode"_ct)->set_value("single"_ct);
    r2->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(temp_dir));

    ASSERT_TRUE(rec1.start());
    write_test_data(rec1, 5, 1000000000ull, 1000000ull);
    ASSERT_TRUE(rec1.stop());

    std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Different filename timestamp

    ASSERT_TRUE(rec2.start());
    for (int i = 5; i < 10; ++i)
    {
        buffer* buf = buffer_manager::get().request_buffer(64);
        ASSERT_NE(buf, nullptr);
        std::string text = "Test packet " + std::to_string(i);
        std::memcpy(buf->data_as<char>(), text.c_str(), text.size() + 1);
        buf->set_size(static_cast<uint32_t>(text.size() + 1));
        buf->set_timestamp(2000000000ull + i * 1000000ull);
        EXPECT_TRUE(rec2.write(buf));
        buf->release();
    }
    ASSERT_TRUE(rec2.stop());

    std::vector<std::string> files;
    for (const auto& entry : std::filesystem::directory_iterator(temp_dir))
    {
        if (entry.path().extension() == ".pcap") files.push_back(entry.path().string());
    }
    ASSERT_EQ(files.size(), 2);
    // Sort to ensure sequential
    std::sort(files.begin(), files.end());
    std::string concat_path = files[0] + ";" + files[1];

    port_input_replay rep_port("rep_mult");
    rep_port.set_controller(&controller::get());
    auto i_params = rep_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    i_params->get<configuration_parameter_string>("data_format"_ct)->set_value("pcap"_ct);
    i_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("multiple_files"_ct);
    i_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(concat_path));
    i_params->get<configuration_parameter_double>("speed"_ct)->set_value(0.0);

    ASSERT_TRUE(rep_port.start());
    read_and_verify(rep_port, 10);
    ASSERT_TRUE(rep_port.stop());
}

TEST_F(replay_test, mode_directory)
{
    // Write two PCAP files
    port_output_recording rec1("rec1_dir"), rec2("rec2_dir");
    rec1.set_controller(&controller::get());
    rec2.set_controller(&controller::get());

    auto r1 = rec1.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    r1->get<configuration_parameter_string>("data_format"_ct)->set_value("pcap"_ct);
    r1->get<configuration_parameter_string>("file_mode"_ct)->set_value("single"_ct);
    r1->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(temp_dir));
    
    auto r2 = rec2.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    r2->get<configuration_parameter_string>("data_format"_ct)->set_value("pcap"_ct);
    r2->get<configuration_parameter_string>("file_mode"_ct)->set_value("single"_ct);
    r2->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(temp_dir));

    ASSERT_TRUE(rec1.start());
    write_test_data(rec1, 5, 1000000000ull, 1000000ull);
    ASSERT_TRUE(rec1.stop());

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    ASSERT_TRUE(rec2.start());
    for (int i = 5; i < 10; ++i)
    {
        buffer* buf = buffer_manager::get().request_buffer(64);
        ASSERT_NE(buf, nullptr);
        std::string text = "Test packet " + std::to_string(i);
        std::memcpy(buf->data_as<char>(), text.c_str(), text.size() + 1);
        buf->set_size(static_cast<uint32_t>(text.size() + 1));
        buf->set_timestamp(2000000000ull + i * 1000000ull);
        EXPECT_TRUE(rec2.write(buf));
        buf->release();
    }
    ASSERT_TRUE(rec2.stop());

    port_input_replay rep_port("rep_dir");
    rep_port.set_controller(&controller::get());
    auto i_params = rep_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    i_params->get<configuration_parameter_string>("data_format"_ct)->set_value("pcap"_ct);
    i_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("directory"_ct);
    i_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(temp_dir));
    i_params->get<configuration_parameter_double>("speed"_ct)->set_value(0.0);

    ASSERT_TRUE(rep_port.start());
    read_and_verify(rep_port, 10);
    ASSERT_TRUE(rep_port.stop());
}
