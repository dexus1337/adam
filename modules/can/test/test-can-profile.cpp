#include <gtest/gtest.h>
#include "data/can-types.hpp"
#include "data/can-profile.hpp"
#include "data/can-parser.hpp"
#include "data/can-analyzer.hpp"
#include "data/profiles/mercedes/w209/w209-can-b.hpp"
#include "data/profiles/mercedes/w209/w209-can-c.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "memory/buffer/buffer.hpp"
#include <vector>
#include <cstring>

/**
 * @file    test-can-profile.cpp
 * @author  dexus1337
 * @brief   Unit tests for CAN profiles, signal extraction, parser, and analyzer.
 * @version 1.0
 * @date    24.08.2026
 */

using namespace adam::modules::can;
using namespace adam::string_hashed_ct_literals;

class can_profile_test : public ::testing::Test
{
protected:
    void SetUp() override
    {
        adam::buffer_manager::get().initialize();
    }

    void TearDown() override
    {
        adam::buffer_manager::get().destroy();
    }
};

TEST_F(can_profile_test, w209_can_b_profile_lookup_and_signals)
{
    const auto& profile = profiles::mercedes::w209::can_b::get_profile();
    EXPECT_EQ(profile.get_name(), "Mercedes W209 CAN-B"_ct);

    // Lookup EZS_A10 (0x010A)
    const auto* msg_ezs_a10 = profile.find_message(0x010A, false);
    ASSERT_NE(msg_ezs_a10, nullptr);
    EXPECT_STREQ(msg_ezs_a10->ecu_name, "EZS_A10");
    EXPECT_EQ(msg_ezs_a10->signal_count, 6);

    // Verify 0-indexed signal access
    const auto* sig0 = msg_ezs_a10->get_signal(0);
    ASSERT_NE(sig0, nullptr);
    EXPECT_EQ(sig0->index, 0);
    EXPECT_STREQ(sig0->name, "RIZ_HL");
    EXPECT_EQ(sig0->bit_offset, 0);
    EXPECT_EQ(sig0->bit_length, 8);

    const auto* sig5 = msg_ezs_a10->get_signal(5);
    ASSERT_NE(sig5, nullptr);
    EXPECT_EQ(sig5->index, 5);
    EXPECT_STREQ(sig5->name, "DHL");
    EXPECT_EQ(sig5->bit_offset, 34);
    EXPECT_EQ(sig5->bit_length, 14);

    // Out of bounds signal index returns nullptr
    EXPECT_EQ(msg_ezs_a10->get_signal(6), nullptr);

    // Lookup EZS_A11 (0x0016)
    const auto* msg_ezs_a11 = profile.find_message(0x0016, false);
    ASSERT_NE(msg_ezs_a11, nullptr);
    EXPECT_STREQ(msg_ezs_a11->ecu_name, "EZS_A11");
    EXPECT_EQ(msg_ezs_a11->signal_count, 1);

    const auto* u_batt = msg_ezs_a11->get_signal(0);
    ASSERT_NE(u_batt, nullptr);
    EXPECT_EQ(u_batt->index, 0);
    EXPECT_STREQ(u_batt->name, "U_BATT");
    EXPECT_EQ(u_batt->bit_offset, 0);
    EXPECT_EQ(u_batt->bit_length, 8);
}

TEST_F(can_profile_test, signal_bit_extraction)
{
    const auto& profile = profiles::mercedes::w209::can_b::get_profile();
    const auto* msg_ezs_a10 = profile.find_message(0x010A, false);
    ASSERT_NE(msg_ezs_a10, nullptr);

    // Construct test CAN payload:
    // Byte 0: 0x2A (42) -> RIZ_HL
    // Byte 1: 0x35 (53) -> RIZ_HR
    // Bytes 2-3:
    //   Bit 16-17 (offset 16, len 2): 0x01 -> DRTGHR
    //   Bit 18-31 (offset 18, len 14): 850 (0x0352) -> DHR
    //   Byte 2: (0x01) | ((850 & 0x3F) << 2) = 1 | (0x12 << 2) = 1 | 0x48 = 0x49
    //   Byte 3: (850 >> 6) = 0x0D
    // Bytes 4-5:
    //   Bit 32-33 (offset 32, len 2): 0x02 -> DRTGHL
    //   Bit 34-47 (offset 34, len 14): 1200 (0x04B0) -> DHL
    //   Byte 4: (0x02) | ((1200 & 0x3F) << 2) = 2 | (0x30 << 2) = 2 | 0xC0 = 0xC2
    //   Byte 5: (1200 >> 6) = 0x12
    uint8_t payload[8] = { 0x2A, 0x35, 0x49, 0x0D, 0xC2, 0x12, 0x00, 0x00 };

    uint64_t v_riz_hl = extract_raw_signal(payload, 8, *msg_ezs_a10->get_signal(0));
    EXPECT_EQ(v_riz_hl, 42);

    uint64_t v_riz_hr = extract_raw_signal(payload, 8, *msg_ezs_a10->get_signal(1));
    EXPECT_EQ(v_riz_hr, 53);

    uint64_t v_drtghr = extract_raw_signal(payload, 8, *msg_ezs_a10->get_signal(2));
    EXPECT_EQ(v_drtghr, 1);

    uint64_t v_dhr = extract_raw_signal(payload, 8, *msg_ezs_a10->get_signal(3));
    EXPECT_EQ(v_dhr, 850);

    uint64_t v_drtghl = extract_raw_signal(payload, 8, *msg_ezs_a10->get_signal(4));
    EXPECT_EQ(v_drtghl, 2);

    uint64_t v_dhl = extract_raw_signal(payload, 8, *msg_ezs_a10->get_signal(5));
    EXPECT_EQ(v_dhl, 1200);
}

