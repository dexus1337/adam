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

TEST_F(parser_test, parse_fixed)
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
    EXPECT_GT(internal_data->get_size(), 0ul);

    // Verify layout
    const auto* buf_header = internal_data->get_begin_as<asterix::frame>();
    EXPECT_EQ(buf_header->block_count, 1);

    auto block = buf_header->get_block(0);
    ASSERT_NE(block, nullptr);
    EXPECT_FALSE(block->has_next());
    EXPECT_EQ(block->category, 48);
    EXPECT_EQ(block->record_count, 1);
    EXPECT_EQ(block->raw_length, 9);
    EXPECT_EQ(block->raw_offset, 0ul);

    auto record = block->get_record(0);
    ASSERT_NE(record, nullptr);
    EXPECT_FALSE(record->has_next());
    EXPECT_EQ(record->item_count, block->get_uap()->get_highest_frn()); // no children, so item count = last FRN
    EXPECT_EQ(record->fspec_size, 1);
    EXPECT_EQ(record->raw_length, 6);
    EXPECT_EQ(record->raw_offset, 3ul);
    EXPECT_EQ(record->category, 48);

    auto item0 = record->get_item(0);
    EXPECT_EQ(item0, nullptr);

    auto item1 = record->get_item(1);
    ASSERT_NE(item1, nullptr);
    EXPECT_TRUE(item1->is_populated());
    EXPECT_EQ(item1->type, asterix::item_type_fixed);
    EXPECT_EQ(item1->raw_length, 2ul);
    EXPECT_EQ(item1->raw_offset, 4ul);

    auto item2 = record->get_item(2);
    ASSERT_NE(item2, nullptr);
    EXPECT_TRUE(item2->is_populated());
    EXPECT_EQ(item2->type, asterix::item_type_fixed);
    EXPECT_EQ(item2->raw_length, 3ul);
    EXPECT_EQ(item2->raw_offset, 6ul);

    auto item3 = record->get_item(3);
    ASSERT_NE(item3, nullptr);
    EXPECT_FALSE(item3->is_populated());

    adam::buffer_manager::get().return_buffer(internal_data);
    adam::buffer_manager::get().return_buffer(raw_buf);
}

TEST_F(parser_test, parse_variable)
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
    EXPECT_GT(internal_data->get_size(), 0ul);

    const auto* buf_header = internal_data->get_begin_as<asterix::frame>();
    EXPECT_EQ(buf_header->block_count, 1);
    
    auto block = buf_header->get_block(0);
    ASSERT_NE(block, nullptr);
    EXPECT_FALSE(block->has_next());
    EXPECT_EQ(block->category, 62);
    EXPECT_EQ(block->record_count, 1);
    EXPECT_EQ(block->raw_length, 13ul);
    EXPECT_EQ(block->raw_offset, 0ul);

    auto record = block->get_record(0);
    ASSERT_NE(record, nullptr);
    EXPECT_FALSE(record->has_next());
    EXPECT_FALSE(record->has_next());
    EXPECT_EQ(record->item_count, block->get_uap()->get_highest_frn()); // no children, so item count = last FRN
    EXPECT_EQ(record->fspec_size, 4);
    EXPECT_EQ(record->raw_length, 10ul);
    EXPECT_EQ(record->raw_offset, 3ul);
    EXPECT_EQ(record->category, 62);

    auto item26 = record->get_item(26);
    ASSERT_NE(item26, nullptr);
    EXPECT_TRUE(item26->is_populated());
    EXPECT_EQ(item26->type, asterix::item_type_variable);
    EXPECT_EQ(item26->raw_length, 6ul);
    EXPECT_EQ(item26->raw_offset, 7ul);

    adam::buffer_manager::get().return_buffer(internal_data);
    adam::buffer_manager::get().return_buffer(raw_buf);
}

TEST_F(parser_test, parse_repetitive)
{
    // ---------------------------------------------------------------
    // Test a repetitive item in isolation.
    //
    // We use CAT048 FRN 10 – I048/250 BDS Register Data:
    //   header_size = 1  (1-byte repetition count)
    //   data_size   = 8  (8 bytes per repetition)
    //
    // Packet layout with N=2 repetitions:
    //   [0]      Category = 48
    //   [1..2]   Length (big-endian) = 22
    //   [3]      FSPEC byte 1: no items, FX=1 → 0x01
    //   [4]      FSPEC byte 2: FRN 10 (bit 5) = 1, FX=0 → 0x20
    //              FRNs 1-7 are in byte 1 (bit 0 = FX).
    //              FRN 10 is in byte 2: bit 7=FRN8, bit 6=FRN9, bit 5=FRN10
    //   [5]      Repetition count = 2
    //   [6..13]  Repetition 1 (8 bytes)
    //   [14..21] Repetition 2 (8 bytes)
    //
    // Total = 3 (block hdr) + 2 (FSPEC) + 1 (rep-count) + 2×8 (payload) = 22 bytes
    // ---------------------------------------------------------------

    adam::buffer* raw_buf = adam::buffer_manager::get().request_buffer(64);
    ASSERT_NE(raw_buf, nullptr);

    uint8_t* raw_data = raw_buf->begin_as<uint8_t>();

    constexpr uint8_t  N_REPS    = 2;
    constexpr uint16_t DATA_SIZE = 8;
    // total = 3 (block hdr) + 2 (FSPEC) + 1 (rep count) + N_REPS * DATA_SIZE
    constexpr uint16_t TOTAL_LEN = 3 + 2 + 1 + N_REPS * DATA_SIZE;

    raw_data[0] = 48;              // CAT
    raw_data[1] = 0;               // Length MSB
    raw_data[2] = TOTAL_LEN;       // Length LSB

    // FSPEC: 2 bytes – only FRN 10 present
    raw_data[3] = 0x01;            // FSPEC byte 1: empty, FX=1
    raw_data[4] = 0x20;            // FSPEC byte 2: FRN 10 (bit 5) = 1, FX=0

    // Repetitive item: 1-byte rep count + N_REPS * 8 bytes
    raw_data[5] = N_REPS;          // repetition count

    // Repetition 1
    raw_data[6]  = 0x11;
    raw_data[7]  = 0x22;
    raw_data[8]  = 0x33;
    raw_data[9]  = 0x44;
    raw_data[10] = 0x55;
    raw_data[11] = 0x66;
    raw_data[12] = 0x77;
    raw_data[13] = 0x88;

    // Repetition 2
    raw_data[14] = 0xAA;
    raw_data[15] = 0xBB;
    raw_data[16] = 0xCC;
    raw_data[17] = 0xDD;
    raw_data[18] = 0xEE;
    raw_data[19] = 0xFF;
    raw_data[20] = 0x01;
    raw_data[21] = 0x02;

    raw_buf->set_size(TOTAL_LEN);

    adam::buffer* internal_data = nullptr;
    bool success = parser.parse(raw_buf, internal_data);

    EXPECT_TRUE(success);
    ASSERT_NE(internal_data, nullptr);
    EXPECT_GT(internal_data->get_size(), 0ul);

    // ── Frame ────────────────────────────────────────────────────────
    const auto* buf_header = internal_data->get_begin_as<asterix::frame>();
    EXPECT_EQ(buf_header->block_count, 1u);

    // ── Block ────────────────────────────────────────────────────────
    const auto* block = buf_header->get_block(0);
    ASSERT_NE(block, nullptr);
    EXPECT_FALSE(block->has_next());
    EXPECT_EQ(block->category,     48);
    EXPECT_EQ(block->record_count, 1u);
    EXPECT_EQ(block->raw_length,   TOTAL_LEN);
    EXPECT_EQ(block->raw_offset,   0ul);

    // ── Record ───────────────────────────────────────────────────────
    const auto* record = block->get_record(0);
    ASSERT_NE(record, nullptr);
    EXPECT_FALSE(record->has_next());
    EXPECT_EQ(record->item_count,  block->get_uap()->get_highest_frn());
    EXPECT_EQ(record->fspec_size,  2u);
    EXPECT_EQ(record->raw_offset,  3ul);
    EXPECT_EQ(record->raw_length,  static_cast<uint16_t>(TOTAL_LEN - 3));
    EXPECT_EQ(record->category,    48);

    // FRNs 1-9 and 11-28 must not be populated
    for (uint8_t frn = 1; frn <= 28; ++frn)
    {
        if (frn == 10) continue;
        const auto* item = record->get_item(frn);
        ASSERT_NE(item, nullptr)  << "FRN " << frn << " item must exist";
        EXPECT_FALSE(item->is_populated()) << "FRN " << frn << " must not be populated";
    }

    // ── FRN 10 – I048/250 BDS Register Data (repetitive) ────────────
    //  raw_offset = 5  (3 block hdr + 2 FSPEC)
    //  raw_length = 1 (rep-count byte) + N_REPS * DATA_SIZE = 17
    const auto* item10 = record->get_item(10);
    ASSERT_NE(item10, nullptr);
    EXPECT_TRUE(item10->is_populated());
    EXPECT_EQ(item10->type,       asterix::item_type_repetetive);
    EXPECT_EQ(item10->raw_offset, 5ul);
    EXPECT_EQ(item10->raw_length, static_cast<uint16_t>(1 + N_REPS * DATA_SIZE));

    adam::buffer_manager::get().return_buffer(internal_data);
    adam::buffer_manager::get().return_buffer(raw_buf);
}

