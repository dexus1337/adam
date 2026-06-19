#include <gtest/gtest.h>
#include "data/asterix-parser.hpp"
#include "data/asterix-encoder.hpp"
#include "data/asterix-uap.hpp"
#include "data/asterix-internal.hpp"
#include "data/categories/048/cat048-uap.hpp"
#include "test-messages.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "memory/buffer/buffer.hpp"
#include "data/asterix-types.hpp"
#include "data/categories/001/cat001.hpp"

#include <vector>
#include <cstring>
#include <iostream>
#include <chrono>

using namespace adam::modules;

class encoder_test : public ::testing::Test 
{
protected:
    void SetUp() override 
    {
        adam::buffer_manager::get().initialize();
        parser = asterix::asterix_parser();
        encoder = asterix::asterix_encoder();
    }

    void TearDown() override 
    {
        adam::buffer_manager::get().destroy();
    }
    
    asterix::asterix_parser parser;
    asterix::asterix_encoder encoder;
};

TEST_F(encoder_test, unmodified_frame_returns_original_buffer)
{
    std::vector<uint8_t> raw = build_cat048_message();

    adam::buffer* raw_buf = adam::buffer_manager::get().request_buffer(static_cast<uint32_t>(raw.size()));
    std::memcpy(raw_buf->begin_as<uint8_t>(), raw.data(), raw.size());
    raw_buf->set_size(static_cast<uint32_t>(raw.size()));

    adam::buffer* internal = nullptr;
    ASSERT_TRUE(parser.parse(raw_buf, internal));

    adam::buffer* encoded = nullptr;
    ASSERT_TRUE(encoder.encode(encoded, internal));
    ASSERT_NE(encoded, nullptr);

    // Optimization: since the frame was not modified, it must reuse the original buffer directly
    EXPECT_EQ(encoded, raw_buf);
    EXPECT_EQ(encoded->get_ref_count(), 2u);

    adam::buffer_manager::get().return_buffer(encoded);
    adam::buffer_manager::get().return_buffer(internal);
    adam::buffer_manager::get().return_buffer(raw_buf);
}

TEST_F(encoder_test, bulk_copy_unmodified_blocks_and_records)
{
    // Build a block with two records
    adam::buffer* raw_buf = adam::buffer_manager::get().request_buffer(128);
    ASSERT_NE(raw_buf, nullptr);

    uint8_t* raw_data = raw_buf->begin_as<uint8_t>();
    
    // Block 1 header: CAT 48, Length 10
    raw_data[0] = 48; 
    raw_data[1] = 0;  
    raw_data[2] = 10; 
    
    // Record 1: 1 item (DSID, 2 bytes)
    raw_data[3] = 0x80; // FSPEC: FRN 1
    raw_data[4] = 0x11;
    raw_data[5] = 0x22;
    
    // Record 2: 1 item (TOD, 3 bytes)
    raw_data[6] = 0x40; // FSPEC: FRN 2
    raw_data[7] = 0xAA;
    raw_data[8] = 0xBB;
    raw_data[9] = 0xCC;

    raw_buf->set_size(10);

    adam::buffer* internal = nullptr;
    ASSERT_TRUE(parser.parse(raw_buf, internal));

    // Force frame modification
    auto* frm = internal->begin_as<asterix::frame>();
    frm->set_modified(true);

    // Modify record 1, but leave record 2 unmodified.
    // Also, the block is modified because record 1 is modified.
    auto* blk = const_cast<asterix::block*>(frm->get_block(0));
    blk->set_modified(true);

    auto* rec1 = const_cast<asterix::record*>(blk->get_record(0));
    rec1->set_modified(true);

    auto* item = const_cast<asterix::item*>(rec1->get_item(1));
    item->set_modified(true);

    adam::buffer* encoded = nullptr;
    ASSERT_TRUE(encoder.encode(encoded, internal));
    ASSERT_NE(encoded, nullptr);
    EXPECT_NE(encoded, raw_buf);

    // Verify raw output data matches exactly
    EXPECT_EQ(encoded->get_size(), raw_buf->get_size());
    EXPECT_EQ(std::memcmp(encoded->get_begin(), raw_buf->get_begin(), raw_buf->get_size()), 0);

    adam::buffer_manager::get().return_buffer(encoded);
    adam::buffer_manager::get().return_buffer(internal);
    adam::buffer_manager::get().return_buffer(raw_buf);
}