TEST_F(can_profile_test, w209_can_c_profile_lookup_and_signals)
{
    const auto& profile = profiles::mercedes::w209::can_c::get_profile();
    EXPECT_EQ(profile.get_name(), "Mercedes W209 CAN-C"_ct);

    // Lookup BS_208h (0x0208)
    const auto* msg_bs_208 = profile.find_message(0x0208, false);
    ASSERT_NE(msg_bs_208, nullptr);
    EXPECT_STREQ(msg_bs_208->ecu_name, "BS_208h");
    EXPECT_EQ(msg_bs_208->signal_count, 17);

    // Verify 0-indexed signal access
    const auto* sig0 = msg_bs_208->get_signal(0);
    ASSERT_NE(sig0, nullptr);
    EXPECT_EQ(sig0->index, 0);
    EXPECT_STREQ(sig0->name, "AKT_R_ESP");
    EXPECT_EQ(sig0->bit_offset, 0);
    EXPECT_EQ(sig0->bit_length, 1);

    // Lookup MS_308h (0x0308)
    const auto* msg_ms_308 = profile.find_message(0x0308, false);
    ASSERT_NE(msg_ms_308, nullptr);
    EXPECT_STREQ(msg_ms_308->ecu_name, "MS_308h");
    EXPECT_EQ(msg_ms_308->signal_count, 28);
}

TEST_F(can_profile_test, profile_pool_singleton)
{
    auto& pool = can_profile_pool::get();
    const auto* prof_b = pool.get_profile("Mercedes W209 CAN-B"_ct.get_hash());
    ASSERT_NE(prof_b, nullptr);
    EXPECT_EQ(prof_b->get_name(), "Mercedes W209 CAN-B"_ct);

    const auto* prof_c = pool.get_profile("Mercedes W209 CAN-C"_ct.get_hash());
    ASSERT_NE(prof_c, nullptr);
    EXPECT_EQ(prof_c->get_name(), "Mercedes W209 CAN-C"_ct);

    EXPECT_EQ(pool.get_default_profile(), prof_b);
}