TEST_F(parser_test, parse_compound)
{
    // Create a dummy raw buffer representing an Asterix packet
    adam::buffer* raw_buf = adam::buffer_manager::get().request_buffer(64);
    ASSERT_NE(raw_buf, nullptr);

    uint8_t* raw_data = raw_buf->begin_as<uint8_t>();

    // Build a valid CAT062 packet
    raw_data[0] = 62; // CAT
    raw_data[1] = 0;  // Length MSB
    raw_data[2] = 15;  // Calculated Later
    
    // FSPEC: I0062/110 (FRN 24)
    // Bit 7 is FRN 1, bit 6 is FRN 2. So 11000000 = 0xC0. FX=0.
    raw_data[3] = 0x01; // Byte 1 empty, only fx
    raw_data[4] = 0x01; // Byte 2 empty, only fx
    raw_data[5] = 0x01; // Byte 3 empty, only fx
    raw_data[6] = 0b00100000; // FRN24 set
    
    // FRN 24: Mode 5 Data reports & Extended Mode 1 Code (Compound)
    raw_data[7] = 0b10100000; // FRN 1 SUM and FRN 3 POS set
    
    // CHILD FRN 1: SUM
    raw_data[8] = 0b10101010; // random data

    // CHILD FRN 3: POS
    raw_data[9] = 0x11; // Lat
    raw_data[10] = 0x22;
    raw_data[11] = 0x33;
    raw_data[12] = 0x44; // Long
    raw_data[13] = 0x55;
    raw_data[14] = 0x66;

    raw_buf->set_size(15);

    adam::buffer* internal_data = nullptr;
    bool success = parser.parse(raw_buf, internal_data);

    EXPECT_TRUE(success);
    ASSERT_NE(internal_data, nullptr);
    EXPECT_GT(internal_data->get_size(), 0ul);

    // Verify layout
    const auto* buf_header = internal_data->get_begin_as<asterix::frame>();
    EXPECT_EQ(buf_header->block_count, 1);

    auto block = buf_header->get_block(0);
    ASSERT_NE(block, nullptr);
    EXPECT_FALSE(block->has_next());
    EXPECT_EQ(block->category, 62);
    EXPECT_EQ(block->record_count, 1);
    EXPECT_EQ(block->raw_length, 15);
    EXPECT_EQ(block->raw_offset, 0);

    auto record = block->get_record(0);
    ASSERT_NE(record, nullptr);
    EXPECT_FALSE(record->has_next());
    auto uap = record->find_used_uap();
    EXPECT_EQ(record->item_count, uap->get_highest_frn() + uap->get_spec(24)->sub_uap->get_highest_frn());
    EXPECT_EQ(record->fspec_size, 4);
    EXPECT_EQ(record->raw_length, 15 - 3);
    EXPECT_EQ(record->raw_offset, 3);

    for (uint8_t i = 1; i <= uap->get_highest_frn(); i++)
    {
        auto* item = record->get_item(i);
        ASSERT_NE(item, nullptr);

        EXPECT_EQ(item->is_populated(), i == 24);
    }

    auto item24 = record->get_item(24);
    ASSERT_NE(item24, nullptr);
    EXPECT_TRUE(item24->is_populated());
    EXPECT_EQ(item24->type, asterix::item_type_compound);
    EXPECT_EQ(item24->raw_length, 8);
    EXPECT_EQ(item24->raw_offset, 7);

    auto child_item1 = item24->get_child_item(1);
    ASSERT_NE(child_item1, nullptr);
    EXPECT_TRUE(child_item1->is_populated());
    EXPECT_EQ(child_item1->type, asterix::item_type_fixed);
    EXPECT_EQ(child_item1->raw_length, 1);
    EXPECT_EQ(child_item1->raw_offset, 8);

    auto child_item2 = item24->get_child_item(2);
    ASSERT_NE(child_item2, nullptr);
    EXPECT_FALSE(child_item2->is_populated());

    auto child_item3 = item24->get_child_item(3);
    ASSERT_NE(child_item3, nullptr);
    EXPECT_TRUE(child_item3->is_populated());
    EXPECT_EQ(child_item3->type, asterix::item_type_fixed);
    EXPECT_EQ(child_item3->raw_length, 6);
    EXPECT_EQ(child_item3->raw_offset, 9);


    adam::buffer_manager::get().return_buffer(internal_data);
    adam::buffer_manager::get().return_buffer(raw_buf);
}

TEST_F(parser_test, parse_explicit_no_uap)
{
    adam::buffer* raw_buf = adam::buffer_manager::get().request_buffer(64);
    ASSERT_NE(raw_buf, nullptr);

    uint8_t* raw_data = raw_buf->begin_as<uint8_t>();

    // Build a CAT048 packet that contains only FRN 27 (SP – Special Purpose Field).
    //
    // SP is an explicit item: its first byte is the total length (including that
    // length byte itself), followed by (length - 1) bytes of payload.
    //
    // FSPEC byte layout for CAT048:
    //   Byte 1:  FRNs 1-7  (bit 0 = FX)
    //   Byte 2:  FRNs 8-14 (bit 0 = FX)
    //   Byte 3:  FRNs 15-21 (bit 0 = FX)
    //   Byte 4:  FRNs 22-28 (bit 0 = FX, no further extension)
    //
    // FRN 27 sits in FSPEC byte 4, bit 2 (0-indexed from MSB):
    //   bit 7 = FRN 22, bit 6 = FRN 23, ..., bit 2 = FRN 27, bit 1 = FRN 28, bit 0 = FX
    //   → 0b00000100 = 0x04
    //
    // Total packet: 3 (header) + 4 (FSPEC) + 5 (SP: 1 length + 4 payload) = 12 bytes

    raw_data[0] = 48;  // CAT
    raw_data[1] = 0;   // Length MSB
    raw_data[2] = 12;  // Length LSB

    // FSPEC: extend through bytes 1-3 (FX=1), then FRN 27 in byte 4 (FX=0)
    raw_data[3] = 0x01; // FSPEC byte 1: no items, FX=1
    raw_data[4] = 0x01; // FSPEC byte 2: no items, FX=1
    raw_data[5] = 0x01; // FSPEC byte 3: no items, FX=1
    raw_data[6] = 0x04; // FSPEC byte 4: FRN 27 (bit 2) = 1, FX=0

    // SP explicit item: length byte = 5 (includes itself) + 4 payload bytes
    raw_data[7]  = 0x05; // length byte
    raw_data[8]  = 0xDE; // payload byte 1
    raw_data[9]  = 0xAD; // payload byte 2
    raw_data[10] = 0xBE; // payload byte 3
    raw_data[11] = 0xEF; // payload byte 4

    raw_buf->set_size(12);

    adam::buffer* internal_data = nullptr;
    bool success = parser.parse(raw_buf, internal_data);

    EXPECT_TRUE(success);
    ASSERT_NE(internal_data, nullptr);
    EXPECT_GT(internal_data->get_size(), 0ul);

    // Verify layout
    const auto* buf_header = internal_data->get_begin_as<asterix::frame>();
    EXPECT_EQ(buf_header->block_count, 1);

    auto block = buf_header->get_block(0);
    ASSERT_NE(block, nullptr);
    EXPECT_FALSE(block->has_next());
    EXPECT_EQ(block->category,     48);
    EXPECT_EQ(block->record_count, 1);
    EXPECT_EQ(block->raw_length,   12);
    EXPECT_EQ(block->raw_offset,   0ul);

    auto record = block->get_record(0);
    ASSERT_NE(record, nullptr);
    EXPECT_FALSE(record->has_next());
    EXPECT_EQ(record->item_count,  block->get_uap()->get_highest_frn());
    EXPECT_EQ(record->raw_length,  9);   // 12 - 3 header bytes
    EXPECT_EQ(record->raw_offset,  3ul);
    EXPECT_EQ(record->category,    48);

    // FRNs 1-26 and 28 must not be populated
    for (uint8_t frn = 1; frn <= 28; ++frn)
    {
        if (frn == 27) continue;
        auto* item = record->get_item(frn);
        ASSERT_NE(item, nullptr) << "FRN " << frn << " item must exist (even if not populated)";
        EXPECT_FALSE(item->is_populated()) << "FRN " << frn << " must not be populated";
    }

    // FRN 27 – SP explicit item
    auto item27 = record->get_item(27);
    ASSERT_NE(item27, nullptr);
    EXPECT_TRUE(item27->is_populated());
    EXPECT_EQ(item27->type,       asterix::item_type_explicit);
    EXPECT_EQ(item27->raw_length, 5u);    // value of the length byte
    EXPECT_EQ(item27->raw_offset, 7ul);   // immediately after the 4-byte FSPEC

    adam::buffer_manager::get().return_buffer(internal_data);
    adam::buffer_manager::get().return_buffer(raw_buf);
}

