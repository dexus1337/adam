/**
 * @file    test-asterix-sac-sic-filter.cpp
 * @author  dexus1337
 * @brief   Unit and integration tests for asterix sac_sic_filter.
 */

#include <gtest/gtest.h>
#include "data/filters/asterix-sac-sic-filter.hpp"
#include "data/asterix-parser.hpp"
#include "data/asterix-encoder.hpp"
#include "data/asterix-internal.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "memory/buffer/buffer.hpp"
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"

#include <vector>
#include <cstring>
#include <adam-core.hpp>

using namespace adam;
using namespace adam::modules;
using namespace adam::modules::asterix;

class filter_sac_sic_test : public ::testing::Test
{
protected:
    void SetUp() override
    {
        adam::buffer_manager::get().initialize();
        parser = asterix_parser();
        encoder = asterix_encoder();
        filter = std::make_unique<sac_sic_filter>("test_sac_sic_filter"_ct);
    }

    void TearDown() override
    {
        filter.reset();
        adam::buffer_manager::get().destroy();
    }

    void set_mode(const char* mode)
    {
        auto* user_params = filter->get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
        auto* mode_param = user_params->get<configuration_parameter_string>("filter_mode"_ct);
        mode_param->set_value(string_hashed(mode));
    }

    void set_sac_sic(const char* patterns)
    {
        auto* user_params = filter->get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
        auto* sac_sic_param = user_params->get<configuration_parameter_string>("sac_sic"_ct);
        sac_sic_param->set_value(string_hashed(patterns));
    }

    struct test_sac_sic
    {
        uint8_t sac;
        uint8_t sic;
    };

    /**
     * @brief Helper to create a parsed internal Asterix buffer containing records with specified (SAC, SIC) pairs.
     * Category 34 is used, where FRN 1 is the Data Source Identifier (SAC/SIC).
     */
    adam::buffer* create_parsed_frame(const std::vector<test_sac_sic>& sac_sic_pairs, uint8_t cat = 34)
    {
        std::vector<uint8_t> raw;
        for (const auto& pair : sac_sic_pairs)
        {
            // Block Header: CAT (1B), Length (2B, big-endian) = 6 bytes (3B header + 1B FSPEC + 2B SAC/SIC)
            raw.push_back(cat);
            raw.push_back(0);
            raw.push_back(6);

            // Record: FSPEC with FRN 1 active
            raw.push_back(0x80);

            // SAC / SIC payload (2B)
            raw.push_back(pair.sac);
            raw.push_back(pair.sic);
        }

        adam::buffer* raw_buf = adam::buffer_manager::get().request_buffer(static_cast<uint32_t>(raw.size()));
        std::memcpy(raw_buf->begin_as<uint8_t>(), raw.data(), raw.size());
        raw_buf->set_size(static_cast<uint32_t>(raw.size()));

        adam::buffer* internal_data = nullptr;
        bool ok = parser.parse(raw_buf, internal_data);
        adam::buffer_manager::get().return_buffer(raw_buf);

        if (!ok) return nullptr;
        return internal_data;
    }

    asterix_parser parser;
    asterix_encoder encoder;
    std::unique_ptr<sac_sic_filter> filter;
};

// ─── Initialization & Parameter Tests ────────────────────────────────────────

TEST_F(filter_sac_sic_test, initial_properties)
{
    EXPECT_EQ(filter->get_type_name(), "asterix-sac-sic-filter"_ct);
    EXPECT_EQ(filter->get_input_format()->get_name(), "asterix"_ct);
    EXPECT_EQ(filter->get_output_format()->get_name(), "asterix"_ct);

    auto* user_params = filter->get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    ASSERT_NE(user_params, nullptr);

    auto* mode_param = user_params->get<configuration_parameter_string>("filter_mode"_ct);
    auto* sac_sic_param = user_params->get<configuration_parameter_string>("sac_sic"_ct);

    ASSERT_NE(mode_param, nullptr);
    ASSERT_NE(sac_sic_param, nullptr);

    EXPECT_EQ(mode_param->get_value(), "whitelist"_ct);
    EXPECT_EQ(sac_sic_param->get_value(), ""_ct);
}

TEST_F(filter_sac_sic_test, handle_null_buffer)
{
    adam::buffer* null_buf = nullptr;
    EXPECT_FALSE(filter->handle_data(null_buf));
}

