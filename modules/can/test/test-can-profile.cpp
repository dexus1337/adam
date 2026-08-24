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

TEST_F(can_profile_test, w209_profile_lookup_and_signals)
{
    const auto& profile = profiles::mercedes::w209::get_profile();
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
    const auto& profile = profiles::mercedes::w209::get_profile();
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
    const auto& profile = profiles::mercedes::w209::get_can_c_profile();
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
    EXPECT_EQ(msg_ms_308->signal_count, 27);
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