TEST_F(parser_test, parse_explicit_with_uap)
{
    adam::buffer* raw_buf = adam::buffer_manager::get().request_buffer(64);
    ASSERT_NE(raw_buf, nullptr);

    uint8_t* raw_data = raw_buf->begin_as<uint8_t>();

    raw_data[0] = 48;  // CAT
    raw_data[1] = 0;   // Length MSB
    raw_data[2] = 17;  // Length LSB

    // FSPEC
    raw_data[3] = 0x01; // byte 1: FX=1
    raw_data[4] = 0x01; // byte 2: FX=1
    raw_data[5] = 0x01; // byte 3: FX=1
    raw_data[6] = 0x02; // byte 4: FRN 28 set, FX=0

    // RE explicit item
    raw_data[7]  = 0x0A; // length = 10 (includes itself)
    raw_data[8]  = 0x10; // REF FSPEC: only RPC (FRN 4) present

    // RPC compound
    raw_data[9]  = 0xF0; // RPC FSPEC: all 4 children present
    raw_data[10] = 0x20; // Score (1 B)
    raw_data[11] = 0x21; // Signal/Clutter Ratio MSB
    raw_data[12] = 0x22; // Signal/Clutter Ratio LSB
    raw_data[13] = 0x23; // Range Width MSB
    raw_data[14] = 0x24; // Range Width LSB
    raw_data[15] = 0x25; // Ambiguous Range MSB
    raw_data[16] = 0x26; // Ambiguous Range LSB

    raw_buf->set_size(17);

    adam::buffer* internal_data = nullptr;
    ASSERT_TRUE(parser.parse(raw_buf, internal_data));
    ASSERT_NE(internal_data, nullptr);
    EXPECT_GT(internal_data->get_size(), 0ul);

    // ── Frame ────────────────────────────────────────────────────────
    const auto* buf_header = internal_data->get_begin_as<asterix::frame>();
    EXPECT_EQ(buf_header->block_count, 1);

    // ── Block ────────────────────────────────────────────────────────
    const auto* block = buf_header->get_block(0);
    ASSERT_NE(block, nullptr);
    EXPECT_FALSE(block->has_next());
    EXPECT_EQ(block->category,     48);
    EXPECT_EQ(block->record_count, 1);
    EXPECT_EQ(block->raw_length,   17);
    EXPECT_EQ(block->raw_offset,   0ul);

    // ── Record ───────────────────────────────────────────────────────
    const auto* record = block->get_record(0);
    ASSERT_NE(record, nullptr);
    EXPECT_FALSE(record->has_next());
    EXPECT_EQ(record->item_count,  block->get_uap()->get_highest_frn() + block->get_uap()->get_spec(28)->sub_uap->get_highest_frn() + block->get_uap()->get_spec(28)->sub_uap->get_spec(4)->sub_uap->get_highest_frn());
    EXPECT_EQ(record->raw_length,  14);    // 17 - 3 block header bytes
    EXPECT_EQ(record->raw_offset,  3ul);
    EXPECT_EQ(record->category,    48);

    // FRNs 1-27 must not be populated
    for (uint8_t frn = 1; frn <= 27; ++frn)
    {
        const auto* item = record->get_item(frn);
        ASSERT_NE(item, nullptr) << "FRN " << frn << " item must exist";
        EXPECT_FALSE(item->is_populated()) << "FRN " << frn << " must not be populated";
    }

    // ── FRN 28: RE (explicit, backed by REF sub-UAP) ─────────────────
    const auto* re = record->get_item(28);
    ASSERT_NE(re, nullptr);
    EXPECT_TRUE(re->is_populated());
    EXPECT_EQ(re->type,       asterix::item_type_explicit);
    EXPECT_EQ(re->raw_length, 10u);   // value of the length byte
    EXPECT_EQ(re->raw_offset, 7ul);   // immediately after the 4-byte FSPEC

    // REF children FRN 1-3, 5-8 must not be populated (only RPC = FRN 4 was set)
    for (int child_frn = 1; child_frn <= 8; ++child_frn)
    {
        if (child_frn == 4) continue;
        const auto* ch = re->get_child_item(child_frn);
        ASSERT_NE(ch, nullptr) << "REF child FRN " << child_frn << " must exist";
        EXPECT_FALSE(ch->is_populated()) << "REF child FRN " << child_frn << " must not be populated";
    }

    // ── REF child FRN 4: RPC compound ────────────────────────────────
    //    1 FSPEC byte + 1 + 2 + 2 + 2 data bytes = 8 bytes total
    const auto* rpc = re->get_child_item(4);
    ASSERT_NE(rpc, nullptr);
    EXPECT_TRUE(rpc->is_populated());
    EXPECT_EQ(rpc->type,       asterix::item_type_compound);
    EXPECT_EQ(rpc->raw_length, 8u);    // 1 FSPEC + 1 + 2 + 2 + 2
    EXPECT_EQ(rpc->raw_offset, 9ul);   // REF FSPEC at [8], RPC starts at [9]

    // ── RPC child FRN 1: Score (fixed, 1 B) ──────────────────────────
    const auto* score = rpc->get_child_item(1);
    ASSERT_NE(score, nullptr);
    EXPECT_TRUE(score->is_populated());
    EXPECT_EQ(score->type,       asterix::item_type_fixed);
    EXPECT_EQ(score->raw_length, 1u);
    EXPECT_EQ(score->raw_offset, 10ul);  // right after RPC FSPEC at [9]

    // ── RPC child FRN 2: Signal/Clutter Ratio (fixed, 2 B) ───────────
    const auto* scr = rpc->get_child_item(2);
    ASSERT_NE(scr, nullptr);
    EXPECT_TRUE(scr->is_populated());
    EXPECT_EQ(scr->type,       asterix::item_type_fixed);
    EXPECT_EQ(scr->raw_length, 2u);
    EXPECT_EQ(scr->raw_offset, 11ul);

    // ── RPC child FRN 3: Range Width (fixed, 2 B) ────────────────────
    const auto* rw = rpc->get_child_item(3);
    ASSERT_NE(rw, nullptr);
    EXPECT_TRUE(rw->is_populated());
    EXPECT_EQ(rw->type,       asterix::item_type_fixed);
    EXPECT_EQ(rw->raw_length, 2u);
    EXPECT_EQ(rw->raw_offset, 13ul);

    // ── RPC child FRN 4: Ambiguous Range (fixed, 2 B) ────────────────
    const auto* ar = rpc->get_child_item(4);
    ASSERT_NE(ar, nullptr);
    EXPECT_TRUE(ar->is_populated());
    EXPECT_EQ(ar->type,       asterix::item_type_fixed);
    EXPECT_EQ(ar->raw_length, 2u);
    EXPECT_EQ(ar->raw_offset, 15ul);

    adam::buffer_manager::get().return_buffer(internal_data);
    adam::buffer_manager::get().return_buffer(raw_buf);
}

