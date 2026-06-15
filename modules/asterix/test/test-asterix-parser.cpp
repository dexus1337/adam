#include <gtest/gtest.h>
#include "data/asterix-parser.hpp"
#include "data/asterix-uap.hpp"
#include "data/uap/cat048.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "memory/buffer/buffer.hpp"
#include "data/asterix-types.hpp"
#include <string_view>

using namespace adam::modules::asterix::data;

class asterix_parser_test : public ::testing::Test 
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

TEST_F(asterix_parser_test, test_dummy_parse)
{
    asterix_parser parser;

    // Create a dummy raw buffer representing an Asterix packet
    adam::buffer* raw_buf = adam::buffer_manager::get().request_buffer(64);
    ASSERT_NE(raw_buf, nullptr);

    uint8_t* raw_data = raw_buf->begin_as<uint8_t>();
    
    // Build a valid CAT048 packet
    raw_data[0] = 48; // CAT
    raw_data[1] = 0;  // Length MSB
    raw_data[2] = 9;  // Length LSB (3 bytes header + 1 byte FSPEC + 2 bytes DSID + 3 bytes TOD = 9 bytes)
    
    // FSPEC: I048/010 (FRN 1), I048/140 (FRN 2)
    // Bit 7 is FRN 1, bit 6 is FRN 2. So 11000000 = 0xC0. FX=0.
    raw_data[3] = 0xC0; // FSPEC: FRN 1 and FRN 2 only
    
    // FRN 1: Data Source Identifier (2 bytes fixed)
    raw_data[4] = 0xAA;
    raw_data[5] = 0xBB;
    
    // FRN 2: Time of Day (3 bytes fixed)
    raw_data[6] = 0x01;
    raw_data[7] = 0x02;
    raw_data[8] = 0x03;
    
    raw_buf->set_size(9);

    adam::buffer* internal_data = nullptr;
    bool success = parser.parse(raw_buf, internal_data);

    EXPECT_TRUE(success);
    ASSERT_NE(internal_data, nullptr);
    EXPECT_GT(internal_data->get_size(), 0);

    // Verify layout (should be two FIXED items)
    const auto* parsed_item_1 = internal_data->get_begin_as<asterix_item_header>();
    EXPECT_EQ(parsed_item_1->type, asterix_item_type::FIXED);
    EXPECT_EQ(parsed_item_1->data_length, 2);
    
    const auto* parsed_item_2 = reinterpret_cast<const asterix_item_header*>(
        reinterpret_cast<const uint8_t*>(parsed_item_1) + sizeof(asterix_item_header)
    );
    EXPECT_EQ(parsed_item_2->type, asterix_item_type::FIXED);
    EXPECT_EQ(parsed_item_2->data_length, 3);

    adam::buffer_manager::get().return_buffer(internal_data);
    adam::buffer_manager::get().return_buffer(raw_buf);
}

// Dummy classification data for testing custom expansions
class dummy_classification_data : public asterix_uap_expansion
{
public:
    int classification_level = 5;
};

TEST_F(asterix_parser_test, test_custom_uap_expansion_and_o1_lookup)
{
    auto& cat048 = uap::get_cat048_uap();

    // Verify O(1) lookup works and names are intact
    const asterix_field_spec* spec_tod = cat048.get_spec(2); // FRN 2 is Time of Day
    ASSERT_NE(spec_tod, nullptr);
    EXPECT_EQ(spec_tod->type, asterix_item_type::FIXED);
    EXPECT_EQ(std::string_view(spec_tod->name), "I048/140 Time of Day");

    // Register a custom metadata for FRN 29 (SP)
    dummy_classification_data my_metadata;
    my_metadata.classification_level = 42;
    cat048.expand_parameter(29, &my_metadata);

    // Retrieve the expansion metadata in O(1) time
    asterix_uap_expansion* expansion = cat048.get_expansion(29);
    ASSERT_EQ(expansion, &my_metadata);

    // Cast it back to verify custom attributes
    dummy_classification_data* retrieved_data = static_cast<dummy_classification_data*>(expansion);
    EXPECT_EQ(retrieved_data->classification_level, 42);
}