TEST_F(can_profile_test, parser_and_analyzer_integration)
{
    // Build raw CAN buffer with 1 EZS_A10 message
    uint8_t raw_bytes[sizeof(can_message) + 8];
    std::memset(raw_bytes, 0, sizeof(raw_bytes));

    auto* msg = reinterpret_cast<can_message*>(raw_bytes);
    msg->id.bits.std_id = 0x010A;
    msg->id.bits.is_extended = 0;
    msg->dlc = 8;

    uint8_t* payload = raw_bytes + sizeof(can_message);
    payload[0] = 0x2A;
    payload[1] = 0x35;
    payload[2] = 0x49;
    payload[3] = 0x0D;
    payload[4] = 0xC2;
    payload[5] = 0x12;

    auto* buf = adam::buffer_manager::get().request_buffer(sizeof(raw_bytes));
    ASSERT_NE(buf, nullptr);
    std::memcpy(buf->data(), raw_bytes, sizeof(raw_bytes));
    buf->set_size(sizeof(raw_bytes));

    can_parser parser;
    adam::buffer* parsed_buf = nullptr;
    bool parse_ok = parser.parse(buf, parsed_buf);
    EXPECT_TRUE(parse_ok);
    ASSERT_NE(parsed_buf, nullptr);

    can_analyzer analyzer;
    std::vector<adam::analyzer::row> results;
    bool analyze_ok = analyzer.analyze(parsed_buf, results);
    EXPECT_TRUE(analyze_ok);
    ASSERT_EQ(results.size(), 1);

    // Columns: ID, ECU, Name, DLC, Signals, Data
    EXPECT_EQ(results[0].columns[0], "0x010A");
    EXPECT_EQ(results[0].columns[1], "EZS_A10");
    EXPECT_EQ(results[0].columns[2], "Rear Wheel Speed & Direction");
    EXPECT_EQ(results[0].columns[3], "8");
    EXPECT_EQ(results[0].columns[4], "6");

    // Expanded view
    std::vector<adam::analyzer::expanded_data> expansions;
    bool expand_ok = analyzer.analyze_expanded(parsed_buf->get_data_as<uint8_t>(), parsed_buf->get_size(), nullptr, 0, 0, expansions);
    EXPECT_TRUE(expand_ok);
    ASSERT_GE(expansions.size(), 1);
    EXPECT_EQ(expansions[0].data_type, adam::analyzer::expanded_data::type_table);
    EXPECT_EQ(expansions[0].table_rows.size(), 6);

    // Check first signal row: Index 0, RIZ_HL (multi-bit: 0 to 7)
    EXPECT_EQ(expansions[0].table_rows[0].columns[0], "0");
    EXPECT_EQ(expansions[0].table_rows[0].columns[1], "RIZ_HL");
    EXPECT_EQ(expansions[0].table_rows[0].columns[3], "Bits [0:7]");
    EXPECT_EQ(expansions[0].table_rows[0].columns[4], "0x2A");
    EXPECT_EQ(expansions[0].table_rows[0].columns[5], "42");
    EXPECT_EQ(expansions[0].table_rows[0].columns[6], "*");

    // Check single bit signal from 0x0000 (Central Locking)
    uint8_t raw_bytes_0[sizeof(can_message) + 8] = {};
    auto* msg_0 = reinterpret_cast<can_message*>(raw_bytes_0);
    msg_0->id.bits.std_id = 0x0000;
    msg_0->dlc = 8;
    raw_bytes_0[sizeof(can_message)] = 0x01; // KG_KL_AKT bit 0 set

    auto* buf_0 = adam::buffer_manager::get().request_buffer(sizeof(raw_bytes_0));
    ASSERT_NE(buf_0, nullptr);
    std::memcpy(buf_0->data(), raw_bytes_0, sizeof(raw_bytes_0));
    buf_0->set_size(sizeof(raw_bytes_0));

    adam::buffer* parsed_buf_0 = nullptr;
    EXPECT_TRUE(parser.parse(buf_0, parsed_buf_0));
    ASSERT_NE(parsed_buf_0, nullptr);

    std::vector<adam::analyzer::expanded_data> expansions_0;
    EXPECT_TRUE(analyzer.analyze_expanded(parsed_buf_0->get_data_as<uint8_t>(), parsed_buf_0->get_size(), nullptr, 0, 0, expansions_0));
    ASSERT_GE(expansions_0.size(), 1);
    EXPECT_EQ(expansions_0[0].table_rows[0].columns[0], "0");
    EXPECT_EQ(expansions_0[0].table_rows[0].columns[1], "KG_KL_AKT");
    EXPECT_EQ(expansions_0[0].table_rows[0].columns[3], "Bit [0]");
    EXPECT_EQ(expansions_0[0].table_rows[0].columns[4], "0x1");
    EXPECT_EQ(expansions_0[0].table_rows[0].columns[5], "1");
    EXPECT_EQ(expansions_0[0].table_rows[0].columns[6], ".");

    buf_0->release();
    parsed_buf_0->release();
    buf->release();
    parsed_buf->release();
}

TEST_F(can_profile_test, profile_endianness_setting)
{
    static const can_signal_spec test_signals[] =
    {
        { 0, "SIG_16", "16-bit signal", 0, 16 }
    };

    static const can_message_spec test_messages[] =
    {
        { 0x0123, false, 8, "TEST_ECU", "Test Message", test_signals, 1 }
    };

    can_profile prof_le
    (
        "Test LE"_ct,
        "Test Little Endian Profile"_ct,
        test_messages,
        1,
        can_profile::little_endian
    );

    EXPECT_EQ(prof_le.get_endianness(), can_profile::little_endian);

    prof_le.set_endianness(can_profile::big_endian);
    EXPECT_EQ(prof_le.get_endianness(), can_profile::big_endian);

    can_profile prof_be
    (
        "Test BE"_ct,
        "Test Big Endian Profile"_ct,
        test_messages,
        1,
        can_profile::big_endian
    );

    EXPECT_EQ(prof_be.get_endianness(), can_profile::big_endian);
}