TEST_F(parser_test, parse_alternative_uap)
{
    // ---------------------------------------------------------------
    // CAT001 uses a base UAP with a selector function to dispatch
    // to either the PLOT or TRACK alternative UAP, based on the
    // msg_type bit inside FRN 2 (I001/020 Target Report Descriptor).
    //
    // Packet layout (both variants share the same block header and
    // FSPEC format):
    //
    //   [0]     Category = 1
    //   [1..2]  Length (big-endian)
    //   [3]     FSPEC byte 1
    //   ...     payload
    //
    // FSPEC byte 1 for both packets below:
    //   bit 7 = FRN1 (DSID, 2 B fixed)
    //   bit 6 = FRN2 (TRD, variable 1 B, FX=0)
    //   bit 5 = FRN3
    //   ...
    //   bit 0 = FX (extension indicator, 0 = no more FSPEC bytes)
    //
    // target_report_descriptor bit layout (MSB first, as laid out in memory):
    //   bit 7 (MSB): msg_type  (0=plot, 1=track)
    //   bit 6:       orig
    //   bit 5-4:     detect_type
    //   bit 3:       antenna_num
    //   bit 2:       special_pos
    //   bit 1:       rep_type
    //   bit 0 (LSB): extended  (FX)
    // ---------------------------------------------------------------

    // ================================================================
    // Sub-test A: PLOT record
    // ================================================================
    // FRNs present: 1 (DSID 2B), 2 (TRD 1B, FX=0), 3 (Measured Pos 4B)
    // FSPEC byte 1: FRN1=1, FRN2=1, FRN3=1, FX=0  ->  1110 0000 = 0xE0
    // TRD byte: msg_type=PLOT(0) -> MSB=0, all other bits=0, FX=0
    //   => 0b00000000 = 0x00
    //
    // Total:  3 (header) + 1 (FSPEC) + 2 (DSID) + 1 (TRD) + 4 (MeasPos) = 11 bytes
    {
        adam::buffer* raw_buf = adam::buffer_manager::get().request_buffer(64);
        ASSERT_NE(raw_buf, nullptr);

        uint8_t* d = raw_buf->begin_as<uint8_t>();

        d[0] = 1;    // CAT001
        d[1] = 0;    // Length MSB
        d[2] = 11;   // Length LSB
        d[3] = 0xE0; // FSPEC: FRN1, FRN2, FRN3 present, no extension

        // FRN 1 - DSID (2 B)
        d[4] = 0x0A;
        d[5] = 0x0B;

        // FRN 2 - TRD (1 B, FX=0 -> no extension)
        //   msg_type = PLOT (0) -> MSB = 0, everything else 0
        //   => 0b00000000 = 0x00
        d[6] = 0x00;

        // FRN 3 - Measured Position in Polar Coordinates (4 B)
        d[7]  = 0x11;
        d[8]  = 0x22;
        d[9]  = 0x33;
        d[10] = 0x44;

        raw_buf->set_size(11);

        adam::buffer* internal = nullptr;
        ASSERT_TRUE(parser.parse(raw_buf, internal));
        ASSERT_NE(internal, nullptr);

        // -- Frame / Block --
        const auto* frm = internal->get_begin_as<asterix::frame>();
        ASSERT_EQ(frm->block_count, 1u);

        const auto* blk = frm->get_block(0);
        ASSERT_NE(blk, nullptr);
        EXPECT_FALSE(blk->has_next());
        EXPECT_EQ(blk->category,     1);
        EXPECT_EQ(blk->record_count, 1u);
        EXPECT_EQ(blk->raw_length,   11u);
        EXPECT_EQ(blk->raw_offset,   0u);

        // -- Record --
        const auto* rec = blk->get_record(0);
        ASSERT_NE(rec, nullptr);
        EXPECT_FALSE(rec->has_next());
        EXPECT_EQ(rec->category,   1);
        EXPECT_EQ(rec->fspec_size, 1u);
        EXPECT_EQ(rec->raw_offset, 3u);   // immediately after 3-byte block header
        EXPECT_EQ(rec->raw_length, 8u);   // 11 - 3

        // The block-level UAP must be the base CAT001 UAP.
        const auto* base_uap = blk->get_uap();
        ASSERT_NE(base_uap, nullptr);
        EXPECT_EQ(base_uap, &asterix::cat001::get_uap());

        // The record must have been parsed with the PLOT alternative UAP.
        // find_used_uap() must return the PLOT UAP, not the base UAP.
        const auto* used_uap = rec->find_used_uap();
        ASSERT_NE(used_uap, nullptr);
        EXPECT_NE(used_uap, base_uap) << "PLOT record must not use the base UAP";

        // Verify the correct alternative is selected via the base UAP's find_alternative()
        const auto* expected_plot_uap = base_uap->find_alternative(used_uap->get_name());
        EXPECT_EQ(used_uap, expected_plot_uap) << "find_used_uap() must return the PLOT alternative";

        // The PLOT UAP has 15 FRNs (3 FSPEC bytes)
        EXPECT_EQ(used_uap->get_highest_frn(), 15u);

        // item_count must match the PLOT UAP (not the base)
        EXPECT_EQ(rec->item_count, used_uap->get_highest_frn());

        // -- FRN 1 - DSID (fixed, 2 B) --
        const auto* dsid = rec->get_item(1);
        ASSERT_NE(dsid, nullptr);
        EXPECT_TRUE(dsid->is_populated());
        EXPECT_EQ(dsid->type,       asterix::item_type_fixed);
        EXPECT_EQ(dsid->raw_length, 2u);
        EXPECT_EQ(dsid->raw_offset, 4u);  // 3 hdr + 1 FSPEC

        // -- FRN 2 - TRD (variable, 1 B, no FX extension) --
        const auto* trd = rec->get_item(2);
        ASSERT_NE(trd, nullptr);
        EXPECT_TRUE(trd->is_populated());
        EXPECT_EQ(trd->type,       asterix::item_type_variable);
        EXPECT_EQ(trd->raw_length, 1u);
        EXPECT_EQ(trd->raw_offset, 6u);   // 4 + 2 (DSID)

        // -- FRN 3 - Measured Position in Polar Coordinates (fixed, 4 B) --
        // In the PLOT UAP, FRN 3 = I001/040 Measured Position (4 B fixed)
        const auto* meas_pos = rec->get_item(3);
        ASSERT_NE(meas_pos, nullptr);
        EXPECT_TRUE(meas_pos->is_populated());
        EXPECT_EQ(meas_pos->type,       asterix::item_type_fixed);
        EXPECT_EQ(meas_pos->raw_length, 4u);
        EXPECT_EQ(meas_pos->raw_offset, 7u);  // 6 + 1 (TRD)

        adam::buffer_manager::get().return_buffer(internal);
        adam::buffer_manager::get().return_buffer(raw_buf);
    }

    // ================================================================
    // Sub-test B: TRACK record
    // ================================================================
    // FRNs present: 1 (DSID 2B), 2 (TRD 1B), 3 (Track/Plot# 2B), 4 (MeasPos 4B)
    // FSPEC byte 1: FRN1=1, FRN2=1, FRN3=1, FRN4=1, FX=0  ->  0xF0
    // TRD byte: msg_type=TRACK(1) -> MSB=1, everything else=0, FX=0
    //   => 0b10000000 = 0x80
    //
    // Total:  3 (header) + 1 (FSPEC) + 2 (DSID) + 1 (TRD) + 2 (Track#) + 4 (MeasPos) = 13 bytes
    {
        adam::buffer* raw_buf = adam::buffer_manager::get().request_buffer(64);
        ASSERT_NE(raw_buf, nullptr);

        uint8_t* d = raw_buf->begin_as<uint8_t>();

        d[0] = 1;    // CAT001
        d[1] = 0;    // Length MSB
        d[2] = 13;   // Length LSB
        d[3] = 0xF0; // FSPEC: FRN1, FRN2, FRN3, FRN4 present, no extension

        // FRN 1 - DSID (2 B)
        d[4] = 0x0C;
        d[5] = 0x0D;

        // FRN 2 - TRD (1 B, FX=0)
        //   msg_type = TRACK (1) -> MSB = 1, everything else 0
        //   => 0b10000000 = 0x80
        d[6] = 0x80;

        // FRN 3 - Track/Plot Number (2 B)  [TRACK UAP FRN 3 = I001/161]
        d[7] = 0x55;
        d[8] = 0x66;

        // FRN 4 - Measured Position in Polar Coordinates (4 B)  [TRACK UAP FRN 4 = I001/040]
        d[9]  = 0xAA;
        d[10] = 0xBB;
        d[11] = 0xCC;
        d[12] = 0xDD;

        raw_buf->set_size(13);

        adam::buffer* internal = nullptr;
        ASSERT_TRUE(parser.parse(raw_buf, internal));
        ASSERT_NE(internal, nullptr);

        // -- Frame / Block --
        const auto* frm = internal->get_begin_as<asterix::frame>();
        ASSERT_EQ(frm->block_count, 1u);

        const auto* blk = frm->get_block(0);
        ASSERT_NE(blk, nullptr);
        EXPECT_FALSE(blk->has_next());
        EXPECT_EQ(blk->category,     1);
        EXPECT_EQ(blk->record_count, 1u);
        EXPECT_EQ(blk->raw_length,   13u);
        EXPECT_EQ(blk->raw_offset,   0u);

        // -- Record --
        const auto* rec = blk->get_record(0);
        ASSERT_NE(rec, nullptr);
        EXPECT_FALSE(rec->has_next());
        EXPECT_EQ(rec->category,   1);
        EXPECT_EQ(rec->fspec_size, 1u);
        EXPECT_EQ(rec->raw_offset, 3u);
        EXPECT_EQ(rec->raw_length, 10u);  // 13 - 3

        // The block-level UAP must still be the base CAT001 UAP.
        const auto* base_uap = blk->get_uap();
        ASSERT_NE(base_uap, nullptr);
        EXPECT_EQ(base_uap, &asterix::cat001::get_uap());

        // The record must have been parsed with the TRACK alternative UAP.
        // find_used_uap() must return the TRACK UAP, not the base UAP.
        const auto* used_uap = rec->find_used_uap();
        ASSERT_NE(used_uap, nullptr);
        EXPECT_NE(used_uap, base_uap) << "TRACK record must not use the base UAP";

        // Verify the correct alternative is selected via the base UAP's find_alternative()
        const auto* expected_track_uap = base_uap->find_alternative(used_uap->get_name());
        EXPECT_EQ(used_uap, expected_track_uap) << "find_used_uap() must return the TRACK alternative";

        // The TRACK UAP has 14 FRNs (2 FSPEC bytes)
        EXPECT_EQ(used_uap->get_highest_frn(), 14u);

        // item_count must match the TRACK UAP (not the base)
        EXPECT_EQ(rec->item_count, used_uap->get_highest_frn());

        // -- FRN 1 - DSID (fixed, 2 B) --
        const auto* dsid = rec->get_item(1);
        ASSERT_NE(dsid, nullptr);
        EXPECT_TRUE(dsid->is_populated());
        EXPECT_EQ(dsid->type,       asterix::item_type_fixed);
        EXPECT_EQ(dsid->raw_length, 2u);
        EXPECT_EQ(dsid->raw_offset, 4u);  // 3 hdr + 1 FSPEC

        // -- FRN 2 - TRD (variable, 1 B) --
        const auto* trd = rec->get_item(2);
        ASSERT_NE(trd, nullptr);
        EXPECT_TRUE(trd->is_populated());
        EXPECT_EQ(trd->type,       asterix::item_type_variable);
        EXPECT_EQ(trd->raw_length, 1u);
        EXPECT_EQ(trd->raw_offset, 6u);   // 4 + 2 (DSID)

        // -- FRN 3 - Track/Plot Number (fixed, 2 B) --
        // In the TRACK UAP, FRN 3 = I001/161 Track/Plot Number (2 B fixed).
        // This is the key differentiator from the PLOT UAP where FRN 3 is
        // I001/040 Measured Position (4 B fixed).
        const auto* track_num = rec->get_item(3);
        ASSERT_NE(track_num, nullptr);
        EXPECT_TRUE(track_num->is_populated());
        EXPECT_EQ(track_num->type,       asterix::item_type_fixed);
        EXPECT_EQ(track_num->raw_length, 2u);
        EXPECT_EQ(track_num->raw_offset, 7u);  // 6 + 1 (TRD)

        // -- FRN 4 - Measured Position (fixed, 4 B) --
        // In the TRACK UAP, FRN 4 = I001/040 Measured Position (4 B fixed).
        const auto* meas_pos = rec->get_item(4);
        ASSERT_NE(meas_pos, nullptr);
        EXPECT_TRUE(meas_pos->is_populated());
        EXPECT_EQ(meas_pos->type,       asterix::item_type_fixed);
        EXPECT_EQ(meas_pos->raw_length, 4u);
        EXPECT_EQ(meas_pos->raw_offset, 9u);  // 7 + 2 (Track#)

        adam::buffer_manager::get().return_buffer(internal);
        adam::buffer_manager::get().return_buffer(raw_buf);
    }
}

TEST_F(parser_test, parse_multiple_blocks_and_records)
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
    EXPECT_GT(internal_data->get_size(), 0ul);

    // Verify layout
    const auto* buf_header = internal_data->get_begin_as<asterix::frame>();
    EXPECT_EQ(buf_header->block_count, 2);
    
    auto block48 = buf_header->get_block(0);
    ASSERT_NE(block48, nullptr);
    EXPECT_TRUE(block48->has_next());
    EXPECT_EQ(block48->category, 48);
    EXPECT_EQ(block48->record_count, 2);
    EXPECT_EQ(block48->raw_length, 10);
    EXPECT_EQ(block48->raw_offset, 0);

    auto record48_0 = block48->get_record(0);
    ASSERT_NE(record48_0, nullptr);
    EXPECT_TRUE(record48_0->has_next());
    EXPECT_EQ(record48_0->item_count, record48_0->find_used_uap()->get_highest_frn()); // no children, so item count = last FRN
    EXPECT_EQ(record48_0->fspec_size, 1);
    EXPECT_EQ(record48_0->raw_length, 3);
    EXPECT_EQ(record48_0->raw_offset, 3);
    EXPECT_EQ(record48_0->category, 48);

    auto record48_1 = block48->get_record(1);
    ASSERT_NE(record48_1, nullptr);
    EXPECT_EQ(record48_0->get_next(), record48_1);
    EXPECT_FALSE(record48_1->has_next());
    EXPECT_EQ(record48_1->item_count, record48_1->find_used_uap()->get_highest_frn()); // no children, so item count = last FRN
    EXPECT_EQ(record48_1->fspec_size, 1);
    EXPECT_EQ(record48_1->raw_length, 4);
    EXPECT_EQ(record48_1->raw_offset, 6);
    EXPECT_EQ(record48_1->category, 48);

    auto block62 = buf_header->get_block(1);
    ASSERT_NE(block62, nullptr);
    EXPECT_EQ(block48->get_next(), block62);
    EXPECT_FALSE(block62->has_next());
    EXPECT_EQ(block62->category, 62);
    EXPECT_EQ(block62->record_count, 1);
    EXPECT_EQ(block62->raw_length, 6);
    EXPECT_EQ(block62->raw_offset, 10);

    auto record62_0 = block62->get_record(0);
    ASSERT_NE(record62_0, nullptr);
    EXPECT_FALSE(record62_0->has_next());
    EXPECT_EQ(record62_0->item_count, record62_0->find_used_uap()->get_highest_frn()); // no children, so item count = last FRN
    EXPECT_EQ(record62_0->raw_length, 3);
    EXPECT_EQ(record62_0->raw_offset, 13);
    EXPECT_EQ(record62_0->category, 62);

    adam::buffer_manager::get().return_buffer(internal_data);
    adam::buffer_manager::get().return_buffer(raw_buf);
}

