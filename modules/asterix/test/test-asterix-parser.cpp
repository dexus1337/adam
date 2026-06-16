#include <gtest/gtest.h>
#include "data/asterix-parser.hpp"
#include "data/asterix-uap.hpp"
#include "data/asterix-internal.hpp"
#include "data/categories/48/cat048_uap.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "memory/buffer/buffer.hpp"
#include "data/asterix-types.hpp"

#include <string_view>

using namespace adam::modules;

class parser_test : public ::testing::Test 
{
protected:
    void SetUp() override 
    {
        adam::buffer_manager::get().initialize();
        parser = asterix::asterix_parser();
    }

    void TearDown() override 
    {
        adam::buffer_manager::get().destroy();
    }
    
    asterix::asterix_parser parser;
};

TEST_F(parser_test, test_dummy_parse)
{
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

    // Verify layout
    const auto* buf_header = internal_data->get_begin_as<asterix::frame>();
    EXPECT_EQ(buf_header->block_count, 1);
    
    const uint32_t* block_offsets = buf_header->get_block_offsets();
    const auto* block = reinterpret_cast<const asterix::block*>(
        reinterpret_cast<const uint8_t*>(buf_header) + block_offsets[0]
    );
    EXPECT_EQ(block->category, 48);
    EXPECT_EQ(block->record_count, 1);
    
    const uint32_t* record_offsets = block->get_record_offsets();
    const auto* record = reinterpret_cast<const asterix::record*>(
        reinterpret_cast<const uint8_t*>(buf_header) + record_offsets[0]
    );
    EXPECT_EQ(record->item_count, 2);
    
    const uint32_t* item_offsets = record->get_item_offsets();

    const auto* parsed_item_1 = reinterpret_cast<const asterix::item*>(
        reinterpret_cast<const uint8_t*>(buf_header) + item_offsets[0]
    );
    EXPECT_EQ(parsed_item_1->type, asterix::item_type_fixed);
    EXPECT_EQ(parsed_item_1->data_length, 2);
    
    const auto* parsed_item_2 = reinterpret_cast<const asterix::item*>(
        reinterpret_cast<const uint8_t*>(buf_header) + item_offsets[1]
    );
    EXPECT_EQ(parsed_item_2->type, asterix::item_type_fixed);
    EXPECT_EQ(parsed_item_2->data_length, 3);

    adam::buffer_manager::get().return_buffer(internal_data);
    adam::buffer_manager::get().return_buffer(raw_buf);
}

TEST_F(parser_test, test_cat062_variable_item_parse)
{
    adam::buffer* raw_buf = adam::buffer_manager::get().request_buffer(64);
    ASSERT_NE(raw_buf, nullptr);

    uint8_t* raw_data = raw_buf->begin_as<uint8_t>();
    
    // Build a valid CAT062 packet with variable item 510 (Composed Track Number, FRN 26)
    raw_data[0] = 62; // CAT
    raw_data[1] = 0;  // Length MSB
    raw_data[2] = 13; // Length LSB (3 bytes header + 4 bytes FSPEC + 6 bytes Composed Track Number = 13 bytes)
    
    // FSPEC: FRN 26 only, FX=1 on bytes 1, 2, 3 to extend to byte 4
    raw_data[3] = 0x01; // FSPEC byte 1: FX=1
    raw_data[4] = 0x01; // FSPEC byte 2: FX=1
    raw_data[5] = 0x01; // FSPEC byte 3: FX=1
    raw_data[6] = 0x08; // FSPEC byte 4: FRN 26 (bit 3) = 1, FX=0
    
    // Composed Track Number (Variable: first size 3, extent size 3)
    // First part: 3 bytes, last bit is FX. 0x33 has bit 0 = 1 (extended)
    raw_data[7] = 0x11;
    raw_data[8] = 0x22;
    raw_data[9] = 0x33;
    
    // Second part (extent): 3 bytes, last bit is FX. 0x66 has bit 0 = 0 (not extended)
    raw_data[10] = 0x44;
    raw_data[11] = 0x55;
    raw_data[12] = 0x66;
    
    raw_buf->set_size(13);

    adam::buffer* internal_data = nullptr;
    bool success = parser.parse(raw_buf, internal_data);

    EXPECT_TRUE(success);
    ASSERT_NE(internal_data, nullptr);
    EXPECT_GT(internal_data->get_size(), 0);

    // Verify layout
    const auto* buf_header = internal_data->get_begin_as<asterix::frame>();
    EXPECT_EQ(buf_header->block_count, 1);
    
    const uint32_t* block_offsets = buf_header->get_block_offsets();
    const auto* block = reinterpret_cast<const asterix::block*>(
        reinterpret_cast<const uint8_t*>(buf_header) + block_offsets[0]
    );
    EXPECT_EQ(block->category, 62);
    EXPECT_EQ(block->record_count, 1);
    
    const uint32_t* record_offsets = block->get_record_offsets();
    const auto* record = reinterpret_cast<const asterix::record*>(
        reinterpret_cast<const uint8_t*>(buf_header) + record_offsets[0]
    );
    EXPECT_EQ(record->item_count, 1);
    
    const uint32_t* item_offsets = record->get_item_offsets();

    const auto* parsed_item = reinterpret_cast<const asterix::item*>(
        reinterpret_cast<const uint8_t*>(buf_header) + item_offsets[0]
    );
    EXPECT_EQ(parsed_item->type, asterix::item_type_variable);
    EXPECT_EQ(parsed_item->data_length, 6);

    adam::buffer_manager::get().return_buffer(internal_data);
    adam::buffer_manager::get().return_buffer(raw_buf);
}

