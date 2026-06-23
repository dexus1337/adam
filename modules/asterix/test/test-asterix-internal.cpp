/**
 * @file    test-asterix-internal.cpp
 * @author  dexus1337
 * @brief   Unit tests for the iterators added to record, block and frame in asterix-internal.hpp.
 *
 * All tests construct minimal, hand-crafted in-memory layouts that mirror the exact
 * #pragma pack(1) binary representation used by the parser.  No parser, no UAP, no
 * buffer_manager is involved – these are pure white-box structural tests.
 *
 * Confirmed sizes under #pragma pack(1):
 *   item   : child_count(2) + raw_length(2) + raw_offset(4) + child_offset(4)
 *            + type(1) + flags(1)  = 14 bytes
 *   record : used_uap(8) + item_count(2) + raw_length(2) + raw_offset(4)
 *            + category(1) + fspec_size(1) + flags(1)     = 19 bytes
 *   block  : record_count(2) + raw_length(2) + raw_offset(4) + item_count(2)
 *            + category(1) + flags(1)                      = 12 bytes
 *   frame  : block_count(2) + flags(1) + reserved(1)       =  4 bytes
 */

#include <gtest/gtest.h>
#include "data/asterix-internal.hpp"

#include <algorithm>
#include <array>
#include <cstring>
#include <iterator>
#include <vector>

using namespace adam::modules::asterix;

// ─────────────────────────────────────────────────────────────────────────────
// Compile-time size assertions – catch regressions immediately
// ─────────────────────────────────────────────────────────────────────────────
static_assert(sizeof(item)   == 14, "item size mismatch – update tests");
static_assert(sizeof(record) == 19, "record size mismatch – update tests");
static_assert(sizeof(block)  == 12, "block size mismatch – update tests");
static_assert(sizeof(frame)  ==  4, "frame size mismatch – update tests");

// ─────────────────────────────────────────────────────────────────────────────
// Helpers: build minimal in-memory structures
// ─────────────────────────────────────────────────────────────────────────────

/** Build an item with the given fields, zeroing everything else. */
static item make_item(uint16_t raw_len, uint32_t raw_off,
                      item_type type, bool populated)
{
    item it{};
    it.child_count  = 0;
    it.raw_length   = raw_len;
    it.raw_offset   = raw_off;
    it.child_offset = 0;
    it.type         = type;
    it.flags        = populated ? item_flag_populated : item_flag_none;
    return it;
}

// ═════════════════════════════════════════════════════════════════════════════
//  record::item_iterator tests
// ═════════════════════════════════════════════════════════════════════════════

class record_item_iterator_test : public ::testing::Test
{
protected:
    /**
     * Build a minimal in-memory blob that looks like:
     *   [record header (19 B)] [item_0 (14 B)] [item_1 (14 B)] ... [item_{N-1}]
     *
     * @param items   Items to embed after the record header.
     * @returns A byte vector whose first byte is the record.
     */
    std::vector<uint8_t> build_record_blob(std::vector<item> items)
    {
        std::vector<uint8_t> blob(sizeof(record) + items.size() * sizeof(item), 0);

        record* rec = reinterpret_cast<record*>(blob.data());
        rec->used_uap    = 0;
        rec->item_count  = static_cast<uint16_t>(items.size());
        rec->raw_length  = 0;
        rec->raw_offset  = 0;
        rec->category    = 48;
        rec->fspec_size  = 1;
        rec->flags       = record_flag_none;

        uint8_t* dst = blob.data() + sizeof(record);
        for (const auto& it : items)
        {
            std::memcpy(dst, &it, sizeof(item));
            dst += sizeof(item);
        }
        return blob;
    }
};

// ─── Trait checks ────────────────────────────────────────────────────────────

TEST_F(record_item_iterator_test, iterator_traits)
{
    using Itr = record::item_iterator;
    static_assert(std::is_same_v<Itr::iterator_category, std::forward_iterator_tag>);
    static_assert(std::is_same_v<Itr::value_type,        const item>);
    static_assert(std::is_same_v<Itr::pointer,           const item*>);
    static_assert(std::is_same_v<Itr::reference,         const item&>);
    SUCCEED(); // compile-time checks passed
}

// ─── Empty record ─────────────────────────────────────────────────────────────

TEST_F(record_item_iterator_test, empty_record_begin_equals_end)
{
    auto blob = build_record_blob({});
    const auto* rec = reinterpret_cast<const record*>(blob.data());
    EXPECT_EQ(rec->begin(), rec->end());
}

TEST_F(record_item_iterator_test, empty_record_range_for_is_never_entered)
{
    auto blob = build_record_blob({});
    const auto* rec = reinterpret_cast<const record*>(blob.data());
    int count = 0;
    for ([[maybe_unused]] const auto& it : *rec) ++count;
    EXPECT_EQ(count, 0);
}