TEST_F(parser_test, parse_multiple_children)
{
    adam::buffer* raw_buf = adam::buffer_manager::get().request_buffer(64);
    ASSERT_NE(raw_buf, nullptr);

    uint8_t* raw_data = raw_buf->begin_as<uint8_t>();
    
    // Block Header: CAT 48, length = 17
    raw_data[0] = 48;
    raw_data[1] = 0;
    raw_data[2] = 17;

    // FSPEC: FRN 7 and FRN 20 active
    raw_data[3] = 0x03; // FRN 7 set, FX=1
    raw_data[4] = 0x01; // FX=1
    raw_data[5] = 0x04; // FRN 20 set, FX=0

    // FRN 7 compound (Radar Plot Characteristics, UAP 130)
    // FSPEC of UAP 130: 0xFE (all 7 items present)
    raw_data[6] = 0xFE;
    // 7 bytes data
    raw_data[7] = 0x11;
    raw_data[8] = 0x12;
    raw_data[9] = 0x13;
    raw_data[10] = 0x14;
    raw_data[11] = 0x15;
    raw_data[12] = 0x16;
    raw_data[13] = 0x17;

    // FRN 20 compound (Radial Doppler Speed, UAP 120)
    // FSPEC of UAP 120: 0x80 (only CAL present)
    raw_data[14] = 0x80;
    // 2 bytes data
    raw_data[15] = 0x99;
    raw_data[16] = 0x99;

    raw_buf->set_size(17);

    adam::buffer* internal_data = nullptr;
    bool success = parser.parse(raw_buf, internal_data);

    EXPECT_TRUE(success);
    ASSERT_NE(internal_data, nullptr);

    const auto* buf_header = internal_data->get_begin_as<asterix::frame>();
    auto block = buf_header->get_block(0);
    auto record = block->get_record(0);

    auto item7 = record->get_item(7);
    ASSERT_NE(item7, nullptr);
    EXPECT_TRUE(item7->is_populated());

    auto item20 = record->get_item(20);
    ASSERT_NE(item20, nullptr);
    EXPECT_TRUE(item20->is_populated());

    // Verify FRN 7's child 1 (SRL): raw_offset should be 7, raw_length should be 1
    auto srl = item7->get_child_item(1);
    ASSERT_NE(srl, nullptr);
    EXPECT_TRUE(srl->is_populated());
    EXPECT_EQ(srl->raw_offset, 7u);
    EXPECT_EQ(srl->raw_length, 1u);

    // Verify FRN 20's child 1 (CAL): raw_offset should be 15, raw_length should be 2
    auto cal = item20->get_child_item(1);
    ASSERT_NE(cal, nullptr);
    EXPECT_TRUE(cal->is_populated());
    EXPECT_EQ(cal->raw_offset, 15u);
    EXPECT_EQ(cal->raw_length, 2u);

    adam::buffer_manager::get().return_buffer(internal_data);
    adam::buffer_manager::get().return_buffer(raw_buf);
}


// Dummy classification data for testing custom expansions
class dummy_classification_data : public asterix::uap_expansion
{
public:
    int classification_level = 5;
};