TEST_F(parser_test, test_multiple_blocks_and_records)
{
    adam::buffer* raw_buf = adam::buffer_manager::get().request_buffer(128);
    ASSERT_NE(raw_buf, nullptr);

    uint8_t* raw_data = raw_buf->begin_as<uint8_t>();
    
    // --- BLOCK 1: CAT 48 (2 records) ---
    // Record 1: 1 item (DSID, 2 bytes)
    // Record 2: 1 item (TOD, 3 bytes)
    // Total block size: 3 (hdr) + 3 (rec1: 1 byte fspec + 2 bytes DSID) + 4 (rec2: 1 byte fspec + 3 bytes TOD) = 10 bytes
    raw_data[0] = 48; // CAT
    raw_data[1] = 0;  // Length MSB
    raw_data[2] = 10; // Length LSB
    
    // Record 1
    raw_data[3] = 0x80; // FSPEC: DSID only (FRN 1)
    raw_data[4] = 0x11;
    raw_data[5] = 0x22;
    
    // Record 2
    raw_data[6] = 0x40; // FSPEC: TOD only (FRN 2)
    raw_data[7] = 0xAA;
    raw_data[8] = 0xBB;
    raw_data[9] = 0xCC;

    // --- BLOCK 2: CAT 62 (1 record) ---
    // Record 1: 1 item (Track Number, 2 bytes - FRN 3 is fixed, primary_header_size = 2 in CAT62)
    // Let's verify CAT 62 FRN 3 in cat062_uap.cpp if possible or just use DSID (FRN 1) in CAT 62
    // Let's check FRN 1 in CAT 62: it is I062/010 Data Source Identifier (2 bytes fixed)
    // Total block size: 3 (hdr) + 3 (rec1: 1 byte fspec + 2 bytes DSID) = 6 bytes
    raw_data[10] = 62; // CAT
    raw_data[11] = 0;  // Length MSB
    raw_data[12] = 6;  // Length LSB
    
    // Record 1 of block 2
    raw_data[13] = 0x80; // FSPEC: DSID (FRN 1)
    raw_data[14] = 0x99;
    raw_data[15] = 0x88;

    raw_buf->set_size(16);

    adam::buffer* internal_data = nullptr;
    bool success = parser.parse(raw_buf, internal_data);

    EXPECT_TRUE(success);
    ASSERT_NE(internal_data, nullptr);
    EXPECT_GT(internal_data->get_size(), 0);

    // Verify layout
    const auto* buf_header = internal_data->get_begin_as<asterix::frame>();
    EXPECT_EQ(buf_header->block_count, 2);
    
    const uint32_t* block_offsets = buf_header->get_block_offsets();
    
    // Block 1 (CAT 48)
    const auto* block_1 = reinterpret_cast<const asterix::block*>(
        reinterpret_cast<const uint8_t*>(buf_header) + block_offsets[0]
    );
    EXPECT_EQ(block_1->category, 48);
    EXPECT_EQ(block_1->record_count, 2);
    EXPECT_EQ(block_1->block_length, 10);
    EXPECT_EQ(block_1->raw_offset, 0);

    // Block 1, Record 1
    const uint32_t* rec_offsets_1 = block_1->get_record_offsets();
    const auto* rec_1_1 = reinterpret_cast<const asterix::record*>(
        reinterpret_cast<const uint8_t*>(buf_header) + rec_offsets_1[0]
    );
    EXPECT_EQ(rec_1_1->item_count, 1);
    EXPECT_EQ(rec_1_1->record_length, 3);
    EXPECT_EQ(rec_1_1->raw_offset, 3);

    const auto* item_1_1_1 = reinterpret_cast<const asterix::item*>(
        reinterpret_cast<const uint8_t*>(buf_header) + rec_1_1->get_item_offsets()[0]
    );
    EXPECT_EQ(item_1_1_1->type, asterix::item_type_fixed);
    EXPECT_EQ(item_1_1_1->data_length, 2);

    // Block 1, Record 2
    const auto* rec_1_2 = reinterpret_cast<const asterix::record*>(
        reinterpret_cast<const uint8_t*>(buf_header) + rec_offsets_1[1]
    );
    EXPECT_EQ(rec_1_2->item_count, 1);
    EXPECT_EQ(rec_1_2->record_length, 4);
    EXPECT_EQ(rec_1_2->raw_offset, 6);

    const auto* item_1_2_1 = reinterpret_cast<const asterix::item*>(
        reinterpret_cast<const uint8_t*>(buf_header) + rec_1_2->get_item_offsets()[0]
    );
    EXPECT_EQ(item_1_2_1->type, asterix::item_type_fixed);
    EXPECT_EQ(item_1_2_1->data_length, 3);

    // Block 2 (CAT 62)
    const auto* block_2 = reinterpret_cast<const asterix::block*>(
        reinterpret_cast<const uint8_t*>(buf_header) + block_offsets[1]
    );
    EXPECT_EQ(block_2->category, 62);
    EXPECT_EQ(block_2->record_count, 1);
    EXPECT_EQ(block_2->block_length, 6);
    EXPECT_EQ(block_2->raw_offset, 10);

    // Block 2, Record 1
    const uint32_t* rec_offsets_2 = block_2->get_record_offsets();
    const auto* rec_2_1 = reinterpret_cast<const asterix::record*>(
        reinterpret_cast<const uint8_t*>(buf_header) + rec_offsets_2[0]
    );
    EXPECT_EQ(rec_2_1->item_count, 1);
    EXPECT_EQ(rec_2_1->record_length, 3);
    EXPECT_EQ(rec_2_1->raw_offset, 13);

    const auto* item_2_1_1 = reinterpret_cast<const asterix::item*>(
        reinterpret_cast<const uint8_t*>(buf_header) + rec_2_1->get_item_offsets()[0]
    );
    EXPECT_EQ(item_2_1_1->type, asterix::item_type_fixed);
    EXPECT_EQ(item_2_1_1->data_length, 2);

    adam::buffer_manager::get().return_buffer(internal_data);
    adam::buffer_manager::get().return_buffer(raw_buf);
}