// ─── Single item ──────────────────────────────────────────────────────────────

TEST_F(record_item_iterator_test, single_item_dereference)
{
    item it0 = make_item(4, 100, item_type_fixed, true);
    auto blob = build_record_blob({it0});
    const auto* rec = reinterpret_cast<const record*>(blob.data());

    auto itr = rec->begin();
    EXPECT_NE(itr, rec->end());
    EXPECT_EQ(itr->raw_length,  4u);
    EXPECT_EQ(itr->raw_offset,  100u);
    EXPECT_EQ(itr->type,        item_type_fixed);
    EXPECT_TRUE(itr->is_populated());
}

TEST_F(record_item_iterator_test, single_item_pre_increment_reaches_end)
{
    auto blob = build_record_blob({make_item(2, 0, item_type_fixed, true)});
    const auto* rec = reinterpret_cast<const record*>(blob.data());

    auto itr = rec->begin();
    ++itr;
    EXPECT_EQ(itr, rec->end());
}

TEST_F(record_item_iterator_test, single_item_post_increment_returns_old)
{
    item it0 = make_item(3, 10, item_type_variable, false);
    auto blob = build_record_blob({it0});
    const auto* rec = reinterpret_cast<const record*>(blob.data());

    auto itr   = rec->begin();
    auto old   = itr++;        // post-increment
    EXPECT_EQ(old->raw_length,  3u);
    EXPECT_EQ(old->raw_offset,  10u);
    EXPECT_FALSE(old->is_populated());
    EXPECT_EQ(itr, rec->end());
}

// ─── Multiple items ───────────────────────────────────────────────────────────

TEST_F(record_item_iterator_test, multiple_items_correct_sequence)
{
    item it0 = make_item(2,  4, item_type_fixed,      true);
    item it1 = make_item(0,  0, item_type_fixed,      false); // unpopulated gap
    item it2 = make_item(3,  6, item_type_variable,   true);

    auto blob = build_record_blob({it0, it1, it2});
    const auto* rec = reinterpret_cast<const record*>(blob.data());

    std::vector<const item*> visited;
    for (const auto& it : *rec)
        visited.push_back(&it);

    ASSERT_EQ(visited.size(), 3u);
    EXPECT_EQ(visited[0]->raw_length,  2u);
    EXPECT_TRUE(visited[0]->is_populated());

    EXPECT_EQ(visited[1]->raw_length,  0u);
    EXPECT_FALSE(visited[1]->is_populated());

    EXPECT_EQ(visited[2]->raw_length,  3u);
    EXPECT_TRUE(visited[2]->is_populated());
}

TEST_F(record_item_iterator_test, multiple_items_count_matches_item_count)
{
    auto blob = build_record_blob({
        make_item(1, 0, item_type_fixed, true),
        make_item(2, 1, item_type_fixed, false),
        make_item(3, 3, item_type_fixed, true),
        make_item(4, 6, item_type_fixed, false),
        make_item(5, 10, item_type_fixed, true),
    });
    const auto* rec = reinterpret_cast<const record*>(blob.data());

    int count = 0;
    for ([[maybe_unused]] const auto& _ : *rec) ++count;
    EXPECT_EQ(count, static_cast<int>(rec->item_count));
}

// ─── Consistency with get_item() ─────────────────────────────────────────────

TEST_F(record_item_iterator_test, iterator_matches_get_item)
{
    std::vector<item> items;
    for (uint8_t i = 0; i < 8; ++i)
        items.push_back(make_item(i, i * 10u, item_type_fixed, i % 2 == 0));

    auto blob = build_record_blob(items);
    const auto* rec = reinterpret_cast<const record*>(blob.data());

    uint8_t frn = 1;
    for (const auto& it : *rec)
    {
        const auto* via_get = rec->get_item(frn);
        ASSERT_NE(via_get, nullptr);
        EXPECT_EQ(&it, via_get) << "iterator and get_item disagree at FRN " << (int)frn;
        ++frn;
    }
}

// ─── Equality / inequality ────────────────────────────────────────────────────

TEST_F(record_item_iterator_test, equality_and_inequality)
{
    auto blob = build_record_blob({make_item(1, 0, item_type_fixed, true)});
    const auto* rec = reinterpret_cast<const record*>(blob.data());

    auto a = rec->begin();
    auto b = rec->begin();
    EXPECT_EQ(a, b);

    ++b;
    EXPECT_NE(a, b);
    EXPECT_EQ(b, rec->end());
}

// ─── Arrow operator ───────────────────────────────────────────────────────────

TEST_F(record_item_iterator_test, arrow_operator)
{
    item it0 = make_item(7, 42, item_type_repetetive, true);
    auto blob = build_record_blob({it0});
    const auto* rec = reinterpret_cast<const record*>(blob.data());

    auto itr = rec->begin();
    EXPECT_EQ(itr->raw_length,  7u);
    EXPECT_EQ(itr->raw_offset,  42u);
    EXPECT_EQ(itr->type, item_type_repetetive);
}