TEST_F(parser_test, custom_uap_expansion_and_o1_lookup)
{
    auto& cat048 = asterix::cat048::get_uap();

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

    // ---------------------------------------------------------------
    // Frame – one CAT048 block
    // ---------------------------------------------------------------
    ASSERT_NE(internal, nullptr);
    const auto* frm = internal->get_begin_as<asterix::frame>();
    EXPECT_EQ(frm->block_count, 1u);    

    // ---------------------------------------------------------------
    // Block
    // ---------------------------------------------------------------
    const auto* blk = frm->get_block(0);
    ASSERT_NE(blk, nullptr);
    EXPECT_EQ(blk->category,      48);
    EXPECT_EQ(blk->record_count,  1u);
    EXPECT_EQ(blk->raw_length,    static_cast<uint16_t>(raw.size()));
    EXPECT_EQ(blk->raw_offset,    0u);

    // ---------------------------------------------------------------
    // Record – all 28 FRNs are present (FSPEC = FF FF FF FE)
    // ---------------------------------------------------------------
    const auto* rec = blk->get_record(0);
    ASSERT_NE(rec, nullptr);
    EXPECT_EQ(rec->category,    48);
    EXPECT_EQ(rec->item_count,  84);
    EXPECT_EQ(rec->raw_offset,  3u);                              // after 3-byte block header
    EXPECT_EQ(rec->raw_length,  static_cast<uint16_t>(raw.size() - 3));

    // ---------------------------------------------------------------
    // Helper: verify a simple (non-compound) item
    // ---------------------------------------------------------------
    auto check_item = [&](uint8_t frn, asterix::item_type expected_type,
                          uint16_t expected_raw_len, uint32_t expected_raw_off)
    {
        const auto* it = rec->get_item(frn);
        ASSERT_NE(it, nullptr)             << "FRN " << (int)frn << " must exist";
        EXPECT_TRUE(it->is_populated())         << "FRN " << (int)frn << " must be populated";
        EXPECT_EQ(it->type, expected_type) << "FRN " << (int)frn << " type mismatch";
        EXPECT_EQ(it->raw_length, expected_raw_len) << "FRN " << (int)frn << " raw_length mismatch";
        EXPECT_EQ(it->raw_offset, expected_raw_off) << "FRN " << (int)frn << " raw_offset mismatch";
    };

    // ---------------------------------------------------------------
    // FRN 1  – I048/010 Data Source Identifier          (fixed, 2 B)
    // ---------------------------------------------------------------
    check_item(1, asterix::item_type_fixed, 2, 7u);

    // ---------------------------------------------------------------
    // FRN 2  – I048/140 Time of Day                     (fixed, 3 B)
    // ---------------------------------------------------------------
    check_item(2, asterix::item_type_fixed, 3, 9u);

    // ---------------------------------------------------------------
    // FRN 3  – I048/020 Target Report Descriptor        (variable, 1 B, FX=0)
    // ---------------------------------------------------------------
    check_item(3, asterix::item_type_variable, 1, 12u);

    // ---------------------------------------------------------------
    // FRN 4  – I048/040 Measured Position               (fixed, 4 B)
    // ---------------------------------------------------------------
    check_item(4, asterix::item_type_fixed, 4, 13u);

    // ---------------------------------------------------------------
    // FRN 5  – I048/070 Mode-3/A Code                   (fixed, 2 B)
    // ---------------------------------------------------------------
    check_item(5, asterix::item_type_fixed, 2, 17u);

    // ---------------------------------------------------------------
    // FRN 6  – I048/090 Flight Level                    (fixed, 2 B)
    // ---------------------------------------------------------------
    check_item(6, asterix::item_type_fixed, 2, 19u);

    // ---------------------------------------------------------------
    // FRN 7  – I048/130 Radar Plot Characteristics      (compound)
    //          FSPEC=0xFE → all 7 sub-items present, 8 B total
    // ---------------------------------------------------------------
    {
        const auto* it = rec->get_item(7);
        ASSERT_NE(it, nullptr);
        EXPECT_TRUE(it->is_populated());
        EXPECT_EQ(it->type,       asterix::item_type_compound);
        EXPECT_EQ(it->raw_length, 8u);   // 1 FSPEC + 7×1 B
        EXPECT_EQ(it->raw_offset, 21u);

        // All 7 children are 1-byte fixed items starting at offset 22
        auto check_child = [&](uint8_t child_frn, uint32_t off)
        {
            const auto* ch = it->get_child_item(child_frn);
            ASSERT_NE(ch, nullptr)             << "I048/130 child FRN " << (int)child_frn;
            EXPECT_TRUE(ch->is_populated())         << "I048/130 child FRN " << (int)child_frn;
            EXPECT_EQ(ch->type, asterix::item_type_fixed) << "I048/130 child FRN " << (int)child_frn;
            EXPECT_EQ(ch->raw_length, 1u)      << "I048/130 child FRN " << (int)child_frn;
            EXPECT_EQ(ch->raw_offset, off)     << "I048/130 child FRN " << (int)child_frn;
        };

        check_child(1, 22u);  // SRL
        check_child(2, 23u);  // SSR
        check_child(3, 24u);  // SAM
        check_child(4, 25u);  // PRL
        check_child(5, 26u);  // PAM
        check_child(6, 27u);  // RDP
        check_child(7, 28u);  // SPI
    }

    // ---------------------------------------------------------------
    // FRN 8  – I048/220 Aircraft Address                (fixed, 3 B)
    // ---------------------------------------------------------------
    check_item(8, asterix::item_type_fixed, 3, 29u);

    // ---------------------------------------------------------------
    // FRN 9  – I048/240 Aircraft Identification         (fixed, 6 B)
    // ---------------------------------------------------------------
    check_item(9, asterix::item_type_fixed, 6, 32u);

    // ---------------------------------------------------------------
    // FRN 10 – I048/250 BDS Register Data               (repetitive, 1 rep × 8 B = 9 B)
    // ---------------------------------------------------------------
    check_item(10, asterix::item_type_repetetive, 9, 38u);

    // ---------------------------------------------------------------
    // FRN 11 – I048/161 Track Number                    (fixed, 2 B)
    // ---------------------------------------------------------------
    check_item(11, asterix::item_type_fixed, 2, 47u);

    // ---------------------------------------------------------------
    // FRN 12 – I048/042 Calculated Position             (fixed, 4 B)
    // ---------------------------------------------------------------
    check_item(12, asterix::item_type_fixed, 4, 49u);

    // ---------------------------------------------------------------
    // FRN 13 – I048/200 Calculated Track Velocity       (fixed, 4 B)
    // ---------------------------------------------------------------
    check_item(13, asterix::item_type_fixed, 4, 53u);

    // ---------------------------------------------------------------
    // FRN 14 – I048/170 Track Status                    (variable, 2 B, FX=1 then FX=0)
    // ---------------------------------------------------------------
    check_item(14, asterix::item_type_variable, 2, 57u);

    // ---------------------------------------------------------------
    // FRN 15 – I048/210 Track Quality                   (fixed, 4 B)
    // ---------------------------------------------------------------
    check_item(15, asterix::item_type_fixed, 4, 59u);

    // ---------------------------------------------------------------
    // FRN 16 – I048/030 Warning/Error Conditions        (variable, 2 B)
    // ---------------------------------------------------------------
    check_item(16, asterix::item_type_variable, 2, 63u);

    // ---------------------------------------------------------------
    // FRN 17 – I048/080 Mode-3/A Code Confidence        (fixed, 2 B)
    // ---------------------------------------------------------------
    check_item(17, asterix::item_type_fixed, 2, 65u);

    // ---------------------------------------------------------------
    // FRN 18 – I048/100 Mode-C Code & Confidence        (fixed, 4 B)
    // ---------------------------------------------------------------
    check_item(18, asterix::item_type_fixed, 4, 67u);

    // ---------------------------------------------------------------
    // FRN 19 – I048/110 Height (3D Radar)               (fixed, 2 B)
    // ---------------------------------------------------------------
    check_item(19, asterix::item_type_fixed, 2, 71u);

    // ---------------------------------------------------------------
    // FRN 20 – I048/120 Radial Doppler Speed            (compound)
    //          FSPEC=0xC0 → children 1 (CAL) and 2 (RDS) present
    //          10 B total: 1 FSPEC + 2 CAL + 1 repCount + 6 RDS
    // ---------------------------------------------------------------
    {
        const auto* it = rec->get_item(20);
        ASSERT_NE(it, nullptr);
        EXPECT_TRUE(it->is_populated());
        EXPECT_EQ(it->type,       asterix::item_type_compound);
        EXPECT_EQ(it->raw_length, 10u);
        EXPECT_EQ(it->raw_offset, 73u);

        // Child 1: CAL – fixed, 2 B
        const auto* cal = it->get_child_item(1);
        ASSERT_NE(cal, nullptr);
        EXPECT_TRUE(cal->is_populated());
        EXPECT_EQ(cal->type,       asterix::item_type_fixed);
        EXPECT_EQ(cal->raw_length, 2u);
        EXPECT_EQ(cal->raw_offset, 74u);

        // Child 2: RDS – repetitive, 1 rep × 6 B = 7 B (includes rep count byte)
        const auto* rds = it->get_child_item(2);
        ASSERT_NE(rds, nullptr);
        EXPECT_TRUE(rds->is_populated());
        EXPECT_EQ(rds->type,       asterix::item_type_repetetive);
        EXPECT_EQ(rds->raw_length, 7u);
        EXPECT_EQ(rds->raw_offset, 76u);
    }

    // ---------------------------------------------------------------
    // FRN 21 – I048/230 Comm/ACAS Capability            (fixed, 2 B)
    // ---------------------------------------------------------------
    check_item(21, asterix::item_type_fixed, 2, 83u);

    // ---------------------------------------------------------------
    // FRN 22 – I048/260 ACAS Resolution Advisory        (fixed, 7 B)
    // ---------------------------------------------------------------
    check_item(22, asterix::item_type_fixed, 7, 85u);

    // ---------------------------------------------------------------
    // FRN 23 – I048/055 Mode-1 Code                     (fixed, 1 B)
    // ---------------------------------------------------------------
    check_item(23, asterix::item_type_fixed, 1, 92u);

    // ---------------------------------------------------------------
    // FRN 24 – I048/050 Mode-2 Code                     (fixed, 2 B)
    // ---------------------------------------------------------------
    check_item(24, asterix::item_type_fixed, 2, 93u);

    // ---------------------------------------------------------------
    // FRN 25 – I048/065 Mode-1 Code Confidence          (fixed, 1 B)
    // ---------------------------------------------------------------
    check_item(25, asterix::item_type_fixed, 1, 95u);

    // ---------------------------------------------------------------
    // FRN 26 – I048/060 Mode-2 Code Confidence          (fixed, 2 B)
    // ---------------------------------------------------------------
    check_item(26, asterix::item_type_fixed, 2, 96u);

    // ---------------------------------------------------------------
    // FRN 27 – SP Special Purpose Field                 (explicit, 6 B total)
    //          Length byte = 0x06, payload = 5 B (FSPEC + 4 B dummy)
    // ---------------------------------------------------------------
    check_item(27, asterix::item_type_explicit, 6, 98u);

    // ---------------------------------------------------------------
    // FRN 28 – RE Reserved Expansion Field              (explicit, 135 B)
    //          Contains all 8 REF sub-items
    // ---------------------------------------------------------------
    {
        const auto* re = rec->get_item(28);
        ASSERT_NE(re, nullptr);
        EXPECT_TRUE(re->is_populated());
        EXPECT_EQ(re->type,       asterix::item_type_explicit);
        EXPECT_EQ(re->raw_length, 134u);   // includes the length byte
        EXPECT_EQ(re->raw_offset, 104u);

        // ── REF child FRN 1: MD5 (compound, 18 B: 1 FSPEC + 17 data) ──
        {
            const auto* md5 = re->get_child_item(1);
            ASSERT_NE(md5, nullptr);
            EXPECT_TRUE(md5->is_populated());
            EXPECT_EQ(md5->type,       asterix::item_type_compound);
            EXPECT_EQ(md5->raw_length, 18u);
            EXPECT_EQ(md5->raw_offset, 106u);

            auto check_md5_child = [&](uint8_t frn, uint16_t len, uint32_t off)
            {
                const auto* ch = md5->get_child_item(frn);
                ASSERT_NE(ch, nullptr)      << "MD5 child FRN " << (int)frn;
                EXPECT_TRUE(ch->is_populated())  << "MD5 child FRN " << (int)frn;
                EXPECT_EQ(ch->type, asterix::item_type_fixed) << "MD5 child FRN " << (int)frn;
                EXPECT_EQ(ch->raw_length, len) << "MD5 child FRN " << (int)frn;
                EXPECT_EQ(ch->raw_offset, off) << "MD5 child FRN " << (int)frn;
            };
            check_md5_child(1, 1u, 107u);   // Mode 5 Summary
            check_md5_child(2, 4u, 108u);   // PIN / Nat Origin / Mission Code
            check_md5_child(3, 6u, 112u);   // Reported Position
            check_md5_child(4, 2u, 118u);   // GNSS-derived Altitude
            check_md5_child(5, 2u, 120u);   // Extended Mode 1 Code
            check_md5_child(6, 1u, 122u);   // Time Offset
            check_md5_child(7, 1u, 123u);   // X Pulse Presence
        }

        // ── REF child FRN 2: M5N (compound, 20 B: 2 FSPEC + 18 data) ──
        {
            const auto* m5n = re->get_child_item(2);
            ASSERT_NE(m5n, nullptr);
            EXPECT_TRUE(m5n->is_populated());
            EXPECT_EQ(m5n->type,       asterix::item_type_compound);
            EXPECT_EQ(m5n->raw_length, 20u);
            EXPECT_EQ(m5n->raw_offset, 124u);

            auto check_m5n_child = [&](uint8_t frn, uint16_t len, uint32_t off)
            {
                const auto* ch = m5n->get_child_item(frn);
                ASSERT_NE(ch, nullptr)      << "M5N child FRN " << (int)frn;
                EXPECT_TRUE(ch->is_populated())  << "M5N child FRN " << (int)frn;
                EXPECT_EQ(ch->type, asterix::item_type_fixed) << "M5N child FRN " << (int)frn;
                EXPECT_EQ(ch->raw_length, len) << "M5N child FRN " << (int)frn;
                EXPECT_EQ(ch->raw_offset, off) << "M5N child FRN " << (int)frn;
            };
            check_m5n_child(1, 1u, 126u);   // Mode 5 Summary
            check_m5n_child(2, 4u, 127u);   // PIN / Nat Origin
            check_m5n_child(3, 6u, 131u);   // Reported Position
            check_m5n_child(4, 2u, 137u);   // GNSS-derived Altitude
            check_m5n_child(5, 2u, 139u);   // Extended Mode 1 Code
            check_m5n_child(6, 1u, 141u);   // Time Offset
            check_m5n_child(7, 1u, 142u);   // X Pulse Presence
            check_m5n_child(8, 1u, 143u);   // Figure of Merit
        }

        // ── REF child FRN 3: M4E (variable, 2 B: FX=1 + FX=0) ──
        {
            const auto* m4e = re->get_child_item(3);
            ASSERT_NE(m4e, nullptr);
            EXPECT_TRUE(m4e->is_populated());
            EXPECT_EQ(m4e->type,       asterix::item_type_variable);
            EXPECT_EQ(m4e->raw_length, 2u);
            EXPECT_EQ(m4e->raw_offset, 144u);
        }

        // ── REF child FRN 4: RPC (compound, 8 B: 1 FSPEC + 1+2+2+2 data) ──
        {
            const auto* rpc = re->get_child_item(4);
            ASSERT_NE(rpc, nullptr);
            EXPECT_TRUE(rpc->is_populated());
            EXPECT_EQ(rpc->type,       asterix::item_type_compound);
            EXPECT_EQ(rpc->raw_length, 8u);
            EXPECT_EQ(rpc->raw_offset, 146u);

            // Score (1 B)
            const auto* score = rpc->get_child_item(1);
            ASSERT_NE(score, nullptr);
            EXPECT_TRUE(score->is_populated());
            EXPECT_EQ(score->raw_length, 1u);
            EXPECT_EQ(score->raw_offset, 147u);

            // Signal/Clutter Ratio (2 B)
            const auto* scr = rpc->get_child_item(2);
            ASSERT_NE(scr, nullptr);
            EXPECT_TRUE(scr->is_populated());
            EXPECT_EQ(scr->raw_length, 2u);
            EXPECT_EQ(scr->raw_offset, 148u);

            // Range Width (2 B)
            const auto* rw = rpc->get_child_item(3);
            ASSERT_NE(rw, nullptr);
            EXPECT_TRUE(rw->is_populated());
            EXPECT_EQ(rw->raw_length, 2u);
            EXPECT_EQ(rw->raw_offset, 150u);

            // Ambiguous Range (2 B)
            const auto* ar = rpc->get_child_item(4);
            ASSERT_NE(ar, nullptr);
            EXPECT_TRUE(ar->is_populated());
            EXPECT_EQ(ar->raw_length, 2u);
            EXPECT_EQ(ar->raw_offset, 152u);
        }

        // ── REF child FRN 5: ERR (fixed, 3 B) ──
        {
            const auto* err = re->get_child_item(5);
            ASSERT_NE(err, nullptr);
            EXPECT_TRUE(err->is_populated());
            EXPECT_EQ(err->type,       asterix::item_type_fixed);
            EXPECT_EQ(err->raw_length, 3u);
            EXPECT_EQ(err->raw_offset, 154u);
        }

        // ── REF child FRN 6: RTC (compound, 56 B: 2 FSPEC + 54 data) ──
        //    FSPEC = 0xFF 0xF0 → items 1-8 from byte-1, items 9-11 from byte-2
        {
            const auto* rtc = re->get_child_item(6);
            ASSERT_NE(rtc, nullptr);
            EXPECT_TRUE(rtc->is_populated());
            EXPECT_EQ(rtc->type,       asterix::item_type_compound);
            EXPECT_EQ(rtc->raw_length, 56u);
            EXPECT_EQ(rtc->raw_offset, 157u);

            // RTC child 1: Plot/Track Link (fixed, 3 B)
            const auto* ptl = rtc->get_child_item(1);
            ASSERT_NE(ptl, nullptr);
            EXPECT_EQ(ptl->raw_length, 3u);
            EXPECT_EQ(ptl->raw_offset, 159u);

            // RTC child 2: ADS-B/Track Link (repetetive, 1 rep × 2 B = 3 B)
            const auto* adsb = rtc->get_child_item(2);
            ASSERT_NE(adsb, nullptr);
            EXPECT_EQ(adsb->type,       asterix::item_type_repetetive);
            EXPECT_EQ(adsb->raw_length, 3u);
            EXPECT_EQ(adsb->raw_offset, 162u);

            // RTC child 3: Turn State (fixed, 1 B)
            const auto* ts = rtc->get_child_item(3);
            ASSERT_NE(ts, nullptr);
            EXPECT_EQ(ts->raw_length, 1u);
            EXPECT_EQ(ts->raw_offset, 165u);

            // RTC child 4: Next Predicted Position (fixed, 22 B)
            const auto* npp = rtc->get_child_item(4);
            ASSERT_NE(npp, nullptr);
            EXPECT_EQ(npp->raw_length, 22u);
            EXPECT_EQ(npp->raw_offset, 166u);

            // RTC child 5: Data Link Characteristics (repetetive, 1 rep × 1 B = 2 B)
            const auto* dlc = rtc->get_child_item(5);
            ASSERT_NE(dlc, nullptr);
            EXPECT_EQ(dlc->type,       asterix::item_type_repetetive);
            EXPECT_EQ(dlc->raw_length, 2u);
            EXPECT_EQ(dlc->raw_offset, 188u);

            // RTC child 6: Lockout Characteristics (fixed, 2 B)
            const auto* lc = rtc->get_child_item(6);
            ASSERT_NE(lc, nullptr);
            EXPECT_EQ(lc->raw_length, 2u);
            EXPECT_EQ(lc->raw_offset, 190u);

            // RTC child 7: Transition Codes (fixed, 6 B)
            const auto* tc = rtc->get_child_item(7);
            ASSERT_NE(tc, nullptr);
            EXPECT_EQ(tc->raw_length, 6u);
            EXPECT_EQ(tc->raw_offset, 192u);

            // RTC child 8: Track Lifecycle (fixed, 4 B)
            const auto* tl = rtc->get_child_item(8);
            ASSERT_NE(tl, nullptr);
            EXPECT_EQ(tl->raw_length, 4u);
            EXPECT_EQ(tl->raw_offset, 198u);

            // RTC child 9: Adjacent Sensor Information (repetetive, 1 rep × 8 B = 9 B)
            const auto* asi = rtc->get_child_item(9);
            ASSERT_NE(asi, nullptr);
            EXPECT_EQ(asi->type,       asterix::item_type_repetetive);
            EXPECT_EQ(asi->raw_length, 9u);
            EXPECT_EQ(asi->raw_offset, 202u);

            // RTC child 10: Track Extrapolation Source (fixed, 1 B)
            const auto* tes = rtc->get_child_item(10);
            ASSERT_NE(tes, nullptr);
            EXPECT_EQ(tes->raw_length, 1u);
            EXPECT_EQ(tes->raw_offset, 211u);

            // RTC child 11: Identity Requested (fixed, 1 B)
            const auto* ir = rtc->get_child_item(11);
            ASSERT_NE(ir, nullptr);
            EXPECT_EQ(ir->raw_length, 1u);
            EXPECT_EQ(ir->raw_offset, 212u);
        }

        // ── REF child FRN 7: CPC (compound, 12 B: 1 FSPEC + 11 data) ──
        //    FSPEC = 0xF0 → items 1-4 present
        {
            const auto* cpc = re->get_child_item(7);
            ASSERT_NE(cpc, nullptr);
            EXPECT_TRUE(cpc->is_populated());
            EXPECT_EQ(cpc->type,       asterix::item_type_compound);
            EXPECT_EQ(cpc->raw_length, 12u);
            EXPECT_EQ(cpc->raw_offset, 213u);

            // CPC child 1: Plot Number (fixed, 2 B)
            const auto* pn = cpc->get_child_item(1);
            ASSERT_NE(pn, nullptr);
            EXPECT_EQ(pn->raw_length, 2u);
            EXPECT_EQ(pn->raw_offset, 214u);

            // CPC child 2: Replies/Plot Link (repetetive, 1 rep × 3 B = 4 B)
            const auto* rpl = cpc->get_child_item(2);
            ASSERT_NE(rpl, nullptr);
            EXPECT_EQ(rpl->type,       asterix::item_type_repetetive);
            EXPECT_EQ(rpl->raw_length, 4u);
            EXPECT_EQ(rpl->raw_offset, 216u);

            // CPC child 3: Scan Number (fixed, 1 B)
            const auto* sn = cpc->get_child_item(3);
            ASSERT_NE(sn, nullptr);
            EXPECT_EQ(sn->raw_length, 1u);
            EXPECT_EQ(sn->raw_offset, 220u);

            // CPC child 4: Date (fixed, 4 B)
            const auto* dt = cpc->get_child_item(4);
            ASSERT_NE(dt, nullptr);
            EXPECT_EQ(dt->raw_length, 4u);
            EXPECT_EQ(dt->raw_offset, 221u);
        }

        // ── REF child FRN 8: GEN48 (compound, 13 B: 1 FSPEC + 12 data) ──
        //    FSPEC = 0xF8 → items 1-5 present
        {
            const auto* gen48 = re->get_child_item(8);
            ASSERT_NE(gen48, nullptr);
            EXPECT_TRUE(gen48->is_populated());
            EXPECT_EQ(gen48->type,       asterix::item_type_compound);
            EXPECT_EQ(gen48->raw_length, 13u);
            EXPECT_EQ(gen48->raw_offset, 225u);

            // GEN48 child 1: Alternative Mode 2 Code (fixed, 2 B)
            const auto* am2 = gen48->get_child_item(1);
            ASSERT_NE(am2, nullptr);
            EXPECT_EQ(am2->raw_length, 2u);
            EXPECT_EQ(am2->raw_offset, 226u);

            // GEN48 child 2: Alternative Mode 3/A (fixed, 2 B)
            const auto* am3a = gen48->get_child_item(2);
            ASSERT_NE(am3a, nullptr);
            EXPECT_EQ(am3a->raw_length, 2u);
            EXPECT_EQ(am3a->raw_offset, 228u);

            // GEN48 child 3: Alternative Flight Level (fixed, 2 B)
            const auto* afl = gen48->get_child_item(3);
            ASSERT_NE(afl, nullptr);
            EXPECT_EQ(afl->raw_length, 2u);
            EXPECT_EQ(afl->raw_offset, 230u);

            // GEN48 child 4: Radar Cross Section dBm² (fixed, 2 B)
            const auto* rcsdbm = gen48->get_child_item(4);
            ASSERT_NE(rcsdbm, nullptr);
            EXPECT_EQ(rcsdbm->raw_length, 2u);
            EXPECT_EQ(rcsdbm->raw_offset, 232u);

            // GEN48 child 5: Radar Cross Section m² (fixed, 4 B)
            const auto* rcsm2 = gen48->get_child_item(5);
            ASSERT_NE(rcsm2, nullptr);
            EXPECT_EQ(rcsm2->raw_length, 4u);
            EXPECT_EQ(rcsm2->raw_offset, 234u);
        }
    }

    adam::buffer_manager::get().return_buffer(internal);
    adam::buffer_manager::get().return_buffer(data);
}

