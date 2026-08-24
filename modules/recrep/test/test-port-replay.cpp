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
#include <algorithm>
#include <fstream>
#include <mutex>
#include <condition_variable>

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
        if (entry.path().extension() == ".pcap")
        {
            pcap_file = entry.path().string();
        }
        if (entry.path().extension() == ".rff")
        {
            rff_file = entry.path().string();
        }
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
        read_and_verify(rep_port, 5);
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
        read_and_verify(rep_port, 5);
    }
}

TEST_F(replay_test, mode_single_file_pcap)
{
    port_output_recording rec_port("rec_single_pcap");
    rec_port.set_controller(&controller::get());
    auto r_params = rec_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    r_params->get<configuration_parameter_string>("data_format"_ct)->set_value("pcap"_ct);
    r_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("single"_ct);
    r_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(temp_dir));
    ASSERT_TRUE(rec_port.start());
    write_test_data(rec_port, 8, 1000000000ull, 1000000ull);
    ASSERT_TRUE(rec_port.stop());

    std::string pcap_file;
    for (const auto& entry : std::filesystem::directory_iterator(temp_dir))
    {
        if (entry.path().extension() == ".pcap")
        {
            pcap_file = entry.path().string();
        }
    }

    port_input_replay rep_port("rep_single_pcap");
    rep_port.set_controller(&controller::get());
    auto i_params = rep_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    i_params->get<configuration_parameter_string>("data_format"_ct)->set_value("pcap"_ct);
    i_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("single_file"_ct);
    i_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(pcap_file));
    i_params->get<configuration_parameter_double>("speed"_ct)->set_value(0.0);

    read_and_verify(rep_port, 8);
}

TEST_F(replay_test, mode_single_file_rff)
{
    port_output_recording rec_port("rec_single_rff");
    rec_port.set_controller(&controller::get());
    auto r_params = rec_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    r_params->get<configuration_parameter_string>("data_format"_ct)->set_value("rff"_ct);
    r_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("single"_ct);
    r_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(temp_dir));
    ASSERT_TRUE(rec_port.start());
    write_test_data(rec_port, 8, 1000000000ull, 1000000ull);
    ASSERT_TRUE(rec_port.stop());

    std::string rff_file;
    for (const auto& entry : std::filesystem::directory_iterator(temp_dir))
    {
        if (entry.path().extension() == ".rff")
        {
            rff_file = entry.path().string();
        }
    }

    port_input_replay rep_port("rep_single_rff");
    rep_port.set_controller(&controller::get());
    auto i_params = rep_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    i_params->get<configuration_parameter_string>("data_format"_ct)->set_value("rff"_ct);
    i_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("single_file"_ct);
    i_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(rff_file));
    i_params->get<configuration_parameter_double>("speed"_ct)->set_value(0.0);

    read_and_verify(rep_port, 8);
}

TEST_F(replay_test, mode_multiple_files_pcap)
{
    // Write two PCAP files
    port_output_recording rec1("rec1_mult_pcap"), rec2("rec2_mult_pcap");
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

    std::vector<std::string> files;
    for (const auto& entry : std::filesystem::directory_iterator(temp_dir))
    {
        if (entry.path().extension() == ".pcap")
        {
            files.push_back(entry.path().string());
        }
    }
    ASSERT_EQ(files.size(), 2u);
    std::sort(files.begin(), files.end());
    std::string concat_path = files[0] + ";" + files[1];

    port_input_replay rep_port("rep_mult_pcap");
    rep_port.set_controller(&controller::get());
    auto i_params = rep_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    i_params->get<configuration_parameter_string>("data_format"_ct)->set_value("pcap"_ct);
    i_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("multiple_files"_ct);
    i_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(concat_path));
    i_params->get<configuration_parameter_double>("speed"_ct)->set_value(0.0);

    read_and_verify(rep_port, 10);
}

