/**
 * @file    test-asterix-sac-sic-replacer.cpp
 * @author  dexus1337
 * @brief   Unit and integration tests for asterix sac_sic_replacer with pattern matching.
 */

#include <gtest/gtest.h>
#include "data/filters/asterix-sac-sic-replacer.hpp"
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

class filter_sac_sic_replacer_test : public ::testing::Test
{
protected:
    void SetUp() override
    {
        adam::buffer_manager::get().initialize();
        parser = asterix_parser();
        encoder = asterix_encoder();
        replacer = std::make_unique<sac_sic_replacer>("test_replacer"_ct);
    }

    void TearDown() override
    {
        replacer.reset();
        adam::buffer_manager::get().destroy();
    }

    void set_source(const char* patterns)
    {
        auto* user_params = replacer->get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
        auto* source_param = user_params->get<configuration_parameter_string>("source_sac_sic"_ct);
        source_param->set_value(string_hashed(patterns));
    }

    void set_target(const char* pattern)
    {
        auto* user_params = replacer->get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
        auto* target_param = user_params->get<configuration_parameter_string>("target_sac_sic"_ct);
        target_param->set_value(string_hashed(pattern));
    }

    struct test_sac_sic
    {
        uint8_t sac;
        uint8_t sic;
    };

    adam::buffer* create_parsed_frame(const std::vector<test_sac_sic>& sac_sic_pairs, uint8_t cat = 34)
    {
        std::vector<uint8_t> raw;
        for (const auto& pair : sac_sic_pairs)
        {
            // Block Header: CAT (1B), Length (2B, big-endian) = 6 bytes
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
    std::unique_ptr<sac_sic_replacer> replacer;
};

// ─── Initialization & Parameters ─────────────────────────────────────────────

TEST_F(filter_sac_sic_replacer_test, initial_properties)
{
    EXPECT_EQ(replacer->get_type_name(), "asterix-sac-sic-replacer"_ct);
    EXPECT_EQ(replacer->get_input_format()->get_name(), "asterix"_ct);
    EXPECT_EQ(replacer->get_output_format()->get_name(), "asterix"_ct);

    auto* user_params = replacer->get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    ASSERT_NE(user_params, nullptr);

    auto* source_param = user_params->get<configuration_parameter_string>("source_sac_sic"_ct);
    auto* target_param = user_params->get<configuration_parameter_string>("target_sac_sic"_ct);

    ASSERT_NE(source_param, nullptr);
    ASSERT_NE(target_param, nullptr);

    EXPECT_EQ(source_param->get_value(), ""_ct);
    EXPECT_EQ(target_param->get_value(), "x/x"_ct);
}

TEST_F(filter_sac_sic_replacer_test, handle_null_buffer)
{
    adam::buffer* null_buf = nullptr;
    EXPECT_FALSE(replacer->handle_data(null_buf));
}

// ─── Replacement Operations ──────────────────────────────────────────────────

TEST_F(filter_sac_sic_replacer_test, exact_replacement)
{
    set_source("103/200");
    set_target("105/201");

    adam::buffer* buf = create_parsed_frame({{103, 200}, {103, 63}, {104, 200}});
    ASSERT_NE(buf, nullptr);

    bool result = replacer->handle_data(buf);
    EXPECT_TRUE(result);

    auto* root_frame = buf->begin_as<frame>();
    EXPECT_TRUE(root_frame->is_modified());

    auto* ref_buf = buf->get_referenced_buffer();
    const auto* uap = uap_pool::get().get_uap(34);

    auto it = root_frame->begin();
    const auto* raw1 = uap->get_record_sac_sic(&(*it->begin()), ref_buf);
    EXPECT_EQ(raw1->sac, 105);
    EXPECT_EQ(raw1->sic, 201);

    ++it;
    const auto* raw2 = uap->get_record_sac_sic(&(*it->begin()), ref_buf);
    EXPECT_EQ(raw2->sac, 103);
    EXPECT_EQ(raw2->sic, 63);

    ++it;
    const auto* raw3 = uap->get_record_sac_sic(&(*it->begin()), ref_buf);
    EXPECT_EQ(raw3->sac, 104);
    EXPECT_EQ(raw3->sic, 200);

    adam::buffer_manager::get().return_buffer(buf);
}

TEST_F(filter_sac_sic_replacer_test, replace_sac_only_keep_sic)
{
    set_source("103/x");
    set_target("200/x"); // Replace SAC with 200, keep original SIC

    adam::buffer* buf = create_parsed_frame({{103, 50}, {103, 60}, {104, 50}});
    ASSERT_NE(buf, nullptr);

    bool result = replacer->handle_data(buf);
    EXPECT_TRUE(result);

    auto* root_frame = buf->begin_as<frame>();
    EXPECT_TRUE(root_frame->is_modified());

    auto* ref_buf = buf->get_referenced_buffer();
    const auto* uap = uap_pool::get().get_uap(34);

    auto it = root_frame->begin();
    const auto* raw1 = uap->get_record_sac_sic(&(*it->begin()), ref_buf);
    EXPECT_EQ(raw1->sac, 200);
    EXPECT_EQ(raw1->sic, 50);

    ++it;
    const auto* raw2 = uap->get_record_sac_sic(&(*it->begin()), ref_buf);
    EXPECT_EQ(raw2->sac, 200);
    EXPECT_EQ(raw2->sic, 60);

    ++it;
    const auto* raw3 = uap->get_record_sac_sic(&(*it->begin()), ref_buf);
    EXPECT_EQ(raw3->sac, 104);
    EXPECT_EQ(raw3->sic, 50);

    adam::buffer_manager::get().return_buffer(buf);
}

TEST_F(filter_sac_sic_replacer_test, replace_sic_only_keep_sac)
{
    set_source("x/63");
    set_target("x/99"); // Keep original SAC, replace SIC with 99

    adam::buffer* buf = create_parsed_frame({{103, 63}, {104, 63}, {103, 64}});
    ASSERT_NE(buf, nullptr);

    bool result = replacer->handle_data(buf);
    EXPECT_TRUE(result);

    auto* root_frame = buf->begin_as<frame>();
    EXPECT_TRUE(root_frame->is_modified());

    auto* ref_buf = buf->get_referenced_buffer();
    const auto* uap = uap_pool::get().get_uap(34);

    auto it = root_frame->begin();
    const auto* raw1 = uap->get_record_sac_sic(&(*it->begin()), ref_buf);
    EXPECT_EQ(raw1->sac, 103);
    EXPECT_EQ(raw1->sic, 99);

    ++it;
    const auto* raw2 = uap->get_record_sac_sic(&(*it->begin()), ref_buf);
    EXPECT_EQ(raw2->sac, 104);
    EXPECT_EQ(raw2->sic, 99);

    ++it;
    const auto* raw3 = uap->get_record_sac_sic(&(*it->begin()), ref_buf);
    EXPECT_EQ(raw3->sac, 103);
    EXPECT_EQ(raw3->sic, 64);

    adam::buffer_manager::get().return_buffer(buf);
}

TEST_F(filter_sac_sic_replacer_test, no_match_unmodified)
{
    set_source("99/99");
    set_target("100/100");

    adam::buffer* buf = create_parsed_frame({{103, 63}, {104, 64}});
    ASSERT_NE(buf, nullptr);

    bool result = replacer->handle_data(buf);
    EXPECT_TRUE(result);

    auto* root_frame = buf->begin_as<frame>();
    EXPECT_FALSE(root_frame->is_modified());

    adam::buffer_manager::get().return_buffer(buf);
}

TEST_F(filter_sac_sic_replacer_test, leading_zeros_decimal)
{
    set_source("025/013");
    set_target("007/008");

    adam::buffer* buf = create_parsed_frame({{25, 13}, {25, 14}});
    ASSERT_NE(buf, nullptr);

    bool result = replacer->handle_data(buf);
    EXPECT_TRUE(result);

    auto* root_frame = buf->begin_as<frame>();
    EXPECT_TRUE(root_frame->is_modified());

    auto* ref_buf = buf->get_referenced_buffer();
    const auto* uap = uap_pool::get().get_uap(34);

    auto it = root_frame->begin();
    const auto* raw1 = uap->get_record_sac_sic(&(*it->begin()), ref_buf);
    EXPECT_EQ(raw1->sac, 7);
    EXPECT_EQ(raw1->sic, 8);

    ++it;
    const auto* raw2 = uap->get_record_sac_sic(&(*it->begin()), ref_buf);
    EXPECT_EQ(raw2->sac, 25);
    EXPECT_EQ(raw2->sic, 14);

    adam::buffer_manager::get().return_buffer(buf);
}

// ─── End-to-End Pipeline Test ────────────────────────────────────────────────

TEST_F(filter_sac_sic_replacer_test, end_to_end_parse_replace_encode)
{
    set_source("103/x");
    set_target("200/x");

    adam::buffer* internal_data = create_parsed_frame({{103, 10}});
    ASSERT_NE(internal_data, nullptr);

    bool replace_ok = replacer->handle_data(internal_data);
    EXPECT_TRUE(replace_ok);

    adam::buffer* encoded_buf = nullptr;
    bool encode_ok = encoder.encode(encoded_buf, internal_data);
    EXPECT_TRUE(encode_ok);
    ASSERT_NE(encoded_buf, nullptr);

    EXPECT_EQ(encoded_buf->get_size(), 6u);

    adam::buffer* reparsed_data = nullptr;
    bool reparse_ok = parser.parse(encoded_buf, reparsed_data);
    EXPECT_TRUE(reparse_ok);
    ASSERT_NE(reparsed_data, nullptr);

    auto* reparsed_frame = reparsed_data->begin_as<frame>();
    EXPECT_EQ(reparsed_frame->block_count, 1u);

    auto* ref_buf = reparsed_data->get_referenced_buffer();
    const auto* uap = uap_pool::get().get_uap(34);
    auto it = reparsed_frame->begin();
    const auto* raw = uap->get_record_sac_sic(&(*it->begin()), ref_buf);
    EXPECT_EQ(raw->sac, 200);
    EXPECT_EQ(raw->sic, 10);

    adam::buffer_manager::get().return_buffer(internal_data);
    adam::buffer_manager::get().return_buffer(encoded_buf);
    adam::buffer_manager::get().return_buffer(reparsed_data);
}