// ═════════════════════════════════════════════════════════════════════════════
//  block::record_iterator tests
// ═════════════════════════════════════════════════════════════════════════════

class block_record_iterator_test : public ::testing::Test
{
protected:
    struct record_desc { uint16_t item_count; uint8_t category; };

    /**
     * Build a minimal blob:  [block header] [record0 hdr][items…] [record1 hdr][items…] …
     *
     * Each record's item array is filled with zeroed (unpopulated) items.
     */
    std::vector<uint8_t> build_block_blob(std::vector<record_desc> descs)
    {
        // Total items across all records
        uint16_t total_items = 0;
        for (const auto& d : descs) total_items += d.item_count;

        size_t payload_size = 0;
        for (const auto& d : descs)
            payload_size += sizeof(record) + d.item_count * sizeof(item);

        std::vector<uint8_t> blob(sizeof(block) + payload_size, 0);

        block* blk = reinterpret_cast<block*>(blob.data());
        blk->record_count = static_cast<uint16_t>(descs.size());
        blk->raw_length   = 0;
        blk->raw_offset   = 0;
        blk->item_count   = total_items;
        blk->category     = 62;
        blk->flags        = block_flag_none;

        uint8_t* ptr = blob.data() + sizeof(block);
        for (uint16_t ri = 0; ri < static_cast<uint16_t>(descs.size()); ++ri)
        {
            record* rec = reinterpret_cast<record*>(ptr);
            rec->used_uap    = static_cast<uint64_t>(ri); // distinct per record
            rec->item_count  = descs[ri].item_count;
            rec->raw_length  = 0;
            rec->raw_offset  = 0;
            rec->category    = descs[ri].category;
            rec->fspec_size  = 1;
            rec->flags       = record_flag_none;

            ptr += sizeof(record) + descs[ri].item_count * sizeof(item);
        }
        return blob;
    }
};

// ─── Trait checks ────────────────────────────────────────────────────────────

TEST_F(block_record_iterator_test, iterator_traits)
{
    using Itr = block::record_iterator;
    static_assert(std::is_same_v<Itr::iterator_category, std::forward_iterator_tag>);
    static_assert(std::is_same_v<Itr::value_type,        const record>);
    static_assert(std::is_same_v<Itr::pointer,           const record*>);
    static_assert(std::is_same_v<Itr::reference,         const record&>);
    SUCCEED();
}

// ─── Empty block ──────────────────────────────────────────────────────────────

TEST_F(block_record_iterator_test, empty_block_begin_equals_end)
{
    auto blob = build_block_blob({});
    const auto* blk = reinterpret_cast<const block*>(blob.data());
    EXPECT_EQ(blk->begin(), blk->end());
}

TEST_F(block_record_iterator_test, empty_block_range_for_never_entered)
{
    auto blob = build_block_blob({});
    const auto* blk = reinterpret_cast<const block*>(blob.data());
    int count = 0;
    for ([[maybe_unused]] const auto& _ : *blk) ++count;
    EXPECT_EQ(count, 0);
}

// ─── Single record ────────────────────────────────────────────────────────────

TEST_F(block_record_iterator_test, single_record_dereference)
{
    auto blob = build_block_blob({{3, 48}});
    const auto* blk = reinterpret_cast<const block*>(blob.data());

    auto itr = blk->begin();
    EXPECT_NE(itr, blk->end());
    EXPECT_EQ(itr->item_count, 3u);
    EXPECT_EQ(itr->category,   48);
}

TEST_F(block_record_iterator_test, single_record_pre_increment_reaches_end)
{
    auto blob = build_block_blob({{5, 48}});
    const auto* blk = reinterpret_cast<const block*>(blob.data());

    auto itr = blk->begin();
    ++itr;
    EXPECT_EQ(itr, blk->end());
}

TEST_F(block_record_iterator_test, single_record_post_increment_returns_old)
{
    auto blob = build_block_blob({{2, 62}});
    const auto* blk = reinterpret_cast<const block*>(blob.data());

    auto itr = blk->begin();
    auto old = itr++;
    EXPECT_EQ(old->item_count, 2u);
    EXPECT_EQ(old->category,   62);
    EXPECT_EQ(itr, blk->end());
}

// ─── Multiple records with DIFFERENT item counts (variable stride) ─────────────