// ─── Whitelist Filtering Tests ────────────────────────────────────────────────

TEST_F(filter_sac_sic_test, whitelist_exact_match)
{
    set_mode("whitelist");
    set_sac_sic("103/200");

    adam::buffer* buf = create_parsed_frame({{103, 200}, {103, 63}, {104, 200}});
    ASSERT_NE(buf, nullptr);

    bool result = filter->handle_data(buf);
    EXPECT_TRUE(result);

    auto* root_frame = buf->begin_as<frame>();
    EXPECT_TRUE(root_frame->is_modified());

    auto it = root_frame->begin();
    EXPECT_FALSE(it->is_removed()); // 103/200 kept
    ++it;
    EXPECT_TRUE(it->is_removed());  // 103/63 removed
    ++it;
    EXPECT_TRUE(it->is_removed());  // 104/200 removed

    adam::buffer_manager::get().return_buffer(buf);
}

TEST_F(filter_sac_sic_test, whitelist_wildcard_sic)
{
    set_mode("whitelist");
    set_sac_sic("103/x");

    adam::buffer* buf = create_parsed_frame({{103, 200}, {103, 63}, {104, 63}});
    ASSERT_NE(buf, nullptr);

    bool result = filter->handle_data(buf);
    EXPECT_TRUE(result);

    auto* root_frame = buf->begin_as<frame>();
    EXPECT_TRUE(root_frame->is_modified());

    auto it = root_frame->begin();
    EXPECT_FALSE(it->is_removed()); // 103/200 kept
    ++it;
    EXPECT_FALSE(it->is_removed()); // 103/63 kept
    ++it;
    EXPECT_TRUE(it->is_removed());  // 104/63 removed

    adam::buffer_manager::get().return_buffer(buf);
}

TEST_F(filter_sac_sic_test, whitelist_wildcard_sac)
{
    set_mode("whitelist");
    set_sac_sic("x/63");

    adam::buffer* buf = create_parsed_frame({{103, 63}, {104, 63}, {103, 200}});
    ASSERT_NE(buf, nullptr);

    bool result = filter->handle_data(buf);
    EXPECT_TRUE(result);

    auto* root_frame = buf->begin_as<frame>();
    EXPECT_TRUE(root_frame->is_modified());

    auto it = root_frame->begin();
    EXPECT_FALSE(it->is_removed()); // 103/63 kept
    ++it;
    EXPECT_FALSE(it->is_removed()); // 104/63 kept
    ++it;
    EXPECT_TRUE(it->is_removed());  // 103/200 removed

    adam::buffer_manager::get().return_buffer(buf);
}

TEST_F(filter_sac_sic_test, whitelist_wildcard_all)
{
    set_mode("whitelist");
    set_sac_sic("x/x");

    adam::buffer* buf = create_parsed_frame({{103, 200}, {104, 63}, {105, 1}});
    ASSERT_NE(buf, nullptr);

    bool result = filter->handle_data(buf);
    EXPECT_TRUE(result);

    auto* root_frame = buf->begin_as<frame>();
    EXPECT_FALSE(root_frame->is_modified());

    for (const auto& blk : *root_frame)
    {
        EXPECT_FALSE(blk.is_removed());
    }

    adam::buffer_manager::get().return_buffer(buf);
}

TEST_F(filter_sac_sic_test, whitelist_multiple_patterns)
{
    set_mode("whitelist");
    set_sac_sic("103/x; x/63; 105/200");

    adam::buffer* buf = create_parsed_frame({{103, 1}, {104, 63}, {105, 200}, {105, 201}, {106, 1}});
    ASSERT_NE(buf, nullptr);

    bool result = filter->handle_data(buf);
    EXPECT_TRUE(result);

    auto* root_frame = buf->begin_as<frame>();
    auto it = root_frame->begin();
    EXPECT_FALSE(it->is_removed()); // 103/1 matches 103/x
    ++it;
    EXPECT_FALSE(it->is_removed()); // 104/63 matches x/63
    ++it;
    EXPECT_FALSE(it->is_removed()); // 105/200 matches 105/200
    ++it;
    EXPECT_TRUE(it->is_removed());  // 105/201 doesn't match
    ++it;
    EXPECT_TRUE(it->is_removed());  // 106/1 doesn't match

    adam::buffer_manager::get().return_buffer(buf);
}