TEST_F(replay_test, mode_multiple_files_rff)
{
    // Write two RFF files
    port_output_recording rec1("rec1_mult_rff"), rec2("rec2_mult_rff");
    rec1.set_controller(&controller::get());
    rec2.set_controller(&controller::get());

    auto r1 = rec1.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    r1->get<configuration_parameter_string>("data_format"_ct)->set_value("rff"_ct);
    r1->get<configuration_parameter_string>("file_mode"_ct)->set_value("single"_ct);
    r1->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(temp_dir));
    
    auto r2 = rec2.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    r2->get<configuration_parameter_string>("data_format"_ct)->set_value("rff"_ct);
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

    std::vector<std::string> files;
    for (const auto& entry : std::filesystem::directory_iterator(temp_dir))
    {
        if (entry.path().extension() == ".rff")
        {
            files.push_back(entry.path().string());
        }
    }
    ASSERT_EQ(files.size(), 2u);
    std::sort(files.begin(), files.end());
    std::string concat_path = files[0] + ";" + files[1];

    port_input_replay rep_port("rep_mult_rff");
    rep_port.set_controller(&controller::get());
    auto i_params = rep_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    i_params->get<configuration_parameter_string>("data_format"_ct)->set_value("rff"_ct);
    i_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("multiple_files"_ct);
    i_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(concat_path));
    i_params->get<configuration_parameter_double>("speed"_ct)->set_value(0.0);

    read_and_verify(rep_port, 10);
}

TEST_F(replay_test, mode_directory_pcap)
{
    port_output_recording rec1("rec1_dir_pcap"), rec2("rec2_dir_pcap");
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

    port_input_replay rep_port("rep_dir_pcap");
    rep_port.set_controller(&controller::get());
    auto i_params = rep_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    i_params->get<configuration_parameter_string>("data_format"_ct)->set_value("pcap"_ct);
    i_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("directory"_ct);
    i_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(temp_dir));
    i_params->get<configuration_parameter_double>("speed"_ct)->set_value(0.0);

    read_and_verify(rep_port, 10);
}

TEST_F(replay_test, mode_directory_rff)
{
    port_output_recording rec1("rec1_dir_rff"), rec2("rec2_dir_rff");
    rec1.set_controller(&controller::get());
    rec2.set_controller(&controller::get());

    auto r1 = rec1.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    r1->get<configuration_parameter_string>("data_format"_ct)->set_value("rff"_ct);
    r1->get<configuration_parameter_string>("file_mode"_ct)->set_value("single"_ct);
    r1->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(temp_dir));
    
    auto r2 = rec2.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    r2->get<configuration_parameter_string>("data_format"_ct)->set_value("rff"_ct);
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

    port_input_replay rep_port("rep_dir_rff");
    rep_port.set_controller(&controller::get());
    auto i_params = rep_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    i_params->get<configuration_parameter_string>("data_format"_ct)->set_value("rff"_ct);
    i_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("directory"_ct);
    i_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(temp_dir));
    i_params->get<configuration_parameter_double>("speed"_ct)->set_value(0.0);

    read_and_verify(rep_port, 10);
}

TEST_F(replay_test, mode_loop_pcap)
{
    port_output_recording rec_port("rec_loop_pcap");
    rec_port.set_controller(&controller::get());
    auto r_params = rec_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    r_params->get<configuration_parameter_string>("data_format"_ct)->set_value("pcap"_ct);
    r_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("single"_ct);
    r_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(temp_dir));
    ASSERT_TRUE(rec_port.start());
    write_test_data(rec_port, 4, 1000000000ull, 1000000ull);
    ASSERT_TRUE(rec_port.stop());

    std::string pcap_file;
    for (const auto& entry : std::filesystem::directory_iterator(temp_dir))
    {
        if (entry.path().extension() == ".pcap")
        {
            pcap_file = entry.path().string();
        }
    }

    port_input_replay rep_port("rep_loop_pcap");
    rep_port.set_controller(&controller::get());
    auto i_params = rep_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    i_params->get<configuration_parameter_string>("data_format"_ct)->set_value("pcap"_ct);
    i_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("single_file"_ct);
    i_params->get<configuration_parameter_string>("mode"_ct)->set_value("loop"_ct);
    i_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(pcap_file));
    i_params->get<configuration_parameter_double>("speed"_ct)->set_value(0.0);

    test_buffer_collector collector(rep_port);
    ASSERT_TRUE(rep_port.start());

    // Read 3 full loops (12 packets total from 4-packet file)
    size_t target_packets = 12;
    auto received = collector.wait_for_buffers(target_packets);
    ASSERT_TRUE(rep_port.stop());

    ASSERT_GE(received.size(), target_packets);
    for (size_t i = 0; i < target_packets; ++i)
    {
        int packet_idx = static_cast<int>(i % 4);
        std::string expected_text = "Test packet " + std::to_string(packet_idx);
        EXPECT_STREQ(received[i].payload.c_str(), expected_text.c_str());
    }
}