TEST_F(block_record_iterator_test, multiple_records_correct_sequence)
{
    // Records with intentionally different item_count values to stress stride
    auto blob = build_block_blob({{1, 48}, {5, 62}, {3, 48}});
    const auto* blk = reinterpret_cast<const block*>(blob.data());

    std::vector<std::pair<uint16_t,uint8_t>> visited;
    for (const auto& rec : *blk)
        visited.emplace_back(rec.item_count, rec.category);

    ASSERT_EQ(visited.size(), 3u);
    EXPECT_EQ(visited[0], std::make_pair(uint16_t(1), uint8_t(48)));
    EXPECT_EQ(visited[1], std::make_pair(uint16_t(5), uint8_t(62)));
    EXPECT_EQ(visited[2], std::make_pair(uint16_t(3), uint8_t(48)));
}

TEST_F(block_record_iterator_test, multiple_records_count_matches_record_count)
{
    auto blob = build_block_blob({{0, 48}, {2, 62}, {7, 1}, {1, 48}, {4, 62}});
    const auto* blk = reinterpret_cast<const block*>(blob.data());

    int count = 0;
    for ([[maybe_unused]] const auto& _ : *blk) ++count;
    EXPECT_EQ(count, static_cast<int>(blk->record_count));
}

// ─── Consistency with get_record() ───────────────────────────────────────────

TEST_F(block_record_iterator_test, iterator_matches_get_record)
{
    auto blob = build_block_blob({{1, 48}, {3, 62}, {0, 1}, {5, 48}});
    const auto* blk = reinterpret_cast<const block*>(blob.data());

    uint16_t idx = 0;
    for (const auto& rec : *blk)
    {
        const auto* via_get = blk->get_record(idx);
        ASSERT_NE(via_get, nullptr);
        EXPECT_EQ(&rec, via_get) << "iterator and get_record disagree at idx " << idx;
        ++idx;
    }
}

// ─── Unique-identity field (used_uap) survives stride ────────────────────────

TEST_F(block_record_iterator_test, records_have_distinct_identities)
{
    // build_block_blob stores ri as used_uap, so we can verify no aliasing
    auto blob = build_block_blob({{2, 48}, {4, 62}, {1, 1}});
    const auto* blk = reinterpret_cast<const block*>(blob.data());

    uint64_t expected_id = 0;
    for (const auto& rec : *blk)
    {
        EXPECT_EQ(rec.used_uap, expected_id)
            << "record at index " << expected_id << " has wrong used_uap";
        ++expected_id;
    }
}

// ─── Equality / inequality ────────────────────────────────────────────────────

TEST_F(block_record_iterator_test, equality_and_inequality)
{
    auto blob = build_block_blob({{2, 48}});
    const auto* blk = reinterpret_cast<const block*>(blob.data());

    auto a = blk->begin();
    auto b = blk->begin();
    EXPECT_EQ(a, b);

    ++b;
    EXPECT_NE(a, b);
    EXPECT_EQ(b, blk->end());
}

// ─── Arrow operator ───────────────────────────────────────────────────────────

TEST_F(block_record_iterator_test, arrow_operator)
{
    auto blob = build_block_blob({{6, 62}});
    const auto* blk = reinterpret_cast<const block*>(blob.data());

    auto itr = blk->begin();
    EXPECT_EQ(itr->item_count, 6u);
    EXPECT_EQ(itr->category,   62);
}

// ─── Nested iteration: records × items ───────────────────────────────────────

TEST_F(block_record_iterator_test, nested_iteration_item_count)
{
    // 3 records: 1st has 2 items, 2nd has 0 items, 3rd has 3 items → 5 total
    auto blob = build_block_blob({{2, 48}, {0, 62}, {3, 1}});
    const auto* blk = reinterpret_cast<const block*>(blob.data());

    int total_items_seen = 0;
    for (const auto& rec : *blk)
        for ([[maybe_unused]] const auto& _ : rec)
            ++total_items_seen;

    EXPECT_EQ(total_items_seen, 5);
}


// ═════════════════════════════════════════════════════════════════════════════
//  frame::block_iterator tests
// ═════════════════════════════════════════════════════════════════════════════

class frame_block_iterator_test : public ::testing::Test
{
protected:
    struct block_desc
    {
        uint8_t  category;
        std::vector<uint16_t> record_item_counts; // one entry per record
    };

