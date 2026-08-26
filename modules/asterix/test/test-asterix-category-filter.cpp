/**
 * @file    test-asterix-category-filter.cpp
 * @author  dexus1337
 * @brief   Unit and integration tests for asterix category_filter.
 */

#include <gtest/gtest.h>
#include "data/filters/asterix-category-filter.hpp"
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

class category_filter_test : public ::testing::Test
{
protected:
    void SetUp() override
    {
        adam::buffer_manager::get().initialize();
        parser = asterix_parser();
        encoder = asterix_encoder();
        filter = std::make_unique<category_filter>("test_cat_filter"_ct);
    }

    void TearDown() override
    {
        filter.reset();
        adam::buffer_manager::get().destroy();
    }

    void set_mode(const char* mode)
    {
        auto* user_params = filter->get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
        auto* mode_param = user_params->get<configuration_parameter_string>("mode"_ct);
        mode_param->set_value(string_hashed(mode));
    }

    void set_cats(const char* cats)
    {
        auto* user_params = filter->get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
        auto* cats_param = user_params->get<configuration_parameter_string>("cats"_ct);
        cats_param->set_value(string_hashed(cats));
    }

    /**
     * @brief Helper to create a parsed internal Asterix buffer with given categories.
     * Each block has minimal valid structure (CAT, Len, 1 record with FSPEC and DSID item).
     */
    adam::buffer* create_parsed_frame(const std::vector<uint8_t>& categories)
    {
        // Build raw message with multiple blocks
        std::vector<uint8_t> raw;
        for (uint8_t cat : categories)
        {
            // Block Header: CAT (1B), Length (2B, big-endian) = 6 bytes total
            raw.push_back(cat);
            raw.push_back(0);
            raw.push_back(6); // 3B header + 1B FSPEC + 2B DSID (FRN 1)

            // Record: FSPEC with FRN 1 active
            raw.push_back(0x80);

            // DSID payload (2B)
            raw.push_back(cat);
            raw.push_back(0x01);
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
    std::unique_ptr<category_filter> filter;
};

// ─── Initialization & Parameter Tests ────────────────────────────────────────

TEST_F(category_filter_test, initial_properties)
{
    EXPECT_EQ(filter->get_type_name(), "asterix-category-filter"_ct);
    EXPECT_EQ(filter->get_input_format()->get_name(), "asterix"_ct);
    EXPECT_EQ(filter->get_output_format()->get_name(), "asterix"_ct);

    auto* user_params = filter->get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    ASSERT_NE(user_params, nullptr);

    auto* mode_param = user_params->get<configuration_parameter_string>("mode"_ct);
    auto* cats_param = user_params->get<configuration_parameter_string>("cats"_ct);

    ASSERT_NE(mode_param, nullptr);
    ASSERT_NE(cats_param, nullptr);

    EXPECT_EQ(mode_param->get_value(), "whitelist"_ct);
    EXPECT_EQ(cats_param->get_value(), ""_ct);
}

// ─── Null and Empty Input ───────────────────────────────────────────────────

TEST_F(category_filter_test, handle_null_buffer)
{
    adam::buffer* null_buf = nullptr;
    EXPECT_FALSE(filter->handle_data(null_buf));
}

// ─── Whitelist Mode: Full Match (Keep All) ──────────────────────────────────

TEST_F(category_filter_test, whitelist_all_match_keeps_frame_unmodified)
{
    set_mode("whitelist");
    set_cats("48; 62");

    adam::buffer* internal = create_parsed_frame({48, 62});
    ASSERT_NE(internal, nullptr);

    auto* frm = internal->begin_as<frame>();
    ASSERT_NE(frm, nullptr);
    EXPECT_FALSE(frm->is_modified());

    bool result = filter->handle_data(internal);
    EXPECT_TRUE(result);

    // Frame should remain unmodified (fast zero-copy path preserved)
    EXPECT_FALSE(frm->is_modified());

    // Both blocks must not be removed
    for (const auto& blk : *frm)
    {
        EXPECT_FALSE(blk.is_removed());
    }

    adam::buffer_manager::get().return_buffer(internal);
}

// ─── Whitelist Mode: Partial Match (Filter Out Some) ────────────────────────

TEST_F(category_filter_test, whitelist_partial_match_removes_non_matching)
{
    set_mode("whitelist");
    set_cats("48, 1"); // Keep CAT 48 and CAT 1, discard CAT 62

    adam::buffer* internal = create_parsed_frame({48, 62, 1});
    ASSERT_NE(internal, nullptr);

    auto* frm = internal->begin_as<frame>();
    ASSERT_NE(frm, nullptr);

    bool result = filter->handle_data(internal);
    EXPECT_TRUE(result);

    // Frame must be marked modified
    EXPECT_TRUE(frm->is_modified());

    // Verify block removal states
    auto it = frm->begin();
    ASSERT_NE(it, frm->end());
    EXPECT_EQ(it->category, 48);
    EXPECT_FALSE(it->is_removed());

    ++it;
    ASSERT_NE(it, frm->end());
    EXPECT_EQ(it->category, 62);
    EXPECT_TRUE(it->is_removed());

    ++it;
    ASSERT_NE(it, frm->end());
    EXPECT_EQ(it->category, 1);
    EXPECT_FALSE(it->is_removed());

    ++it;
    EXPECT_EQ(it, frm->end());

    adam::buffer_manager::get().return_buffer(internal);
}

// ─── Whitelist Mode: No Match (Discard Frame) ───────────────────────────────

TEST_F(category_filter_test, whitelist_no_match_discards_all)
{
    set_mode("whitelist");
    set_cats("21; 34");

    adam::buffer* internal = create_parsed_frame({48, 62});
    ASSERT_NE(internal, nullptr);

    bool result = filter->handle_data(internal);
    EXPECT_FALSE(result);

    auto* stats = filter->get_state_buffer_data();
    if (stats)
    {
        EXPECT_EQ(stats->total_buffers_discarded.load(), 1u);
        EXPECT_EQ(stats->total_buffers_forwarded.load(), 0u);
    }

    adam::buffer_manager::get().return_buffer(internal);
}

// ─── Whitelist Mode: Empty Category List ────────────────────────────────────

TEST_F(category_filter_test, whitelist_empty_cats_discards_all)
{
    set_mode("whitelist");
    set_cats("");

    adam::buffer* internal = create_parsed_frame({48});
    ASSERT_NE(internal, nullptr);

    bool result = filter->handle_data(internal);
    EXPECT_FALSE(result);

    adam::buffer_manager::get().return_buffer(internal);
}

// ─── Blacklist Mode: No Match on Blacklist (Keep All) ───────────────────────

TEST_F(category_filter_test, blacklist_no_match_keeps_all)
{
    set_mode("blacklist");
    set_cats("1; 21");

    adam::buffer* internal = create_parsed_frame({48, 62});
    ASSERT_NE(internal, nullptr);

    auto* frm = internal->begin_as<frame>();
    ASSERT_NE(frm, nullptr);

    bool result = filter->handle_data(internal);
    EXPECT_TRUE(result);
    EXPECT_FALSE(frm->is_modified());

    for (const auto& blk : *frm)
    {
        EXPECT_FALSE(blk.is_removed());
    }

    adam::buffer_manager::get().return_buffer(internal);
}

// ─── Blacklist Mode: Partial Match (Filter Out Blacklisted) ─────────────────

TEST_F(category_filter_test, blacklist_partial_match_removes_blacklisted)
{
    set_mode("blacklist");
    set_cats("62");

    adam::buffer* internal = create_parsed_frame({48, 62, 1});
    ASSERT_NE(internal, nullptr);

    auto* frm = internal->begin_as<frame>();
    ASSERT_NE(frm, nullptr);

    bool result = filter->handle_data(internal);
    EXPECT_TRUE(result);
    EXPECT_TRUE(frm->is_modified());

    auto it = frm->begin();
    EXPECT_EQ(it->category, 48);
    EXPECT_FALSE(it->is_removed());

    ++it;
    EXPECT_EQ(it->category, 62);
    EXPECT_TRUE(it->is_removed());

    ++it;
    EXPECT_EQ(it->category, 1);
    EXPECT_FALSE(it->is_removed());

    adam::buffer_manager::get().return_buffer(internal);
}

// ─── Blacklist Mode: Full Match on Blacklist (Discard All) ──────────────────

TEST_F(category_filter_test, blacklist_all_match_discards_all)
{
    set_mode("blacklist");
    set_cats("48; 62");

    adam::buffer* internal = create_parsed_frame({48, 62});
    ASSERT_NE(internal, nullptr);

    bool result = filter->handle_data(internal);
    EXPECT_FALSE(result);

    adam::buffer_manager::get().return_buffer(internal);
}

// ─── Blacklist Mode: Empty Category List (Keep All) ─────────────────────────

TEST_F(category_filter_test, blacklist_empty_cats_keeps_all)
{
    set_mode("blacklist");
    set_cats("");

    adam::buffer* internal = create_parsed_frame({48, 62});
    ASSERT_NE(internal, nullptr);

    auto* frm = internal->begin_as<frame>();
    ASSERT_NE(frm, nullptr);

    bool result = filter->handle_data(internal);
    EXPECT_TRUE(result);
    EXPECT_FALSE(frm->is_modified());

    adam::buffer_manager::get().return_buffer(internal);
}

// ─── Delimiter and Format Parsing Tests ─────────────────────────────────────

TEST_F(category_filter_test, parses_various_delimiters_and_formats)
{
    // Test comma, semicolon, whitespace, and hex parsing
    set_mode("whitelist");
    set_cats(" 48 , 0x3E ; 0x01 ; "); // 48, 62, 1

    adam::buffer* internal = create_parsed_frame({48, 62, 1, 21});
    ASSERT_NE(internal, nullptr);

    auto* frm = internal->begin_as<frame>();
    ASSERT_NE(frm, nullptr);

    bool result = filter->handle_data(internal);
    EXPECT_TRUE(result);
    EXPECT_TRUE(frm->is_modified());

    auto it = frm->begin();
    EXPECT_EQ(it->category, 48);
    EXPECT_FALSE(it->is_removed());

    ++it;
    EXPECT_EQ(it->category, 62);
    EXPECT_FALSE(it->is_removed());

    ++it;
    EXPECT_EQ(it->category, 1);
    EXPECT_FALSE(it->is_removed());

    ++it;
    EXPECT_EQ(it->category, 21);
    EXPECT_TRUE(it->is_removed());

    adam::buffer_manager::get().return_buffer(internal);
}

// ─── Statistics Tracking Across Multiple Invocations ────────────────────────

TEST_F(category_filter_test, state_buffer_statistics_accumulation)
{
    set_mode("whitelist");
    set_cats("48");

    // Message 1: Has CAT 48 and CAT 62 -> forwarded
    adam::buffer* buf1 = create_parsed_frame({48, 62});
    ASSERT_NE(buf1, nullptr);
    uint32_t size1 = buf1->get_size();
    EXPECT_TRUE(filter->handle_data(buf1));

    // Message 2: Only has CAT 62 -> discarded
    adam::buffer* buf2 = create_parsed_frame({62});
    ASSERT_NE(buf2, nullptr);
    uint32_t size2 = buf2->get_size();
    EXPECT_FALSE(filter->handle_data(buf2));

    // Message 3: Only has CAT 48 -> forwarded
    adam::buffer* buf3 = create_parsed_frame({48});
    ASSERT_NE(buf3, nullptr);
    uint32_t size3 = buf3->get_size();
    EXPECT_TRUE(filter->handle_data(buf3));

    auto* stats = filter->get_state_buffer_data();
    ASSERT_NE(stats, nullptr);

    EXPECT_EQ(stats->total_buffers_recieved.load(), 3u);
    EXPECT_EQ(stats->total_bytes_recieved.load(), size1 + size2 + size3);

    EXPECT_EQ(stats->total_buffers_forwarded.load(), 2u);
    EXPECT_EQ(stats->total_bytes_forwarded.load(), size1 + size3);

    EXPECT_EQ(stats->total_buffers_discarded.load(), 1u);
    EXPECT_EQ(stats->total_bytes_discarded.load(), size2);

    adam::buffer_manager::get().return_buffer(buf1);
    adam::buffer_manager::get().return_buffer(buf2);
    adam::buffer_manager::get().return_buffer(buf3);
}

// ─── End-to-End Pipeline: Parse -> Filter -> Encode -> Re-parse ─────────────

TEST_F(category_filter_test, end_to_end_parse_filter_encode)
{
    set_mode("whitelist");
    set_cats("62"); // Keep only CAT 62, drop CAT 48 and CAT 1

    adam::buffer* internal = create_parsed_frame({48, 62, 1});
    ASSERT_NE(internal, nullptr);

    // Filter
    EXPECT_TRUE(filter->handle_data(internal));

    // Encode
    adam::buffer* encoded = nullptr;
    ASSERT_TRUE(encoder.encode(encoded, internal));
    ASSERT_NE(encoded, nullptr);

    // Re-parse the encoded output buffer
    adam::buffer* reparsed = nullptr;
    ASSERT_TRUE(parser.parse(encoded, reparsed));
    ASSERT_NE(reparsed, nullptr);

    auto* reparsed_frame = reparsed->begin_as<frame>();
    ASSERT_NE(reparsed_frame, nullptr);

    // Must have exactly 1 block of CAT 62
    EXPECT_EQ(reparsed_frame->block_count, 1u);

    auto it = reparsed_frame->begin();
    ASSERT_NE(it, reparsed_frame->end());
    EXPECT_EQ(it->category, 62);
    EXPECT_FALSE(it->is_removed());
    EXPECT_EQ(it->record_count, 1u);

    ++it;
    EXPECT_EQ(it, reparsed_frame->end());

    adam::buffer_manager::get().return_buffer(reparsed);
    adam::buffer_manager::get().return_buffer(encoded);
    adam::buffer_manager::get().return_buffer(internal);
}