TEST_F(filter_sac_sic_test, whitelist_no_match_discards_all)
{
    set_mode("whitelist");
    set_sac_sic("99/99");

    adam::buffer* buf = create_parsed_frame({{103, 200}, {104, 63}});
    ASSERT_NE(buf, nullptr);

    bool result = filter->handle_data(buf);
    EXPECT_FALSE(result);

    adam::buffer_manager::get().return_buffer(buf);
}

TEST_F(filter_sac_sic_test, whitelist_empty_patterns_discards_all)
{
    set_mode("whitelist");
    set_sac_sic("");

    adam::buffer* buf = create_parsed_frame({{103, 200}});
    ASSERT_NE(buf, nullptr);

    bool result = filter->handle_data(buf);
    EXPECT_FALSE(result);

    adam::buffer_manager::get().return_buffer(buf);
}

// ─── Blacklist Filtering Tests ────────────────────────────────────────────────

TEST_F(filter_sac_sic_test, blacklist_no_match_keeps_all)
{
    set_mode("blacklist");
    set_sac_sic("99/99; 100/100");

    adam::buffer* buf = create_parsed_frame({{103, 200}, {104, 63}});
    ASSERT_NE(buf, nullptr);

    bool result = filter->handle_data(buf);
    EXPECT_TRUE(result);

    auto* root_frame = buf->begin_as<frame>();
    EXPECT_FALSE(root_frame->is_modified());

    for (const auto& blk : *root_frame)
    {
        EXPECT_FALSE(blk.is_removed());
    }

    adam::buffer_manager::get().return_buffer(buf);
}

TEST_F(filter_sac_sic_test, blacklist_partial_match_removes_blacklisted)
{
    set_mode("blacklist");
    set_sac_sic("103/x; x/63");

    adam::buffer* buf = create_parsed_frame({{103, 200}, {104, 63}, {105, 1}});
    ASSERT_NE(buf, nullptr);

    bool result = filter->handle_data(buf);
    EXPECT_TRUE(result);

    auto* root_frame = buf->begin_as<frame>();
    EXPECT_TRUE(root_frame->is_modified());

    auto it = root_frame->begin();
    EXPECT_TRUE(it->is_removed());  // 103/200 dropped
    ++it;
    EXPECT_TRUE(it->is_removed());  // 104/63 dropped
    ++it;
    EXPECT_FALSE(it->is_removed()); // 105/1 kept

    adam::buffer_manager::get().return_buffer(buf);
}

TEST_F(filter_sac_sic_test, blacklist_all_match_discards_all)
{
    set_mode("blacklist");
    set_sac_sic("103/x");

    adam::buffer* buf = create_parsed_frame({{103, 1}, {103, 2}});
    ASSERT_NE(buf, nullptr);

    bool result = filter->handle_data(buf);
    EXPECT_FALSE(result);

    adam::buffer_manager::get().return_buffer(buf);
}

TEST_F(filter_sac_sic_test, blacklist_empty_patterns_keeps_all)
{
    set_mode("blacklist");
    set_sac_sic("");

    adam::buffer* buf = create_parsed_frame({{103, 200}, {104, 63}});
    ASSERT_NE(buf, nullptr);

    bool result = filter->handle_data(buf);
    EXPECT_TRUE(result);

    auto* root_frame = buf->begin_as<frame>();
    EXPECT_FALSE(root_frame->is_modified());

    adam::buffer_manager::get().return_buffer(buf);
}

// ─── Delimiter, Case-Insensitivity & Hex Parsing Tests ─────────────────────────