    /**
     * Build a minimal blob:
     *   [frame header (4B)]
     *   [block0 hdr][rec0 hdr][items…] [rec1 hdr][items…] …
     *   [block1 hdr] …
     */
    std::vector<uint8_t> build_frame_blob(std::vector<block_desc> descs)
    {
        // Pre-calculate total size
        size_t total = sizeof(frame);
        for (const auto& bd : descs)
        {
            total += sizeof(block);
            for (uint16_t ic : bd.record_item_counts)
                total += sizeof(record) + ic * sizeof(item);
        }

        std::vector<uint8_t> blob(total, 0);

        frame* frm = reinterpret_cast<frame*>(blob.data());
        frm->block_count = static_cast<uint16_t>(descs.size());
        frm->flags       = frame_flag_none;
        frm->reserved    = 0;

        uint8_t* ptr = blob.data() + sizeof(frame);

        for (uint64_t bi = 0; bi < descs.size(); ++bi)
        {
            const auto& bd = descs[bi];

            uint16_t total_items = 0;
            for (uint16_t ic : bd.record_item_counts) total_items += ic;

            block* blk = reinterpret_cast<block*>(ptr);
            blk->record_count = static_cast<uint16_t>(bd.record_item_counts.size());
            blk->raw_length   = 0;
            blk->raw_offset   = 0;
            blk->item_count   = total_items;
            blk->category     = bd.category;
            blk->flags        = block_flag_none;

            // Reuse bi as a sentinel in raw_offset so we can identify blocks
            blk->raw_offset   = static_cast<uint32_t>(bi);

            ptr += sizeof(block);

            for (uint64_t ri = 0; ri < bd.record_item_counts.size(); ++ri)
            {
                uint16_t ic = bd.record_item_counts[ri];
                record* rec = reinterpret_cast<record*>(ptr);
                rec->used_uap   = static_cast<uint64_t>(ri);
                rec->item_count = ic;
                rec->raw_length = 0;
                rec->raw_offset = 0;
                rec->category   = bd.category;
                rec->fspec_size = 1;
                rec->flags      = record_flag_none;
                ptr += sizeof(record) + ic * sizeof(item);
            }
        }
        return blob;
    }
};

// ─── Trait checks ────────────────────────────────────────────────────────────

TEST_F(frame_block_iterator_test, iterator_traits)
{
    using Itr = frame::block_iterator;
    static_assert(std::is_same_v<Itr::iterator_category, std::forward_iterator_tag>);
    static_assert(std::is_same_v<Itr::value_type,        const block>);
    static_assert(std::is_same_v<Itr::pointer,           const block*>);
    static_assert(std::is_same_v<Itr::reference,         const block&>);
    SUCCEED();
}

// ─── Empty frame ─────────────────────────────────────────────────────────────

TEST_F(frame_block_iterator_test, empty_frame_begin_equals_end)
{
    auto blob = build_frame_blob({});
    const auto* frm = reinterpret_cast<const frame*>(blob.data());
    EXPECT_EQ(frm->begin(), frm->end());
}

TEST_F(frame_block_iterator_test, empty_frame_range_for_never_entered)
{
    auto blob = build_frame_blob({});
    const auto* frm = reinterpret_cast<const frame*>(blob.data());
    int count = 0;
    for ([[maybe_unused]] const auto& _ : *frm) ++count;
    EXPECT_EQ(count, 0);
}

// ─── Single block ─────────────────────────────────────────────────────────────

TEST_F(frame_block_iterator_test, single_block_dereference)
{
    auto blob = build_frame_blob({{48, {2, 3}}});
    const auto* frm = reinterpret_cast<const frame*>(blob.data());

    auto itr = frm->begin();
    EXPECT_NE(itr, frm->end());
    EXPECT_EQ(itr->category,     48);
    EXPECT_EQ(itr->record_count, 2u);
    EXPECT_EQ(itr->item_count,   5u); // 2+3
}

TEST_F(frame_block_iterator_test, single_block_pre_increment_reaches_end)
{
    auto blob = build_frame_blob({{62, {1}}});
    const auto* frm = reinterpret_cast<const frame*>(blob.data());

    auto itr = frm->begin();
    ++itr;
    EXPECT_EQ(itr, frm->end());
}

TEST_F(frame_block_iterator_test, single_block_post_increment_returns_old)
{
    auto blob = build_frame_blob({{1, {4, 0}}});
    const auto* frm = reinterpret_cast<const frame*>(blob.data());

    auto itr = frm->begin();
    auto old = itr++;
    EXPECT_EQ(old->category,   1);
    EXPECT_EQ(old->item_count, 4u);
    EXPECT_EQ(itr, frm->end());
}

// ─── Multiple blocks ──────────────────────────────────────────────────────────

TEST_F(frame_block_iterator_test, multiple_blocks_correct_sequence)
{
    // Intentionally varied record layouts to stress the stride arithmetic
    auto blob = build_frame_blob({
        {48, {1, 2}},    // block0: 2 records, 3 items total
        {62, {5}},       // block1: 1 record,  5 items
        { 1, {0, 3, 1}}, // block2: 3 records, 4 items
    });
    const auto* frm = reinterpret_cast<const frame*>(blob.data());

    std::vector<std::tuple<uint8_t,uint16_t,uint16_t>> visited;
    for (const auto& blk : *frm)
        visited.emplace_back(blk.category, blk.record_count, blk.item_count);

    ASSERT_EQ(visited.size(), 3u);
    EXPECT_EQ(visited[0], std::make_tuple(uint8_t(48), uint16_t(2), uint16_t(3)));
    EXPECT_EQ(visited[1], std::make_tuple(uint8_t(62), uint16_t(1), uint16_t(5)));
    EXPECT_EQ(visited[2], std::make_tuple(uint8_t(1),  uint16_t(3), uint16_t(4)));
}