TEST_F(replay_test, mode_loop_rff)
{
    port_output_recording rec_port("rec_loop_rff");
    rec_port.set_controller(&controller::get());
    auto r_params = rec_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    r_params->get<configuration_parameter_string>("data_format"_ct)->set_value("rff"_ct);
    r_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("single"_ct);
    r_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(temp_dir));
    ASSERT_TRUE(rec_port.start());
    write_test_data(rec_port, 4, 1000000000ull, 1000000ull);
    ASSERT_TRUE(rec_port.stop());

    std::string rff_file;
    for (const auto& entry : std::filesystem::directory_iterator(temp_dir))
    {
        if (entry.path().extension() == ".rff")
        {
            rff_file = entry.path().string();
        }
    }

    port_input_replay rep_port("rep_loop_rff");
    rep_port.set_controller(&controller::get());
    auto i_params = rep_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    i_params->get<configuration_parameter_string>("data_format"_ct)->set_value("rff"_ct);
    i_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("single_file"_ct);
    i_params->get<configuration_parameter_string>("mode"_ct)->set_value("loop"_ct);
    i_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(rff_file));
    i_params->get<configuration_parameter_double>("speed"_ct)->set_value(0.0);

    test_buffer_collector collector(rep_port);
    ASSERT_TRUE(rep_port.start());

    // Read 3 full loops (12 packets total from 4-packet file)
    size_t target_packets = 12;
    auto received = collector.wait_for_buffers(target_packets);
    ASSERT_TRUE(rep_port.stop());

    ASSERT_GE(received.size(), target_packets);
    for (size_t i = 0; i < target_packets; ++i)
    {
        int packet_idx = static_cast<int>(i % 4);
        std::string expected_text = "Test packet " + std::to_string(packet_idx);
        EXPECT_STREQ(received[i].payload.c_str(), expected_text.c_str());
    }
}

TEST_F(replay_test, timestamps_original_vs_current)
{
    port_output_recording rec_port("rec_timestamps");
    rec_port.set_controller(&controller::get());
    auto r_params = rec_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    r_params->get<configuration_parameter_string>("data_format"_ct)->set_value("pcap"_ct);
    r_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("single"_ct);
    r_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(temp_dir));
    ASSERT_TRUE(rec_port.start());
    write_test_data(rec_port, 2, 1000000000ull, 1000000ull);
    ASSERT_TRUE(rec_port.stop());

    std::string pcap_file;
    for (const auto& entry : std::filesystem::directory_iterator(temp_dir))
    {
        if (entry.path().extension() == ".pcap")
        {
            pcap_file = entry.path().string();
        }
    }

    // Replay with original timestamps
    {
        port_input_replay rep_port("rep_orig_ts");
        rep_port.set_controller(&controller::get());
        auto i_params = rep_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
        i_params->get<configuration_parameter_string>("data_format"_ct)->set_value("pcap"_ct);
        i_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("single_file"_ct);
        i_params->get<configuration_parameter_string>("timestamps"_ct)->set_value("original"_ct);
        i_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(pcap_file));
        i_params->get<configuration_parameter_double>("speed"_ct)->set_value(0.0);

        test_buffer_collector collector(rep_port);
        ASSERT_TRUE(rep_port.start());
        auto received = collector.wait_for_buffers(2);
        ASSERT_TRUE(rep_port.stop());

        ASSERT_EQ(received.size(), 2u);
        EXPECT_GT(received[0].timestamp, 0u);
        EXPECT_GT(received[1].timestamp, 0u);
    }

    // Replay with current timestamps
    {
        port_input_replay rep_port("rep_curr_ts");
        rep_port.set_controller(&controller::get());
        auto i_params = rep_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
        i_params->get<configuration_parameter_string>("data_format"_ct)->set_value("pcap"_ct);
        i_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("single_file"_ct);
        i_params->get<configuration_parameter_string>("timestamps"_ct)->set_value("current"_ct);
        i_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(pcap_file));
        i_params->get<configuration_parameter_double>("speed"_ct)->set_value(0.0);

        test_buffer_collector collector(rep_port);
        ASSERT_TRUE(rep_port.start());
        auto received = collector.wait_for_buffers(2);
        ASSERT_TRUE(rep_port.stop());

        ASSERT_EQ(received.size(), 2u);
        EXPECT_GT(received[0].timestamp, 0u);
        EXPECT_GT(received[1].timestamp, 0u);
    }
}