// Dummy classification data for testing custom expansions
class dummy_classification_data : public asterix::uap_expansion
{
public:
    int classification_level = 5;
};

TEST_F(parser_test, test_custom_uap_expansion_and_o1_lookup)
{
    auto& cat048 = asterix::get_cat048_uap();

    // Verify O(1) lookup works and names are intact
    const asterix::field_spec* spec_tod = cat048.get_spec(2); // FRN 2 is Time of Day
    ASSERT_NE(spec_tod, nullptr);
    EXPECT_EQ(spec_tod->type, asterix::item_type_fixed);
    EXPECT_EQ(std::string_view(spec_tod->name), "I048/140 Time of Day");

    // Register a custom metadata for FRN 29 (SP)
    dummy_classification_data my_metadata;
    my_metadata.classification_level = 42;
    cat048.expand_parameter(29, &my_metadata);

    // Retrieve the expansion metadata in O(1) time
    asterix::uap_expansion* expansion = cat048.get_expansion(29);
    ASSERT_EQ(expansion, &my_metadata);

    // Cast it back to verify custom attributes
    dummy_classification_data* retrieved_data = static_cast<dummy_classification_data*>(expansion);
    EXPECT_EQ(retrieved_data->classification_level, 42);
}

/// Helper to build a raw ASTERIX block that contains one CAT048 record
static std::vector<uint8_t> build_cat048_message()
{
    // -----------------------------------------------------------------
    // 1️⃣  Block header (fixed size, see asterix‑internal.hpp):
    //    - Category = 48
    //    - Length   = will be filled later
    // -----------------------------------------------------------------
    std::vector<uint8_t> raw;
    raw.push_back(48);                 // Category byte

    // Reserve two length bytes (big‑endian)
    raw.push_back(0);
    raw.push_back(0);

    // -----------------------------------------------------------------
    // 2️⃣  Record start – FSPEC (fixed‑size, 4 octets = ceil(28/8))
    // -----------------------------------------------------------------
    // All bits set – every item present
    raw.push_back(0xFF);   // Octet 1
    raw.push_back(0xFF);   // Octet 2
    raw.push_back(0xFF);   // Octet 3
    raw.push_back(0xFE);   // Octet 4

    // -----------------------------------------------------------------
    // 3️⃣  Payload – one value per item, using simple deterministic data
    // -----------------------------------------------------------------
    // Helper lambdas --------------------------------------------------
    auto push_u8 = [&](uint8_t v) { raw.push_back(v); };
    auto push_u16 = [&](uint16_t v) { raw.push_back(v >> 8); raw.push_back(v & 0xFF); };
    auto push_u24 = [&](uint32_t v) {
        raw.push_back((v >> 16) & 0xFF);
        raw.push_back((v >> 8) & 0xFF);
        raw.push_back(v & 0xFF);
    };
    auto push_u32 = [&](uint32_t v) {
        raw.push_back(v >> 24);
        raw.push_back((v >> 16) & 0xFF);
        raw.push_back((v >> 8) & 0xFF);
        raw.push_back(v & 0xFF);
    };
    // -----------------------------------------------------------------

    // 1 – Data Source Identifier (2 bytes)
    push_u16(0x0102);

    // 2 – Time of Day (3 bytes) – 0x030405
    push_u24(0x030405);

    // 3 – Target Report Descriptor (variable, no length byte, just payload)
    push_u8(0x66);

    // 4 – Measured Position (4 bytes)
    push_u32(0x11223344);

    // 5 – Mode‑3/A Code (2 bytes)
    push_u16(0xA5A5);

    // 6 – Flight Level (2 bytes)
    push_u16(0xB0B0);

    // 7 – Radar Plot Characteristics (compound, sub‑UAP 130)
    //    Sub‑UAP 130 has 7 fixed 1‑byte items.
    push_u8(0xFE);   // FSPEC for compound 130 (all sub‑items, no extension)
    push_u8(0x01);   // SRL
    push_u8(0x02);   // SSR
    push_u8(0x03);   // SAM
    push_u8(0x04);   // PRL
    push_u8(0x05);   // PAM
    push_u8(0x06);   // RDP
    push_u8(0x07);   // SPI

    // 8 – Aircraft Address (3 bytes)
    push_u24(0x0A0B0C);

    // 9 – Aircraft Identification (6 bytes – ASCII “ABCDEF”)
    raw.insert(raw.end(), {'A','B','C','D','E','F'});

    // 10 – BDS Register Data (repetitive, 8 bytes per entry, 1 repetition)
    push_u8(1); // repetition count (1)
    push_u32(0xDEADBEEF);    // first 4 bytes of entry
    push_u32(0xFEEDC0DE);    // second 4 bytes of entry

    // 11 – Track Number (2 bytes)
    push_u16(0x7777);

    // 12 – Calculated Position (4 bytes)
    push_u32(0x8899AABB);

    // 13 – Calculated Velocity (4 bytes)
    push_u32(0xCCDDEEFF);

    // 14 – Track Status (variable, multi‑octet payload using FX bits)
    //    First octet has FX=1 indicating another octet follows
    push_u8(0xE5); // example payload with FX=1 (bits 7‑1 are payload)
    //    Second octet ends the field (FX=0)
    push_u8(0x44); // final payload byte
    // No separate offset print for second byte of Track Status

    // 15 – Track Quality (4 bytes)
    push_u32(0x01020304);

    // 16 – Warning/Error Conditions (variable, multi‑octet payload using FX bits)
    //    First octet with FX=1
    push_u8(0xD3); // example payload with FX=1
    //    Second octet ends the field (FX=0)
    push_u8(0x14);
    // End of Warning/Error Conditions

    // 17 – Mode‑3/A Confidence (2 bytes)
    push_u16(0x1234);

    // 18 – Mode‑C Code & Confidence (4 bytes)
    push_u32(0x56789ABC);

    // 19 – Height (2 bytes)
    push_u16(0x9ABC);

    // 20 – Radial Doppler Speed (compound, sub‑UAP 120)
    //    Sub‑UAP 120: CAL (2 bytes) + RDS (repetitive, 6 bytes, 1 rep)
    push_u8(0xC0);          // FSPEC for compound 120 (all sub‑items, no extension)
    push_u16(0x1111);       // CAL
    push_u8(1);             // repetition count for RDS
    push_u32(0x22222222);   // first 4 bytes of RDS
    push_u16(0x3333);       // remaining 2 bytes of RDS (now total 6 bytes)

    // 21 – Comm/ACAS Capability (2 bytes)
    push_u16(0xDEAD);

    // 22 – ACAS RA Report (7 bytes)
    raw.insert(raw.end(), {0x01,0x02,0x03,0x04,0x05,0x06,0x07});

    // 23 – Mode‑1 Code (1 byte)
    push_u8(0x11);

    // 24 – Mode‑2 Code (2 bytes)
    push_u16(0x2222);

    // 25 – Mode‑1 Confidence (1 byte)
    push_u8(0x33);

    // 26 – Mode‑2 Confidence (2 bytes)
    push_u16(0x4444);

    // 27 – Special Purpose Field (explicit, empty payload)
    //    No sub‑items → only the FSPEC (0 octets) is present.
    //    Nothing to add.
    push_u8(0x06); // length of SPF
    push_u8(0x80); //FSPEC (1 item)
    push_u32(0x13371337); // dummy payload that should be ignored due to length=4 but FSPEC=0


    // 28 – Reserved Expansion Field (explicit, sub‑UAP REF)
    //    Sub‑UAP REF contains 8 items – we provide minimal deterministic data.
    //    All bits set in its fixed‑size FSPEC (ceil(8/8) = 1 octet):
    // 28 – Reserved Expansion Field (explicit, sub‑UAP REF)
    //    Sub‑UAP REF contains 8 items – we provide minimal deterministic data.
    //    All bits set in its fixed‑size FSPEC (ceil(8/8) = 1 octet):
    // Reserve placeholder for length byte (to be filled after items)
    size_t ref_len_offset = raw.size();
    raw.push_back(0); // placeholder for length
    // FSPEC for REF (all items present)
    push_u8(0xFF);

    // ---- MD5 sub‑UAP (7 fixed items) -------------------------------
    // FSPEC for MD5 (all 7 fixed items present: 7 bits → 1 octet)
    push_u8(0xFE);   // 11111110 = all 7 fixed bits set
    push_u8(0x01);   // Mode 5 Summary
    push_u8(0x02);   // PIN / Nat Origin / Mission Code (4 bytes)
    push_u8(0x03);   // Reported Position (6 bytes) – we insert 6 bytes
    raw.insert(raw.end(), {0x03,0x04,0x05,0x06,0x07,0x08});
    push_u8(0x07);   // GNSS‑derived Altitude (2 bytes) – insert 2 bytes
    raw.insert(raw.end(), {0x07,0x08});
    push_u8(0x09);   // Ext Mode 1 Code (2 bytes) – insert 2 bytes
    raw.insert(raw.end(), {0x09,0x0A});
    push_u8(0x0B);   // Time Offset (1 byte)
    push_u8(0x0C);   // X Pulse Presence (1 byte)

    // ---- M5N sub‑UAP (8 fixed items) -------------------------------
    // FSPEC for M5N (all 8 fixed items present: 8 bits → 1 octet)
    push_u8(0xFF);   // 11111111 = all 8 fixed bits set, 7 items + expansion
    push_u8(0x80);   // 10000000 = first item present
    push_u8(0x0D);   // Mode 5 Summary
    raw.insert(raw.end(), {0x11,0x22,0x33,0x44}); // PIN / Nat Origin (4 bytes)
    raw.insert(raw.end(), {0x12,0x13,0x14,0x15,0x16,0x17}); // Reported Position (6 bytes)
    raw.insert(raw.end(), {0x18,0x19}); // GNSS‑derived Altitude (2 bytes)
    raw.insert(raw.end(), {0x1A,0x1B}); // Ext Mode 1 Code (2 bytes)
    push_u8(0x1C);   // Time Offset (1 byte)
    push_u8(0x1D);   // X Pulse Presence (1 byte)
    push_u8(0x1E);   // Figure of Merit (1 byte)

    // ---- M4E sub‑UAP (variable) ------------------------------------
    push_u8(0x01);   // 
    push_u8(0x02);   //
    

    // ---- RPC sub‑UAP (4 fixed items) -------------------------------
    push_u8(0xF0);   // FSPEC
    push_u8(0x20);   // Score
    push_u8(0x21);   // Signal/Clutter Ratio (2 bytes)
    push_u8(0x22);
    push_u8(0x23);   // Range Width (2 bytes)
    push_u8(0x24);
    push_u8(0x25);   // Ambiguous Range (2 bytes)
    push_u8(0x26);

    std::cout << "1: " << (raw.size()) << std::endl;


    // ---- ERR Extended Range Reports (3 fixed length) --------------
    push_u8(0x01);
    push_u8(0x02);
    push_u8(0x03);


    // ---- RTC sub‑UAP (7 fixed + 2 repetitive) --------------------
    push_u8(0xFF);   // FSPEC
    push_u8(0xF0);   // FSPEC
    push_u8(0x30);   // Plot/Track Link (3 bytes)
    raw.insert(raw.end(), {0x30,0x31});
    push_u8(0x01);   // ADS‑B/Track Link (repetitive, 2 bytes, 1 rep) repetition count = 1
    push_u8(0x41);   // first byte of link
    push_u8(0x42);   // second byte of link
    push_u8(0x50);   // Turn State (1 byte)
    raw.insert(raw.end(), 22, 0x51); // Next Predicted Position (22 bytes)
    push_u8(0x01);   // Data Link Characteristics (repetitive, 1 byte, 1 rep) rep count = 1
    push_u8(0x61);   // payload
    push_u8(0x70);   // Lockout Characteristics (2 bytes)
    push_u8(0x71);
    raw.insert(raw.end(), 6, 0x80); // Transition Codes (6 bytes)
    raw.insert(raw.end(), 4, 0x90); // Track Lifecycle (4 bytes)
    push_u8(0x01);   // Adjacent Sensor Information (repetitive, 8 bytes, 1 rep) rep count = 1
    raw.insert(raw.end(), 8, 0xA0);
    push_u8(0xB0);   // Track Extrapolation Source (1 byte)
    push_u8(0xC0);   // Identity Requested (1 byte)

    // ---- CPC sub‑UAP (4 fixed items) -------------------------------
    push_u8(0xF0);   // FSPEC
    push_u8(0xD0);   // Plot Number (2 bytes)
    push_u8(0xD1);
    push_u8(0x01);   // Replies/Plot Link (repetitive, 3 bytes, 1 rep) rep count = 1
    push_u8(0xD3);
    push_u8(0xD4);
    push_u8(0xD5);   // Scan Number (1 byte)
    push_u8(0xD6);   // Date (4 bytes)
    raw.insert(raw.end(), 4, 0xD6);

    // ---- GEN48 sub‑UAP (5 fixed items) -----------------------------
    push_u8(0xF8);   // FSPEC
    push_u8(0xE1);   // Alt Mode 2 (2 bytes)
    push_u8(0xE2);
    push_u8(0xE2);   // Alt Mode 3/A (2 bytes)
    push_u8(0xE3);
    push_u8(0xE3);   // Alt Flight Level (2 bytes)
    push_u8(0xE4);
    push_u8(0xE4);   // Radar Cross Section (dBm²) (2 bytes)
    push_u8(0xE5);
    push_u8(0xE5);   // Radar Cross Section (m²) (4 bytes)
    push_u8(0xE6);
    push_u8(0xE6);
    push_u8(0xE7);

    // Compute and fill REF length (including length byte itself)
    size_t ref_len = raw.size() - ref_len_offset;
    raw[ref_len_offset] = static_cast<uint8_t>(ref_len);
    printf("REF payload length set to %zu at offset %zu\n", ref_len, ref_len_offset);


    printf("Total raw message size: %zu\n", raw.size());
    
    // -----------------------------------------------------------------
    // 4️⃣  Fill length fields (block length = raw.size())
    // -----------------------------------------------------------------
    uint16_t block_len = static_cast<uint16_t>(raw.size());
    raw[1] = static_cast<uint8_t>(block_len >> 8);
    raw[2] = static_cast<uint8_t>(block_len & 0xFF);

    return raw;
}