TEST_F(frame_block_iterator_test, multiple_blocks_count_matches_block_count)
{
    auto blob = build_frame_blob({
        {48, {0}}, {62, {1, 2}}, {1, {}}, {48, {3}}, {62, {0, 0, 1}}
    });
    const auto* frm = reinterpret_cast<const frame*>(blob.data());

    int count = 0;
    for ([[maybe_unused]] const auto& _ : *frm) ++count;
    EXPECT_EQ(count, static_cast<int>(frm->block_count));
}

// ─── Consistency with get_block() ────────────────────────────────────────────

TEST_F(frame_block_iterator_test, iterator_matches_get_block)
{
    auto blob = build_frame_blob({
        {48, {2}}, {62, {1, 3}}, {1, {0, 2}}
    });
    const auto* frm = reinterpret_cast<const frame*>(blob.data());

    uint16_t idx = 0;
    for (const auto& blk : *frm)
    {
        const auto* via_get = frm->get_block(idx);
        ASSERT_NE(via_get, nullptr);
        EXPECT_EQ(&blk, via_get)
            << "iterator and get_block disagree at idx " << idx;
        ++idx;
    }
}

// ─── raw_offset sentinel survives stride ─────────────────────────────────────

TEST_F(frame_block_iterator_test, blocks_have_distinct_identities)
{
    // build_frame_blob stores block index as raw_offset
    auto blob = build_frame_blob({
        {48, {1}}, {62, {2, 3}}, {1, {0}}
    });
    const auto* frm = reinterpret_cast<const frame*>(blob.data());

    uint32_t expected_id = 0;
    for (const auto& blk : *frm)
    {
        EXPECT_EQ(blk.raw_offset, expected_id)
            << "block at index " << expected_id << " has wrong raw_offset sentinel";
        ++expected_id;
    }
}

// ─── Equality / inequality ────────────────────────────────────────────────────

TEST_F(frame_block_iterator_test, equality_and_inequality)
{
    auto blob = build_frame_blob({{48, {1}}});
    const auto* frm = reinterpret_cast<const frame*>(blob.data());

    auto a = frm->begin();
    auto b = frm->begin();
    EXPECT_EQ(a, b);

    ++b;
    EXPECT_NE(a, b);
    EXPECT_EQ(b, frm->end());
}

// ─── Arrow operator ───────────────────────────────────────────────────────────

TEST_F(frame_block_iterator_test, arrow_operator)
{
    auto blob = build_frame_blob({{62, {3, 2, 1}}});
    const auto* frm = reinterpret_cast<const frame*>(blob.data());

    auto itr = frm->begin();
    EXPECT_EQ(itr->category,     62);
    EXPECT_EQ(itr->record_count, 3u);
    EXPECT_EQ(itr->item_count,   6u); // 3+2+1
}

// ─── Full nested traversal: frame → blocks → records → items ─────────────────

TEST_F(frame_block_iterator_test, full_nested_traversal)
{
    // Layout:
    //   block0 (cat 48): record0 (2 items), record1 (1 item)
    //   block1 (cat 62): record0 (3 items)
    auto blob = build_frame_blob({
        {48, {2, 1}},
        {62, {3}},
    });
    const auto* frm = reinterpret_cast<const frame*>(blob.data());

    int total_items = 0;
    int total_records = 0;
    int total_blocks  = 0;

    for (const auto& blk : *frm)
    {
        ++total_blocks;
        for (const auto& rec : blk)
        {
            ++total_records;
            for ([[maybe_unused]] const auto& _ : rec)
                ++total_items;
        }
    }

    EXPECT_EQ(total_blocks,  2);
    EXPECT_EQ(total_records, 3);   // 2 in block0 + 1 in block1
    EXPECT_EQ(total_items,   6);   // (2+1) + 3
}


// ═════════════════════════════════════════════════════════════════════════════
//  raw_fspec::iterator tests
// ═════════════════════════════════════════════════════════════════════════════
//
// raw_fspec layout (1 byte):
//   bits 7-1  seven data bits (FRN slots for this octet)
//   bit  0    FX (Field Extension): 1 = next byte is part of this FSPEC
//
// is_frn_active_here(frn) uses bit position = 7 - ((frn-1) % 7)
//   FRN 1 → bit 7 (0x80)
//   FRN 2 → bit 6 (0x40)
//   ...
//   FRN 7 → bit 1 (0x02)
//   FX    → bit 0 (0x01)  ← NOT an FRN

#include "data/asterix-types.hpp"