TEST_F(parser_test, benchmark_cat048_parsing)
{
    std::vector<uint8_t> raw = build_cat048_message();
    
    adam::buffer* raw_buf = adam::buffer_manager::get().request_buffer(static_cast<uint32_t>(raw.size()));
    std::memcpy(raw_buf->begin_as<uint8_t>(), raw.data(), raw.size());
    raw_buf->set_size(static_cast<uint32_t>(raw.size()));

    constexpr int ITERATIONS = 100000;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < ITERATIONS; ++i)
    {
        adam::buffer* internal_data = nullptr;
        bool success = parser.parse(raw_buf, internal_data);
        if (success && internal_data)
        {
            adam::buffer_manager::get().return_buffer(internal_data);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    std::cout << "[          ] Parsed CAT048 message " << ITERATIONS << " times" << std::endl;
    std::cout << "[          ] Total duration: " << duration / 1000000.0 << " s" << std::endl;
    std::cout << "[          ] Average time per parse: " << static_cast<double>(duration) / ITERATIONS << " us" << std::endl;

    adam::buffer_manager::get().return_buffer(raw_buf);
}

// ============================================================================
// Error-Handling Tests
//
// Every test below feeds intentionally malformed raw data to the parser and
// verifies that parse() returns false (and does NOT leak the internal buffer).
// ============================================================================

class parser_error_test : public ::testing::Test
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

    /// Helper: allocate a raw buffer, fill it with the supplied bytes, parse
    /// and expect failure.  The caller must NOT return raw_buf on failure –
    /// buffer_manager::destroy() handles clean-up in TearDown.
    void expect_parse_failure(std::initializer_list<uint8_t> bytes)
    {
        const auto sz = static_cast<uint32_t>(bytes.size());
        adam::buffer* raw_buf = adam::buffer_manager::get().request_buffer(sz ? sz : 1u);
        ASSERT_NE(raw_buf, nullptr);

        uint8_t* d = raw_buf->begin_as<uint8_t>();
        uint32_t i = 0;
        for (uint8_t b : bytes)
            d[i++] = b;
        raw_buf->set_size(sz);

        adam::buffer* internal = nullptr;
        EXPECT_FALSE(parser.parse(raw_buf, internal))
            << "Parser should have rejected the malformed input";

        // internal_data must not be left dangling on error
        EXPECT_EQ(internal, nullptr)
            << "Parser must not return a non-null internal_data on failure";

        adam::buffer_manager::get().return_buffer(raw_buf);
    }
};

// ---------------------------------------------------------------------------
// parse_null_buffer
//   The parser must reject a null raw buffer immediately.
// ---------------------------------------------------------------------------
TEST_F(parser_error_test, parse_null_buffer)
{
    adam::buffer* internal = nullptr;
    EXPECT_FALSE(parser.parse(nullptr, internal));
    EXPECT_EQ(internal, nullptr);
}