TEST_F(filter_sac_sic_test, parses_various_delimiters_and_formats)
{
    set_mode("whitelist");
    set_sac_sic(" 103 / X , x / 0x3F ; 0x67 / 0xC8 ; * / 99 ");

    adam::buffer* buf = create_parsed_frame({{103, 5}, {104, 63}, {103, 200}, {105, 99}, {106, 1}});
    ASSERT_NE(buf, nullptr);

    bool result = filter->handle_data(buf);
    EXPECT_TRUE(result);

    auto* root_frame = buf->begin_as<frame>();
    auto it = root_frame->begin();
    EXPECT_FALSE(it->is_removed()); // 103/5 matches 103/X
    ++it;
    EXPECT_FALSE(it->is_removed()); // 104/63 matches x/0x3F
    ++it;
    EXPECT_FALSE(it->is_removed()); // 103/200 matches 0x67/0xC8 (103/200)
    ++it;
    EXPECT_FALSE(it->is_removed()); // 105/99 matches */99
    ++it;
    EXPECT_TRUE(it->is_removed());  // 106/1 does not match

    adam::buffer_manager::get().return_buffer(buf);
}

TEST_F(filter_sac_sic_test, parses_leading_zeros_as_decimal)
{
    set_mode("whitelist");
    set_sac_sic("025/013; 007/x; x/001");

    adam::buffer* buf = create_parsed_frame({{25, 13}, {7, 99}, {100, 1}, {25, 14}, {8, 99}});
    ASSERT_NE(buf, nullptr);

    bool result = filter->handle_data(buf);
    EXPECT_TRUE(result);

    auto* root_frame = buf->begin_as<frame>();
    auto it = root_frame->begin();
    EXPECT_FALSE(it->is_removed()); // 25/13 matches 025/013
    ++it;
    EXPECT_FALSE(it->is_removed()); // 7/99 matches 007/x
    ++it;
    EXPECT_FALSE(it->is_removed()); // 100/1 matches x/001
    ++it;
    EXPECT_TRUE(it->is_removed());  // 25/14 does not match
    ++it;
    EXPECT_TRUE(it->is_removed());  // 8/99 does not match

    adam::buffer_manager::get().return_buffer(buf);
}

// ─── State Buffer Statistics Tests ───────────────────────────────────────────

TEST_F(filter_sac_sic_test, state_buffer_statistics_accumulation)
{
    set_mode("whitelist");
    set_sac_sic("103/x");

    adam::buffer* buf1 = create_parsed_frame({{103, 1}, {104, 1}});
    adam::buffer* buf2 = create_parsed_frame({{104, 1}});
    ASSERT_NE(buf1, nullptr);
    ASSERT_NE(buf2, nullptr);

    filter->handle_data(buf1); // forwarded
    filter->handle_data(buf2); // discarded

    const auto* stats = filter->get_state_buffer_data();
    ASSERT_NE(stats, nullptr);

    EXPECT_EQ(stats->total_buffers_recieved, 2u);
    EXPECT_EQ(stats->total_buffers_forwarded, 1u);
    EXPECT_EQ(stats->total_buffers_discarded, 1u);

    adam::buffer_manager::get().return_buffer(buf1);
    adam::buffer_manager::get().return_buffer(buf2);
}

// ─── End-to-End Pipeline Test ────────────────────────────────────────────────

TEST_F(filter_sac_sic_test, end_to_end_parse_filter_encode)
{
    set_mode("whitelist");
    set_sac_sic("103/x");

    // 103/1 (keep) and 104/1 (drop)
    adam::buffer* internal_data = create_parsed_frame({{103, 1}, {104, 1}});
    ASSERT_NE(internal_data, nullptr);

    bool filter_ok = filter->handle_data(internal_data);
    EXPECT_TRUE(filter_ok);

    adam::buffer* encoded_buf = nullptr;
    bool encode_ok = encoder.encode(encoded_buf, internal_data);
    EXPECT_TRUE(encode_ok);
    ASSERT_NE(encoded_buf, nullptr);

    EXPECT_EQ(encoded_buf->get_size(), 6u); // 1 block preserved

    adam::buffer* reparsed_data = nullptr;
    bool reparse_ok = parser.parse(encoded_buf, reparsed_data);
    EXPECT_TRUE(reparse_ok);
    ASSERT_NE(reparsed_data, nullptr);

    auto* reparsed_frame = reparsed_data->begin_as<frame>();
    EXPECT_EQ(reparsed_frame->block_count, 1u);

    auto it = reparsed_frame->begin();
    EXPECT_EQ(it->category, 34);
    EXPECT_FALSE(it->is_removed());

    adam::buffer_manager::get().return_buffer(internal_data);
    adam::buffer_manager::get().return_buffer(encoded_buf);
    adam::buffer_manager::get().return_buffer(reparsed_data);
}