class fspec_iterator_test : public ::testing::Test
{
protected:
    // Build a contiguous array of raw_fspec bytes in a std::vector.
    // The last element always has FX=0 (no extension); all preceding elements
    // have FX=1 (bit 0 set).
    std::vector<raw_fspec> make_fspec(std::initializer_list<uint8_t> values)
    {
        std::vector<raw_fspec> chain;
        chain.reserve(values.size());
        for (uint8_t v : values)
            chain.push_back(raw_fspec{v});
        return chain;
    }
};

// ─── Trait checks ────────────────────────────────────────────────────────────

TEST_F(fspec_iterator_test, iterator_traits)
{
    using Itr = raw_fspec::iterator;
    static_assert(std::is_same_v<Itr::iterator_category, std::forward_iterator_tag>);
    static_assert(std::is_same_v<Itr::value_type,        const raw_fspec>);
    static_assert(std::is_same_v<Itr::pointer,           const raw_fspec*>);
    static_assert(std::is_same_v<Itr::reference,         const raw_fspec&>);
    SUCCEED();
}

// ─── Private pointer – cannot access internals directly ──────────────────────

TEST_F(fspec_iterator_test, internal_pointer_is_private)
{
    // This test is a compile-time assertion: the test file must NOT be able to
    // write `it.current` or `it.m_ptr`. Because we can't `static_assert` the
    // absence of a member easily, we simply verify the iterator works correctly
    // through its public API only.  If anyone re-adds a public `current` member,
    // the design intent is violated (and should be caught in code review).
    auto chain = make_fspec({0xC0}); // FRN1+FRN2, no FX
    auto it = chain[0].begin();
    // Accessing via public API only – no it.current, no it.m_ptr
    EXPECT_EQ((*it).value, 0xC0);
    SUCCEED();
}

// ─── Single-byte FSPEC (FX = 0, no extension) ────────────────────────────────

TEST_F(fspec_iterator_test, single_byte_begin_not_equal_end)
{
    auto chain = make_fspec({0xC0}); // FX=0
    EXPECT_NE(chain[0].begin(), chain[0].end());
}

TEST_F(fspec_iterator_test, single_byte_dereference_value)
{
    auto chain = make_fspec({0xA0}); // 1010 0000  →  FRN1 set, FRN3 set, FX=0
    auto it = chain[0].begin();
    EXPECT_EQ((*it).value, 0xA0u);
    EXPECT_EQ(it->value,   0xA0u);
}

TEST_F(fspec_iterator_test, single_byte_pre_increment_reaches_end)
{
    auto chain = make_fspec({0x80}); // FRN1 only, FX=0
    auto it = chain[0].begin();
    ++it;
    EXPECT_EQ(it, chain[0].end());
}

TEST_F(fspec_iterator_test, single_byte_post_increment_returns_old)
{
    auto chain = make_fspec({0x40}); // FRN2 only, FX=0
    auto it  = chain[0].begin();
    auto old = it++;
    EXPECT_EQ(old->value, 0x40u);
    EXPECT_EQ(it, chain[0].end());
}

TEST_F(fspec_iterator_test, single_byte_range_for_visits_once)
{
    auto chain = make_fspec({0xFE}); // all 7 FRN bits set, FX=0
    int count = 0;
    for (const auto& octet : chain[0]) { (void)octet; ++count; }
    EXPECT_EQ(count, 1);
}

// ─── Multi-byte FSPEC chain ───────────────────────────────────────────────────

TEST_F(fspec_iterator_test, two_byte_chain_visits_both_octets)
{
    // byte0: FX=1 (bit 0 set), so next byte is part of chain
    // byte1: FX=0
    auto chain = make_fspec({0x81, 0x40}); // [FRN1+FX=1] [FRN2+FX=0]
    int count = 0;
    for ([[maybe_unused]] const auto& _ : chain[0]) ++count;
    EXPECT_EQ(count, 2);
}

TEST_F(fspec_iterator_test, two_byte_chain_correct_values)
{
    auto chain = make_fspec({0xC1, 0xA0}); // byte0=0xC1, byte1=0xA0
    auto it = chain[0].begin();

    ASSERT_NE(it, chain[0].end());
    EXPECT_EQ(it->value, 0xC1u);
    EXPECT_TRUE(it->has_extension());
    ++it;

    ASSERT_NE(it, chain[0].end());
    EXPECT_EQ(it->value, 0xA0u);
    EXPECT_FALSE(it->has_extension());
    ++it;

    EXPECT_EQ(it, chain[0].end());
}

TEST_F(fspec_iterator_test, three_byte_chain_visits_all_octets)
{
    auto chain = make_fspec({0x01, 0x01, 0x80}); // FX=1, FX=1, FX=0
    int count = 0;
    for ([[maybe_unused]] const auto& _ : chain[0]) ++count;
    EXPECT_EQ(count, 3);
}