TEST_F(replay_test, speed_playback)
{
    port_output_recording rec_port("rec_speed");
    rec_port.set_controller(&controller::get());
    auto r_params = rec_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    r_params->get<configuration_parameter_string>("data_format"_ct)->set_value("pcap"_ct);
    r_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("single"_ct);
    r_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(temp_dir));
    ASSERT_TRUE(rec_port.start());
    // 3 packets with 1ms intervals
    write_test_data(rec_port, 3, 1000000000ull, 1000000ull);
    ASSERT_TRUE(rec_port.stop());

    std::string pcap_file;
    for (const auto& entry : std::filesystem::directory_iterator(temp_dir))
    {
        if (entry.path().extension() == ".pcap")
        {
            pcap_file = entry.path().string();
        }
    }

    port_input_replay rep_port("rep_speed");
    rep_port.set_controller(&controller::get());
    auto i_params = rep_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    i_params->get<configuration_parameter_string>("data_format"_ct)->set_value("pcap"_ct);
    i_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("single_file"_ct);
    i_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(pcap_file));
    i_params->get<configuration_parameter_double>("speed"_ct)->set_value(10.0); // 10x speed

    read_and_verify(rep_port, 3);
}

TEST_F(replay_test, corrupted_or_empty_file_handling)
{
    // 1. Completely empty file
    std::string empty_file = (std::filesystem::path(temp_dir) / "empty.pcap").string();
    {
        std::ofstream ofs(empty_file, std::ios::binary);
    }

    // 2. Corrupt header file (garbage bytes)
    std::string corrupt_file = (std::filesystem::path(temp_dir) / "corrupt.pcap").string();
    {
        std::ofstream ofs(corrupt_file, std::ios::binary);
        ofs << "INVALID_PCAP_GARBAGE_HEADER_DATA";
    }

    port_input_replay rep_port("rep_corrupt");
    rep_port.set_controller(&controller::get());
    auto i_params = rep_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    i_params->get<configuration_parameter_string>("data_format"_ct)->set_value("pcap"_ct);
    i_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("single_file"_ct);
    i_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(empty_file));

    // Starting on empty file should fail gracefully
    EXPECT_FALSE(rep_port.start());

    i_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(corrupt_file));
    // Starting on corrupt file should fail gracefully
    EXPECT_FALSE(rep_port.start());
}

TEST_F(replay_test, non_existent_file_start)
{
    port_input_replay rep_port("rep_nonexistent");
    rep_port.set_controller(&controller::get());
    auto i_params = rep_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    i_params->get<configuration_parameter_string>("data_format"_ct)->set_value("pcap"_ct);
    i_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("single_file"_ct);
    i_params->get<configuration_parameter_string>("path"_ct)->set_value("non_existent_file_12345.pcap"_ct);

    EXPECT_FALSE(rep_port.start());
}

TEST_F(replay_test, state_buffer_metadata)
{
    port_output_recording rec_port("rec_meta");
    rec_port.set_controller(&controller::get());
    auto r_params = rec_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    r_params->get<configuration_parameter_string>("data_format"_ct)->set_value("pcap"_ct);
    r_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("single"_ct);
    r_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(temp_dir));
    ASSERT_TRUE(rec_port.start());
    write_test_data(rec_port, 5, 1000000000ull, 1000000ull);
    ASSERT_TRUE(rec_port.stop());

    std::string pcap_file;
    for (const auto& entry : std::filesystem::directory_iterator(temp_dir))
    {
        if (entry.path().extension() == ".pcap")
        {
            pcap_file = entry.path().string();
        }
    }

    port_input_replay rep_port("rep_meta");
    rep_port.set_controller(&controller::get());
    auto i_params = rep_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    i_params->get<configuration_parameter_string>("data_format"_ct)->set_value("pcap"_ct);
    i_params->get<configuration_parameter_string>("file_mode"_ct)->set_value("single_file"_ct);
    i_params->get<configuration_parameter_string>("path"_ct)->set_value(adam::string_hashed(pcap_file));
    i_params->get<configuration_parameter_double>("speed"_ct)->set_value(0.0);

    test_buffer_collector collector(rep_port);
    ASSERT_TRUE(rep_port.start());
    
    auto* state_data = rep_port.get_state_buffer()->data_as<port_input_replay::replay_state_buffer_data>();
    ASSERT_NE(state_data, nullptr);
    EXPECT_GT(strlen(state_data->file_name), 0u);
    EXPECT_GT(state_data->file_time_start, 0u);
    EXPECT_GE(state_data->file_time_end, state_data->file_time_start);

    auto received = collector.wait_for_buffers(5);
    ASSERT_TRUE(rep_port.stop());

    ASSERT_EQ(received.size(), 5u);
    for (size_t i = 0; i < 5; ++i)
    {
        std::string expected_text = "Test packet " + std::to_string(i);
        EXPECT_STREQ(received[i].payload.c_str(), expected_text.c_str());
    }
}