TEST_F(encoder_test, encode_modified_compound_item)
{
    adam::buffer* raw_buf = adam::buffer_manager::get().request_buffer(64);
    ASSERT_NE(raw_buf, nullptr);

    uint8_t* raw_data = raw_buf->begin_as<uint8_t>();

    // Block Header: CAT 62, Length 15
    raw_data[0] = 62; 
    raw_data[1] = 0;  
    raw_data[2] = 15; 
    
    // FSPEC: FRN 24 active
    raw_data[3] = 0x01; 
    raw_data[4] = 0x01; 
    raw_data[5] = 0x01; 
    raw_data[6] = 0x20; 
    
    // FRN 24: Mode 5 Data (compound)
    // Compound FSPEC: FRN 1 and FRN 3 active
    raw_data[7] = 0xA0; 
    
    // Child FRN 1 (1 byte)
    raw_data[8] = 0x55;

    // Child FRN 3 (6 bytes)
    raw_data[9] = 0x11;
    raw_data[10] = 0x22;
    raw_data[11] = 0x33;
    raw_data[12] = 0x44;
    raw_data[13] = 0x55;
    raw_data[14] = 0x66;

    raw_buf->set_size(15);

    adam::buffer* internal = nullptr;
    ASSERT_TRUE(parser.parse(raw_buf, internal));

    // Modify
    auto* frm = internal->begin_as<asterix::frame>();
    frm->set_modified(true);

    auto* blk = const_cast<asterix::block*>(frm->get_block(0));
    blk->set_modified(true);

    auto* rec = const_cast<asterix::record*>(blk->get_record(0));
    rec->set_modified(true);

    auto* comp = const_cast<asterix::item*>(rec->get_item(24));
    comp->set_modified(true);

    adam::buffer* encoded = nullptr;
    ASSERT_TRUE(encoder.encode(encoded, internal));
    ASSERT_NE(encoded, nullptr);
    EXPECT_EQ(encoded->get_size(), 15u);
    EXPECT_EQ(std::memcmp(encoded->get_begin(), raw_buf->get_begin(), 15), 0);

    adam::buffer_manager::get().return_buffer(encoded);
    adam::buffer_manager::get().return_buffer(internal);
    adam::buffer_manager::get().return_buffer(raw_buf);
}

TEST_F(encoder_test, encode_explicit_uap_backed_item)
{
    adam::buffer* raw_buf = adam::buffer_manager::get().request_buffer(64);
    ASSERT_NE(raw_buf, nullptr);

    uint8_t* raw_data = raw_buf->begin_as<uint8_t>();

    // Block Header: CAT 48, Length 17
    raw_data[0] = 48;  
    raw_data[1] = 0;   
    raw_data[2] = 17;  

    // FSPEC: FRN 28
    raw_data[3] = 0x01; 
    raw_data[4] = 0x01; 
    raw_data[5] = 0x01; 
    raw_data[6] = 0x02; 

    // RE explicit item (backed by REF)
    raw_data[7]  = 0x0A; // length = 10 (includes itself)
    raw_data[8]  = 0x10; // REF FSPEC: RPC (FRN 4) present

    // RPC compound
    raw_data[9]  = 0xF0; // RPC FSPEC: all 4 children present
    raw_data[10] = 0x20; // Score
    raw_data[11] = 0x21; // Signal/Clutter Ratio
    raw_data[12] = 0x22; 
    raw_data[13] = 0x23; // Range Width
    raw_data[14] = 0x24; 
    raw_data[15] = 0x25; // Ambiguous Range
    raw_data[16] = 0x26; 

    raw_buf->set_size(17);

    adam::buffer* internal = nullptr;
    ASSERT_TRUE(parser.parse(raw_buf, internal));

    // Modify
    auto* frm = internal->begin_as<asterix::frame>();
    frm->set_modified(true);

    auto* blk = const_cast<asterix::block*>(frm->get_block(0));
    blk->set_modified(true);

    auto* rec = const_cast<asterix::record*>(blk->get_record(0));
    rec->set_modified(true);

    auto* re = const_cast<asterix::item*>(rec->get_item(28));
    re->set_modified(true);

    adam::buffer* encoded = nullptr;
    ASSERT_TRUE(encoder.encode(encoded, internal));
    ASSERT_NE(encoded, nullptr);
    EXPECT_EQ(encoded->get_size(), 17u);
    EXPECT_EQ(std::memcmp(encoded->get_begin(), raw_buf->get_begin(), 17), 0);

    adam::buffer_manager::get().return_buffer(encoded);
    adam::buffer_manager::get().return_buffer(internal);
    adam::buffer_manager::get().return_buffer(raw_buf);
}

TEST_F(encoder_test, benchmark_cat048_encoding)
{
    std::vector<uint8_t> raw = build_cat048_message();
    
    adam::buffer* raw_buf = adam::buffer_manager::get().request_buffer(static_cast<uint32_t>(raw.size()));
    std::memcpy(raw_buf->begin_as<uint8_t>(), raw.data(), raw.size());
    raw_buf->set_size(static_cast<uint32_t>(raw.size()));

    adam::buffer* internal_data = nullptr;
    ASSERT_TRUE(parser.parse(raw_buf, internal_data));

    // Force frame modification so we benchmark the actual encoding path, not the O(1) bypass
    auto* frm = internal_data->begin_as<asterix::frame>();
    frm->set_modified(true);

    auto* blk = const_cast<asterix::block*>(frm->get_block(0));
    blk->set_modified(true);

    auto* rec = const_cast<asterix::record*>(blk->get_record(0));
    rec->set_modified(true);

    // Set one item as modified to trigger encoding of this record
    auto* item = const_cast<asterix::item*>(rec->get_item(1));
    item->set_modified(true);

    constexpr int ITERATIONS = 100000;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < ITERATIONS; ++i)
    {
        adam::buffer* encoded = nullptr;
        bool success = encoder.encode(encoded, internal_data);
        if (success && encoded)
        {
            adam::buffer_manager::get().return_buffer(encoded);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    std::cout << "[          ] Encoded CAT048 message " << ITERATIONS << " times" << std::endl;
    std::cout << "[          ] Total duration: " << duration / 1000000.0 << " s" << std::endl;
    std::cout << "[          ] Average time per encode: " << static_cast<double>(duration) / ITERATIONS << " us" << std::endl;

    adam::buffer_manager::get().return_buffer(internal_data);
    adam::buffer_manager::get().return_buffer(raw_buf);
}