TEST_F(parser_test, parse_full_cat048_message)
{
    // Build the raw block:
    std::vector<uint8_t> raw = build_cat048_message();

    // Parse using the library:

    adam::buffer* data = adam::buffer_manager::get().request_buffer(static_cast<uint32_t>(raw.size()));
    std::memcpy(data->begin_as<uint8_t>(), raw.data(), raw.size());
    data->set_size(static_cast<uint32_t>(raw.size()));
    
    adam::buffer* internal = nullptr;
    ASSERT_TRUE(parser.parse(data, internal));

    // --------------------------------------------------------------
    // Verify the top‑level block & record count
    // --------------------------------------------------------------
    ASSERT_NE(internal, nullptr);
    auto* frm = reinterpret_cast<asterix::frame*>(internal->begin());
    EXPECT_EQ(frm->block_count, 1u);    

    // Locate the first and only block and verify its header fields:
    const asterix::block* blk = reinterpret_cast<const asterix::block*>(internal->begin_as<uint8_t>() + frm->get_block_offsets()[0]);
    EXPECT_EQ(blk->category, 48u);
    EXPECT_EQ(blk->record_count, 1u);

    const asterix::record* rec = reinterpret_cast<const asterix::record*>(internal->begin_as<uint8_t>() + blk->get_record_offsets()[0]);
    EXPECT_EQ(rec->item_count, 28u);
    EXPECT_EQ(rec->record_length, raw.size()-rec->raw_offset);

    
    
    /*// Extract the first record payload:
    const uint8_t* rec = internal->begin_as<uint8_t>() + blk->get_record_offsets()[0];
    const uint8_t* cur = rec;

    // Helper to read a field and advance pointer
    auto read_u8  = [&](uint8_t& out){ out = *cur++; };
    auto read_u16 = [&](uint16_t& out){
        out = static_cast<uint16_t>(cur[0] << 8 | cur[1]); cur += 2;
    };
    auto read_u24 = [&](uint32_t& out){
        out = static_cast<uint32_t>(cur[0] << 16 | cur[1] << 8 | cur[2]); cur += 3;
    };
    auto read_u32 = [&](uint32_t& out){
        out = static_cast<uint32_t>(cur[0] << 24 | cur[1] << 16 | cur[2] << 8 | cur[3]); cur += 4;
    };

    // ---- Verify each item against the values we encoded ----
    uint16_t v16; uint32_t v32; uint8_t v8;

    read_u16(v16); EXPECT_EQ(v16, 0x0102);               // 1 – Data Source Identifier
    read_u24(v32); EXPECT_EQ(v32, 0x030405);            // 2 – Time of Day
    read_u8 (v8);  EXPECT_EQ(v8,  0x66);                // 3 – Target Report Descriptor
    read_u32(v32); EXPECT_EQ(v32, 0x11223344);          // 4 – Measured Position
    read_u16(v16); EXPECT_EQ(v16, 0xA5A5);               // 5 – Mode‑3/A Code
    read_u16(v16); EXPECT_EQ(v16, 0xB0B0);               // 6 – Flight Level

    // 7 – Radar Plot Characteristics (sub‑UAP 130 – 7 bytes)
    for (uint8_t i = 1; i <= 7; ++i) { read_u8(v8); EXPECT_EQ(v8, i); }

    read_u24(v32); EXPECT_EQ(v32, 0x0A0B0C);            // 8 – Aircraft Address
    // 9 – Aircraft Identification (6 ASCII chars)
    EXPECT_EQ(std::string(reinterpret_cast<const char*>(cur), 6), "ABCDEF"); cur += 6;

    // 10 – BDS Register Data (rep = 1, 8 bytes)
    read_u32(v32); EXPECT_EQ(v32, 0xDEADBEEF);
    read_u32(v32); EXPECT_EQ(v32, 0xFEEDC0DE);

    read_u16(v16); EXPECT_EQ(v16, 0x7777);              // 11 – Track Number
    read_u32(v32); EXPECT_EQ(v32, 0x8899AABB);          // 12 – Calculated Position
    read_u32(v32); EXPECT_EQ(v32, 0xCCDDEEFF);          // 13 – Calculated Velocity
    read_u8 (v8);  EXPECT_EQ(v8,  0x44);                // 14 – Track Status
    read_u32(v32); EXPECT_EQ(v32, 0x01020304);          // 15 – Track Quality
    read_u8 (v8);  EXPECT_EQ(v8,  0x55);                // 16 – Warning/Error Conditions
    read_u16(v16); EXPECT_EQ(v16, 0x1234);              // 17 – Mode‑3/A Confidence
    read_u32(v32); EXPECT_EQ(v32, 0x56789ABC);          // 18 – Mode‑C Code & Confidence
    read_u16(v16); EXPECT_EQ(v16, 0x9ABC);              // 19 – Height

    // 20 – Radial Doppler Speed (sub‑UAP 120)
    read_u16(v16); EXPECT_EQ(v16, 0x1111);              // CAL
    // RDS (6 bytes, we stored 4 + 2)
    read_u32(v32); EXPECT_EQ(v32, 0x22222222);
    read_u8 (v8);  EXPECT_EQ(v8,  0x33);

    read_u16(v16); EXPECT_EQ(v16, 0xDEAD);              // 21 – Comm/ACAS Capability

    // 22 – ACAS Resolution Advisory Report (7 bytes)
    for (uint8_t i = 1; i <= 7; ++i) { read_u8(v8); EXPECT_EQ(v8, i); }

    read_u8 (v8);  EXPECT_EQ(v8, 0x11);                // 23 – Mode‑1 Code
    read_u16(v16); EXPECT_EQ(v16, 0x2222);              // 24 – Mode‑2 Code
    read_u8 (v8);  EXPECT_EQ(v8, 0x33);                // 25 – Mode‑1 Confidence
    read_u16(v16); EXPECT_EQ(v16, 0x4444);              // 26 – Mode‑2 Confidence

    // 27 – Special Purpose Field – no payload (nothing to read)

    // 28 – Reserved Expansion Field (REF sub‑UAP)
    // 1‑byte FSPEC already consumed, now read its 8 items:
    // MD5 sub‑UAP (7 fixed)
    for (uint8_t i = 1; i <= 7; ++i) { read_u8(v8); EXPECT_EQ(v8, i); }
    // M5N sub‑UAP (8 fixed)
    for (uint8_t i = 0x0D; i <= 0x1E; ++i) { read_u8(v8); EXPECT_EQ(v8, i); }
    // RPC sub‑UAP (4 fixed)
    for (uint8_t i = 0x20; i <= 0x26; ++i) { read_u8(v8); EXPECT_EQ(v8, i); }
    // RTC sub‑UAP (mixed)
    read_u8(v8); EXPECT_EQ(v8, 0x30); // Plot/Track Link (3 bytes)
    read_u8(v8); EXPECT_EQ(v8, 0x31);
    // ADS‑B/Track Link – repetition count 1
    read_u8(v8); EXPECT_EQ(v8, 0x01);
    read_u8(v8); EXPECT_EQ(v8, 0x41);
    read_u8(v8); EXPECT_EQ(v8, 0x42);
    read_u8(v8); EXPECT_EQ(v8, 0x50); // Turn State
    // Next Predicted Position (22 bytes – all 0x51)
    for (int i = 0; i < 22; ++i) { read_u8(v8); EXPECT_EQ(v8, 0x51); }
    // Data Link Characteristics – repetition count 1
    read_u8(v8); EXPECT_EQ(v8, 0x01);
    read_u8(v8); EXPECT_EQ(v8, 0x61);
    // Lockout Characteristics (2 bytes)
    read_u8(v8); EXPECT_EQ(v8, 0x70);
    read_u8(v8); EXPECT_EQ(v8, 0x71);
    // Transition Codes (6 bytes)
    for (int i = 0; i < 6; ++i) { read_u8(v8); EXPECT_EQ(v8, 0x80); }
    // Track Lifecycle (4 bytes)
    for (int i = 0; i < 4; ++i) { read_u8(v8); EXPECT_EQ(v8, 0x90); }
    // Adjacent Sensor Info – repetition count 1
    read_u8(v8); EXPECT_EQ(v8, 0x01);
    for (int i = 0; i < 8; ++i) { read_u8(v8); EXPECT_EQ(v8, 0xA0); }
    read_u8(v8); EXPECT_EQ(v8, 0xB0); // Extrapolation Source
    read_u8(v8); EXPECT_EQ(v8, 0xC0); // Identity Requested

    // CPC sub‑UAP (4 items)
    read_u8(v8); EXPECT_EQ(v8, 0xD0); // Plot Number (2 bytes)
    read_u8(v8); EXPECT_EQ(v8, 0xD1);
    read_u8(v8); EXPECT_EQ(v8, 0x01); // Replies/Plot Link repetition = 1
    read_u8(v8); EXPECT_EQ(v8, 0xD3);
    read_u8(v8); EXPECT_EQ(v8, 0xD4);
    read_u8(v8); EXPECT_EQ(v8, 0xD5); // Scan Number
    read_u8(v8); EXPECT_EQ(v8, 0xD6); // Date (4 bytes)
    for (int i = 0; i < 4; ++i) { read_u8(v8); EXPECT_EQ(v8, 0xD6); }

    // GEN48 sub‑UAP (5 items)
    read_u8(v8); EXPECT_EQ(v8, 0xE1);
    read_u8(v8); EXPECT_EQ(v8, 0xE2);
    read_u8(v8); EXPECT_EQ(v8, 0xE3);
    read_u8(v8); EXPECT_EQ(v8, 0xE4);
    read_u8(v8); EXPECT_EQ(v8, 0xE5);

    // --------------------------------------------------------------
    // Cleanup
    // --------------------------------------------------------------*/
    adam::buffer_manager::get().return_buffer(internal);
}