TEST_F(can_profile_test, endianness_signal_extraction)
{
    // 16-bit signal in bytes 0-1
    can_signal_spec sig_16 = { 0, "VAL_16", "16-bit", 0, 16 };
    // 32-bit signal in bytes 2-5
    can_signal_spec sig_32 = { 1, "VAL_32", "32-bit", 16, 32 };
    // 14-bit signal spanning byte 6 (6 bits: 50..55) and byte 7 (8 bits: 56..63) -> bits 50..63
    can_signal_spec sig_14 = { 2, "VAL_14", "14-bit", 50, 14 };

    uint8_t payload[8] =
    {
        0x12, 0x34,                         // Bytes 0-1
        0x01, 0x02, 0x03, 0x04,             // Bytes 2-5
        0x34, 0x49                          // Bytes 6-7: (0x0D << 2) = 0x34 in byte 6, 0x49 in byte 7
    };

    // Little endian interpretation:
    // Byte 0 is LSB, Byte 1 is MSB
    uint64_t val_16_le = extract_raw_signal(payload, 8, sig_16, can_profile::little_endian);
    EXPECT_EQ(val_16_le, 0x3412);

    // Big endian interpretation:
    // Byte 0 is MSB, Byte 1 is LSB
    uint64_t val_16_be = extract_raw_signal(payload, 8, sig_16, can_profile::big_endian);
    EXPECT_EQ(val_16_be, 0x1234);

    // 32-bit little endian
    uint64_t val_32_le = extract_raw_signal(payload, 8, sig_32, can_profile::little_endian);
    EXPECT_EQ(val_32_le, 0x04030201);

    // 32-bit big endian
    uint64_t val_32_be = extract_raw_signal(payload, 8, sig_32, can_profile::big_endian);
    EXPECT_EQ(val_32_be, 0x01020304);

    // 14-bit signal:
    // Byte 6 (start_byte): 6 bits (offset 50..55, so bit 2..7 of byte 6). Byte 6 has (0x34 >> 2) & 0x3F = 0x0D.
    // Byte 7 (end_byte): 8 bits (offset 56..63, so bit 0..7 of byte 7). Byte 7 has 0x49.
    // Little endian: (0x49 << 6) | 0x0D = 0x124D
    uint64_t val_14_le = extract_raw_signal(payload, 8, sig_14, can_profile::little_endian);
    EXPECT_EQ(val_14_le, (0x49ULL << 6) | 0x0DULL);

    // Big endian: (0x0D << 8) | 0x49 = 0x0D49
    uint64_t val_14_be = extract_raw_signal(payload, 8, sig_14, can_profile::big_endian);
    EXPECT_EQ(val_14_be, 0x0D49);
}

TEST_F(can_profile_test, analyzer_with_big_endian_profile)
{
    static const can_signal_spec be_signals[] =
    {
        { 0, "RPM", "Engine Speed", 0, 16 }
    };

    static const can_message_spec be_messages[] =
    {
        { 0x0300, false, 8, "ENGINE", "Speed Data", be_signals, 1 }
    };

    can_profile be_profile
    (
        "BE Engine"_ct,
        "Big Endian Profile"_ct,
        be_messages,
        1,
        can_profile::big_endian
    );

    uint8_t raw_bytes[sizeof(can_message) + 8] = {};
    auto* msg = reinterpret_cast<can_message*>(raw_bytes);
    msg->id.bits.std_id = 0x0300;
    msg->dlc = 8;
    // 2500 rpm = 0x09C4 in big-endian: Byte 0 = 0x09, Byte 1 = 0xC4
    raw_bytes[sizeof(can_message) + 0] = 0x09;
    raw_bytes[sizeof(can_message) + 1] = 0xC4;

    can_analyzer analyzer(&be_profile);
    EXPECT_EQ(analyzer.get_profile(), &be_profile);

    std::vector<adam::analyzer::expanded_data> expansions;
    bool ok = analyzer.analyze_expanded(raw_bytes, sizeof(raw_bytes), nullptr, 0, 0, expansions);
    EXPECT_TRUE(ok);
    ASSERT_EQ(expansions.size(), 1);
    ASSERT_EQ(expansions[0].table_rows.size(), 1);

    // Verify 0x09C4 (2500) decoded properly instead of 0xC409
    EXPECT_EQ(expansions[0].table_rows[0].columns[1], "RPM");
    EXPECT_EQ(expansions[0].table_rows[0].columns[4], "0x9C4");
    EXPECT_EQ(expansions[0].table_rows[0].columns[5], "2500");
}

