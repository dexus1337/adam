#pragma once

/**
 * @file    test-messages.hpp
 * @brief   Helper functions to build raw ASTERIX messages for parsing and encoding tests.
 */

#include <vector>
#include <cstdint>

inline std::vector<uint8_t> build_cat048_message()
{
    std::vector<uint8_t> raw;
    raw.push_back(48);                 // Category byte

    // Reserve two length bytes (big‑endian)
    raw.push_back(0);
    raw.push_back(0);

    // Record start – FSPEC (fixed‑size, 4 octets = ceil(28/8))
    // All bits set – every item present
    raw.push_back(0xFF);   // Octet 1
    raw.push_back(0xFF);   // Octet 2
    raw.push_back(0xFF);   // Octet 3
    raw.push_back(0xFE);   // Octet 4

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

    // 15 – Track Quality (4 bytes)
    push_u32(0x01020304);

    // 16 – Warning/Error Conditions (variable, multi‑octet payload using FX bits)
    //    First octet with FX=1
    push_u8(0xD3); // example payload with FX=1
    //    Second octet ends the field (FX=0)
    push_u8(0x14);

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
    push_u8(0x06); // length of SPF
    push_u8(0x80); // FSPEC (1 item)
    push_u32(0x13371337);

    // 28 – Reserved Expansion Field (explicit, sub‑UAP REF)
    size_t ref_len_offset = raw.size();
    raw.push_back(0); // placeholder for length
    push_u8(0xFF); // FSPEC for REF (all items present)

    // ---- MD5 sub‑UAP (7 fixed items) -------------------------------
    // FSPEC for MD5 (all 7 fixed items present: 7 bits → 1 octet)
    push_u8(0xFE);   // 11111110 = all 7 fixed bits set
    push_u8(0x01);   // Mode 5 Summary
    push_u8(0x02);   // PIN / Nat Origin / Mission Code (4 bytes)
    push_u8(0x03);   // Reported Position (6 bytes)
    raw.insert(raw.end(), {0x03,0x04,0x05,0x06,0x07,0x08});
    push_u8(0x07);   // GNSS‑derived Altitude (2 bytes)
    raw.insert(raw.end(), {0x07,0x08});
    push_u8(0x09);   // Ext Mode 1 Code (2 bytes)
    raw.insert(raw.end(), {0x09,0x0A});
    push_u8(0x0B);   // Time Offset (1 byte)
    push_u8(0x0C);   // X Pulse Presence (1 byte)

    // ---- M5N sub‑UAP (8 fixed items) -------------------------------
    // FSPEC for M5N (all 8 fixed items present: 8 bits → 1 octet)
    push_u8(0xFF);   // all 8 fixed bits set, 7 items + expansion
    push_u8(0x80);   // first item present
    push_u8(0x0D);   // Mode 5 Summary
    raw.insert(raw.end(), {0x11,0x22,0x33,0x44});
    raw.insert(raw.end(), {0x12,0x13,0x14,0x15,0x16,0x17});
    raw.insert(raw.end(), {0x18,0x19});
    raw.insert(raw.end(), {0x1A,0x1B});
    push_u8(0x1C);   // Time Offset (1 byte)
    push_u8(0x1D);   // X Pulse Presence (1 byte)
    push_u8(0x1E);   // Figure of Merit (1 byte)

    // ---- M4E sub‑UAP (variable) ------------------------------------
    push_u8(0x01);
    push_u8(0x02);

    // ---- RPC sub‑UAP (4 fixed items) -------------------------------
    push_u8(0xF0);   // FSPEC
    push_u8(0x20);   // Score
    push_u8(0x21);   // Signal/Clutter Ratio (2 bytes)
    push_u8(0x22);
    push_u8(0x23);   // Range Width (2 bytes)
    push_u8(0x24);
    push_u8(0x25);   // Ambiguous Range (2 bytes)
    push_u8(0x26);

    // ---- ERR Extended Range Reports (3 fixed length) --------------
    push_u8(0x01);
    push_u8(0x02);
    push_u8(0x03);

    // ---- RTC sub‑UAP (7 fixed + 2 repetitive) --------------------
    push_u8(0xFF);   // FSPEC
    push_u8(0xF0);   // FSPEC
    push_u8(0x30);   // Plot/Track Link (3 bytes)
    raw.insert(raw.end(), {0x30,0x31});
    push_u8(0x01);   // ADS‑B/Track Link (repetitive, 2 bytes)
    push_u8(0x41);
    push_u8(0x42);
    push_u8(0x50);   // Turn State (1 byte)
    raw.insert(raw.end(), 22, 0x51); // Next Predicted Position (22 bytes)
    push_u8(0x01);   // Data Link Characteristics
    push_u8(0x61);
    push_u8(0x70);   // Lockout Characteristics (2 bytes)
    push_u8(0x71);
    raw.insert(raw.end(), 6, 0x80); // Transition Codes (6 bytes)
    raw.insert(raw.end(), 4, 0x90); // Track Lifecycle (4 bytes)
    push_u8(0x01);   // Adjacent Sensor Information (repetitive, 8 bytes)
    raw.insert(raw.end(), 8, 0xA0);
    push_u8(0xB0);   // Track Extrapolation Source (1 byte)
    push_u8(0xC0);   // Identity Requested (1 byte)

    // ---- CPC sub‑UAP (4 fixed items) -------------------------------
    push_u8(0xF0);   // FSPEC
    push_u8(0xD0);   // Plot Number (2 bytes)
    push_u8(0xD1);
    push_u8(0x01);   // Replies/Plot Link (repetitive, 3 bytes)
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
    
    // Fill length fields
    uint16_t block_len = static_cast<uint16_t>(raw.size());
    raw[1] = static_cast<uint8_t>(block_len >> 8);
    raw[2] = static_cast<uint8_t>(block_len & 0xFF);

    /*for (int i = 0; i < raw.size(); i+= 16)
    {
        for (int n = 0; n < 16; n++)
        {
            if (i+n >= raw.size()) continue;

            std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)raw[i+n] << " ";
        }

        std::cout << std::endl;
    }*/
   
    return raw;
}