// ---------------------------------------------------------------------------
// parse_empty_buffer
//   A zero-byte buffer has no block header at all.
// ---------------------------------------------------------------------------
TEST_F(parser_error_test, parse_empty_buffer)
{
    adam::buffer* raw_buf = adam::buffer_manager::get().request_buffer(1);
    ASSERT_NE(raw_buf, nullptr);
    raw_buf->set_size(0);

    adam::buffer* internal = nullptr;
    // An empty buffer contains no valid block, so the parser should succeed
    // but return a frame with zero blocks (or it may return false – both
    // behaviours are acceptable; what is NOT acceptable is a crash).
    bool result = parser.parse(raw_buf, internal);
    (void)result;   // either outcome is fine for an empty buffer

    if (internal)
        adam::buffer_manager::get().return_buffer(internal);
    adam::buffer_manager::get().return_buffer(raw_buf);
}

// ---------------------------------------------------------------------------
// parse_truncated_block_header
//   Only 2 bytes are present – not enough for the 3-byte block header
//   (cat + length_msb + length_lsb).
// ---------------------------------------------------------------------------
TEST_F(parser_error_test, parse_truncated_block_header)
{
    expect_parse_failure({
        48,    // CAT
        0x00   // Length MSB only – LSB missing
    });
}

// ---------------------------------------------------------------------------
// parse_block_length_too_small
//   The length field reports a value smaller than min_block_length (3).
//   This is structurally impossible and must be rejected.
// ---------------------------------------------------------------------------
TEST_F(parser_error_test, parse_block_length_too_small)
{
    expect_parse_failure({
        48,    // CAT
        0x00,  // Length MSB
        0x02   // Length = 2 < min_block_length (3) → invalid
    });
}

// ---------------------------------------------------------------------------
// parse_block_length_exceeds_buffer
//   The length field claims more bytes than are actually present in the buffer.
// ---------------------------------------------------------------------------
TEST_F(parser_error_test, parse_block_length_exceeds_buffer)
{
    expect_parse_failure({
        48,    // CAT
        0x00,  // Length MSB
        0xFF,  // Length = 255 – but only 3 bytes of data follow
        0xC0,  // Fake FSPEC
    });
}

// ---------------------------------------------------------------------------
// parse_unknown_category
//   Category 255 has no registered UAP.  The parser must return false instead
//   of dereferencing a null UAP pointer.
// ---------------------------------------------------------------------------
TEST_F(parser_error_test, parse_unknown_category)
{
    // Valid block structure but with a category that has no UAP (255).
    // Length = 4 (header 3 + 1 byte FSPEC).
    expect_parse_failure({
        255,   // CAT – no UAP registered
        0x00,  // Length MSB
        0x04,  // Length LSB = 4
        0x80   // FSPEC: FRN 1 set, FX=0
    });
}

// ---------------------------------------------------------------------------
// parse_fspec_beyond_buffer_end
//   The FSPEC extension bit (FX=1) is set in the very last byte of the block
//   body, forcing the parser to read a byte that does not exist.
// ---------------------------------------------------------------------------
TEST_F(parser_error_test, parse_fspec_beyond_buffer_end)
{
    // Block body is exactly 1 byte (one FSPEC byte) that has FX=1,
    // demanding a second FSPEC byte that isn't there.
    expect_parse_failure({
        48,    // CAT
        0x00,  // Length MSB
        0x04,  // Length LSB = 4  (3 hdr + 1 FSPEC)
        0x01   // FSPEC: no items set, FX=1 → expects another FSPEC byte
    });
}

// ---------------------------------------------------------------------------
// parse_fixed_item_truncated
//   FSPEC signals FRN 1 (I048/010 DSID – 2 bytes fixed), but only 1 byte of
//   payload is available.
// ---------------------------------------------------------------------------
TEST_F(parser_error_test, parse_fixed_item_truncated)
{
    // Total = 3 (hdr) + 1 (FSPEC) + 1 (only 1 of the required 2 DSID bytes)
    expect_parse_failure({
        48,    // CAT
        0x00,  // Length MSB
        0x05,  // Length = 5
        0x80,  // FSPEC: FRN 1 (DSID 2B) set, FX=0
        0xAA   // Only 1 byte of the 2-byte DSID
    });
}

// ---------------------------------------------------------------------------
// parse_variable_item_first_chunk_truncated
//   FSPEC signals FRN 2 (I048/140 TOD – fixed 3 B).
//   Wait – let's use a variable item instead: FRN 3 (I048/020 TRD, variable
//   1+1 B first+extent), supplying no payload at all.
// ---------------------------------------------------------------------------
TEST_F(parser_error_test, parse_variable_item_truncated)
{
    // FSPEC byte: FRN 3 (bit 5 from MSB) set → 0b00100000 = 0x20, FX=0
    // No bytes follow for the variable item's first chunk.
    expect_parse_failure({
        48,    // CAT
        0x00,  // Length MSB
        0x04,  // Length = 4  (3 hdr + 1 FSPEC – no payload)
        0x20   // FSPEC: FRN 3 (I048/020) set, FX=0
        // Missing: 1+ bytes for the variable item
    });
}

// ---------------------------------------------------------------------------
// parse_variable_item_extent_truncated
//   The first chunk of a variable item has FX=1, requesting an extension byte
//   that is not present.
// ---------------------------------------------------------------------------
TEST_F(parser_error_test, parse_variable_item_extent_truncated)
{
    // Using CAT048 FRN 3 (TRD, variable: first_size=1, extent_size=1).
    // Supply 1 byte with FX=1 (bit 0 set) – the extension is missing.
    expect_parse_failure({
        48,    // CAT
        0x00,  // Length MSB
        0x05,  // Length = 5  (3 hdr + 1 FSPEC + 1 TRD byte)
        0x20,  // FSPEC: FRN 3 set, FX=0
        0x01   // TRD byte: FX=1 → extension requested but unavailable
    });
}

// ---------------------------------------------------------------------------
// parse_repetitive_item_rep_count_truncated
//   FSPEC signals a repetitive item but there are no bytes left for the
//   repetition-count byte.
// ---------------------------------------------------------------------------
TEST_F(parser_error_test, parse_repetitive_item_rep_count_truncated)
{
    // CAT048 FRN 10 (I048/250 BDS Register Data) is repetitive.
    // FSPEC bytes: byte-1 empty with FX=1 (0x01), byte-2 FRN10 set (0x20).
    // No payload → the 1-byte repetition-count field is missing.
    expect_parse_failure({
        48,    // CAT
        0x00,  // Length MSB
        0x05,  // Length = 5  (3 hdr + 2 FSPEC)
        0x01,  // FSPEC byte 1: no items, FX=1
        0x20   // FSPEC byte 2: FRN 10 set, FX=0
        // Missing: repetition-count byte and repetition data
    });
}

// ---------------------------------------------------------------------------
// parse_repetitive_item_data_truncated
//   The repetition-count byte is present (N=3) but only 1 of the 3 × 8-byte
//   repetitions is available.
// ---------------------------------------------------------------------------
TEST_F(parser_error_test, parse_repetitive_item_data_truncated)
{
    // 3 hdr + 2 FSPEC + 1 rep-count + 8 data bytes (only 1 full rep)
    //   We claim N=3 repetitions of 8 B each → need 24 B, supply only 8 B.
    expect_parse_failure({
        48,    // CAT
        0x00,  // Length MSB
        0x0C,  // Length = 12  (3 + 2 + 1 + 6 – intentionally short)
        0x01,  // FSPEC byte 1: FX=1
        0x20,  // FSPEC byte 2: FRN 10 set, FX=0
        0x03,  // rep-count = 3 (→ needs 24 B)
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66  // only 6 bytes of rep data
    });
}

// ---------------------------------------------------------------------------
// parse_compound_item_fspec_truncated
//   A compound item (FRN 7, I048/130) is indicated in the record FSPEC, but
//   there are no bytes left for the compound's own FSPEC.
// ---------------------------------------------------------------------------
TEST_F(parser_error_test, parse_compound_item_fspec_truncated)
{
    // CAT048 FRN 7 (I048/130 Radar Plot Characteristics) is a compound item.
    // FSPEC byte for the record: bit 1 (FRN 7) set → 0b00000010 = 0x02, FX=0
    // No bytes follow for the compound item's FSPEC.
    expect_parse_failure({
        48,    // CAT
        0x00,  // Length MSB
        0x04,  // Length = 4  (3 hdr + 1 FSPEC, zero compound payload)
        0x02   // FSPEC: FRN 7 (I048/130 compound) set, FX=0
        // Missing: compound FSPEC + child data
    });
}

// ---------------------------------------------------------------------------
// parse_explicit_item_length_byte_missing
//   An explicit item is signalled (FRN 27, SP field) but not even the
//   leading length byte is present.
// ---------------------------------------------------------------------------
TEST_F(parser_error_test, parse_explicit_item_length_byte_missing)
{
    // FSPEC: extend through bytes 1-3 with FX=1, then FRN 27 in byte 4.
    // No payload follows → the explicit-item length byte is missing.
    expect_parse_failure({
        48,    // CAT
        0x00,  // Length MSB
        0x07,  // Length = 7  (3 hdr + 4 FSPEC bytes, no explicit payload)
        0x01,  // FSPEC byte 1: FX=1
        0x01,  // FSPEC byte 2: FX=1
        0x01,  // FSPEC byte 3: FX=1
        0x04   // FSPEC byte 4: FRN 27 (bit 2) set, FX=0
        // Missing: explicit-item length byte
    });
}

// ---------------------------------------------------------------------------
// parse_explicit_item_payload_truncated
//   The explicit-item length byte claims 8 bytes of data (including itself),
//   but only 3 bytes are present in the buffer.
// ---------------------------------------------------------------------------
TEST_F(parser_error_test, parse_explicit_item_payload_truncated)
{
    // 3 hdr + 4 FSPEC + 1 length byte + 3 payload bytes = 11 total.
    // The length byte says 0x08 (8 bytes including itself) → 7 payload bytes
    // needed but only 3 are supplied.
    expect_parse_failure({
        48,    // CAT
        0x00,  // Length MSB
        0x0B,  // Length = 11
        0x01,  // FSPEC byte 1: FX=1
        0x01,  // FSPEC byte 2: FX=1
        0x01,  // FSPEC byte 3: FX=1
        0x04,  // FSPEC byte 4: FRN 27 set, FX=0
        0x08,  // explicit length = 8 (includes itself) → 7 payload bytes expected
        0xDE, 0xAD, 0xBE  // only 3 of the 7 payload bytes
    });
}