TEST_F(fspec_iterator_test, chain_count_matches_size_helper)
{
    auto chain = make_fspec({0x01, 0x01, 0x01, 0x80}); // 4 bytes
    int iter_count = 0;
    for ([[maybe_unused]] const auto& _ : chain[0]) ++iter_count;
    EXPECT_EQ(iter_count, static_cast<int>(chain[0].size()));
}

// ─── byte_index() ────────────────────────────────────────────────────────────

TEST_F(fspec_iterator_test, byte_index_starts_at_zero)
{
    auto chain = make_fspec({0x01, 0x80}); // 2-byte chain
    auto it = chain[0].begin();
    EXPECT_EQ(it.byte_index(), 0u);
}

TEST_F(fspec_iterator_test, byte_index_increments_per_octet)
{
    auto chain = make_fspec({0x01, 0x01, 0x01, 0x80}); // 4 bytes
    uint16_t expected = 0;
    for (auto it = chain[0].begin(); it != chain[0].end(); ++it)
    {
        EXPECT_EQ(it.byte_index(), expected)
            << "Wrong byte_index at octet " << expected;
        ++expected;
    }
    EXPECT_EQ(expected, 4u); // visited exactly 4 octets
}

TEST_F(fspec_iterator_test, byte_index_post_increment_old_has_old_index)
{
    auto chain = make_fspec({0x01, 0x80}); // 2 bytes
    auto it  = chain[0].begin();
    auto old = it++;   // post-increment: old stays at byte 0, it is at byte 1
    EXPECT_EQ(old.byte_index(), 0u);
    EXPECT_EQ(it.byte_index(),  1u);
}

// ─── Dereference and arrow give access to all raw_fspec methods ───────────────

TEST_F(fspec_iterator_test, dereference_gives_has_extension_access)
{
    auto chain = make_fspec({0xC1, 0x80}); // byte0 has FX=1, byte1 has FX=0
    auto it = chain[0].begin();
    EXPECT_TRUE((*it).has_extension());
    ++it;
    EXPECT_FALSE((*it).has_extension());
}

TEST_F(fspec_iterator_test, arrow_gives_is_frn_active_here_access)
{
    // 0xA0 = 1010 0000 → FRN1 set (bit7), FRN3 set (bit5), FX=0
    auto chain = make_fspec({0xA0});
    auto it = chain[0].begin();
    EXPECT_TRUE(it->is_frn_active_here(1));
    EXPECT_FALSE(it->is_frn_active_here(2));
    EXPECT_TRUE(it->is_frn_active_here(3));
    EXPECT_FALSE(it->is_frn_active_here(4));
}

TEST_F(fspec_iterator_test, arrow_gives_value_access)
{
    auto chain = make_fspec({0xFE}); // all 7 FRN bits, FX=0
    auto it = chain[0].begin();
    EXPECT_EQ(it->value, 0xFEu);
}

// ─── is_frn_active() spanning multiple bytes ──────────────────────────────────

TEST_F(fspec_iterator_test, frn_active_across_two_bytes)
{
    // byte0: FRN1 (bit7) + FX (bit0) = 0x81
    // byte1: FRN8 (bit7) = 0x80, FX=0
    auto chain = make_fspec({0x81, 0x80});

    EXPECT_TRUE(chain[0].is_frn_active(1));   // byte0 bit7
    EXPECT_FALSE(chain[0].is_frn_active(2));  // byte0 bit6 = 0
    EXPECT_TRUE(chain[0].is_frn_active(8));   // byte1 bit7
    EXPECT_FALSE(chain[0].is_frn_active(9));  // byte1 bit6 = 0
}

// ─── Equality / inequality ────────────────────────────────────────────────────

TEST_F(fspec_iterator_test, equality_begin_begin)
{
    auto chain = make_fspec({0x80});
    EXPECT_EQ(chain[0].begin(), chain[0].begin());
}

TEST_F(fspec_iterator_test, inequality_begin_end)
{
    auto chain = make_fspec({0x80});
    EXPECT_NE(chain[0].begin(), chain[0].end());
}

TEST_F(fspec_iterator_test, equality_after_exhaustion)
{
    auto chain = make_fspec({0x80});
    auto it = chain[0].begin();
    ++it;
    EXPECT_EQ(it, chain[0].end());
}

TEST_F(fspec_iterator_test, two_independent_iterators_are_equal_at_start)
{
    auto chain = make_fspec({0x01, 0x80});
    auto a = chain[0].begin();
    auto b = chain[0].begin();
    EXPECT_EQ(a, b);
    ++a;
    EXPECT_NE(a, b);
    ++b;
    EXPECT_EQ(a, b);
}

// ─── End is stable (calling end() twice gives same result) ────────────────────

TEST_F(fspec_iterator_test, end_is_stable)
{
    auto chain = make_fspec({0x40});
    EXPECT_EQ(chain[0].end(), chain[0].end());
}

