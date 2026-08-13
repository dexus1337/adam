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

    for (size_t i = 0; i < raw.size(); i+= 16)
    {
        for (size_t n = 0; n < 16; n++)
        {
            if (i+n >= raw.size()) continue;

            std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)raw[i+n] << " ";
        }

        std::cout << std::endl;
    }
   
    return raw;
}

inline std::vector<uint8_t> build_cat021_message()
{
    std::vector<uint8_t> raw;
    raw.push_back(21);                 // Category byte

    // Reserve two length bytes (big-endian)
    raw.push_back(0);
    raw.push_back(0);

    // Record start – FSPEC for CAT021 (7 octets)
    // FRNs 1-42 present (Bytes 1-6: 0xFF), Byte 7 has FRN 48 (RE) & FRN 49 (SP) set (0x06)
    raw.push_back(0xFF);   // Octet 1 (FRN 1-7, FX=1)
    raw.push_back(0xFF);   // Octet 2 (FRN 8-14, FX=1)
    raw.push_back(0xFF);   // Octet 3 (FRN 15-21, FX=1)
    raw.push_back(0xFF);   // Octet 4 (FRN 22-28, FX=1)
    raw.push_back(0xFF);   // Octet 5 (FRN 29-35, FX=1)
    raw.push_back(0xFF);   // Octet 6 (FRN 36-42, FX=1)
    raw.push_back(0x06);   // Octet 7 (FRN 48 & 49 set, FX=0)

    // Helper lambdas
    auto push_u8  = [&](uint8_t v) { raw.push_back(v); };
    auto push_u16 = [&](uint16_t v) { raw.push_back(v >> 8); raw.push_back(v & 0xFF); };
    auto push_u24 = [&](uint32_t v)
    {
        raw.push_back((v >> 16) & 0xFF);
        raw.push_back((v >> 8) & 0xFF);
        raw.push_back(v & 0xFF);
    };
    auto push_u32 = [&](uint32_t v)
    {
        raw.push_back(v >> 24);
        raw.push_back((v >> 16) & 0xFF);
        raw.push_back((v >> 8) & 0xFF);
        raw.push_back(v & 0xFF);
    };

    // 1 – I021/010 Data Source Identifier (2 bytes)
    push_u16(0x0102);

    // 2 – I021/040 Target Report Descriptor (variable, 1 byte, FX=0)
    push_u8(0x40);

    // 3 – I021/161 Track Number (2 bytes)
    push_u16(0x1234);

    // 4 – I021/015 Service Identification (1 byte)
    push_u8(0x05);

    // 5 – I021/071 Time of Applicability for Position (3 bytes)
    push_u24(0x010203);

    // 6 – I021/130 Position in WGS-84 co-ordinates (6 bytes)
    raw.insert(raw.end(), {0x01, 0x02, 0x03, 0x04, 0x05, 0x06});

    // 7 – I021/131 Position in WGS-84 co-ordinates, high res (8 bytes)
    raw.insert(raw.end(), {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08});

    // 8 – I021/072 Time of Applicability for Velocity (3 bytes)
    push_u24(0x040506);

    // 9 – I021/150 Air Speed (2 bytes)
    push_u16(0x0150);

    // 10 – I021/151 True Air Speed (2 bytes)
    push_u16(0x0151);

    // 11 – I021/080 Target Address (3 bytes)
    push_u24(0xABCDEF);

    // 12 – I021/073 Time of Message Reception of Position (3 bytes)
    push_u24(0x070809);

    // 13 – I021/074 Time of Message Reception of Position-High Precision (4 bytes)
    push_u32(0x0A0B0C0D);

    // 14 – I021/075 Time of Message Reception of Velocity (3 bytes)
    push_u24(0x0E0F10);

    // 15 – I021/076 Time of Message Reception of Velocity-High Precision (4 bytes)
    push_u32(0x11121314);

    // 16 – I021/140 Geometric Height (2 bytes)
    push_u16(0x1400);

    // 17 – I021/090 Quality Indicators (variable, 1 byte, FX=0)
    push_u8(0x90);

    // 18 – I021/210 MOPS Version (1 byte)
    push_u8(0x03);

    // 19 – I021/070 Mode 3/A Code (2 bytes)
    push_u16(0x0700);

    // 20 – I021/230 Roll Angle (2 bytes)
    push_u16(0x0230);

    // 21 – I021/145 Flight Level (2 bytes)
    push_u16(0x0145);

    // 22 – I021/152 Magnetic Heading (2 bytes)
    push_u16(0x0152);

    // 23 – I021/200 Target Status (1 byte)
    push_u8(0x20);

    // 24 – I021/155 Barometric Vertical Rate (2 bytes)
    push_u16(0x0155);

    // 25 – I021/157 Geometric Vertical Rate (2 bytes)
    push_u16(0x0157);

    // 26 – I021/160 Airborne Ground Vector (4 bytes)
    push_u32(0x01600160);

    // 27 – I021/165 Track Angle Rate (2 bytes)
    push_u16(0x0165);

    // 28 – I021/077 Time of Report Transmission (3 bytes)
    push_u24(0x070707);

    // 29 – I021/170 Target Identification (6 bytes ASCII "CAT021")
    raw.insert(raw.end(), {'C', 'A', 'T', '0', '2', '1'});

    // 30 – I021/020 Emitter Category (1 byte)
    push_u8(0x01);

    // 31 – I021/220 Met Information (compound, sub-UAP 220)
    //      FSPEC for sub-UAP 220: 0xF0 (WS, WD, TMP, TRB present)
    push_u8(0xF0);
    push_u16(0x000A); // WS (2 B)
    push_u16(0x00B4); // WD (2 B)
    push_u16(0x0019); // TMP (2 B)
    push_u8(0x01);   // TRB (1 B)

    // 32 – I021/146 Selected Altitude (2 bytes)
    push_u16(0x0146);

    // 33 – I021/148 Final State Selected Altitude (2 bytes)
    push_u16(0x0148);

    // 34 – I021/110 Trajectory Intent (compound, sub-UAP 110)
    //      FSPEC for sub-UAP 110: 0xC0 (TIS, TID present)
    push_u8(0xC0);
    push_u8(0x01); // TIS (1 B)
    push_u8(1);    // TID repetition count (1)
    raw.insert(raw.end(), 15, 0xAA); // TID entry (15 B)

    // 35 – I021/016 Service Management (1 byte)
    push_u8(0x16);

    // 36 – I021/008 Aircraft Operational Status (1 byte)
    push_u8(0x08);

    // 37 – I021/271 Surface Capabilities and Characteristics (variable, 1 byte, FX=0)
    push_u8(0x71);

    // 38 – I021/132 Message Amplitude (1 byte)
    push_u8(0x32);

    // 39 – I021/250 BDS Register Data (repetitive, 8 bytes per entry)
    push_u8(1); // 1 repetition
    push_u32(0x11223344);
    push_u32(0x55667788);

    // 40 – I021/260 ACAS Resolution Advisory Report (7 bytes)
    raw.insert(raw.end(), {1, 2, 3, 4, 5, 6, 7});

    // 41 – I021/400 Receiver ID (1 byte)
    push_u8(0x40);

    // 42 – I021/295 Data Ages (compound, sub-UAP 295)
    //      FSPEC: 0xFF 0xFF 0xFF 0xC0 (23 sub-items present)
    push_u8(0xFF);
    push_u8(0xFF);
    push_u8(0xFF);
    push_u8(0xC0);
    raw.insert(raw.end(), 23, 0x05); // 23 data age bytes

    // 48 – Reserved Expansion Field (explicit, sub-UAP REF)
    size_t ref_len_offset = raw.size();
    raw.push_back(0); // length placeholder
    push_u8(0xFF);    // Explicit FSPEC for CAT021 REF (all 8 FRNs present, 8 bits/octet, no FX)

    // ---- BPS (fixed, 2 B) ----
    push_u16(0x0123);
    // ---- SelH (fixed, 2 B) ----
    push_u16(0x0456);
    // ---- NAV (fixed, 1 B) ----
    push_u8(0x78);
    // ---- GAO (fixed, 1 B) ----
    push_u8(0x90);
    // ---- SGV (variable, 2 B primary, FX=0) ----
    push_u16(0x1234);
    // ---- STA (variable, 1 B primary, FX=0) ----
    push_u8(0x56);
    // ---- TNH (fixed, 2 B) ----
    push_u16(0x7890);
    // ---- MES (compound sub-UAP, 6 items) ----
    push_u8(0xFC); // FSPEC for MES (6 items present, FX=0)
    push_u8(0x01); // Mode 5 Summary (1 B)
    push_u32(0x11223344); // PIN / National Origin (4 B)
    push_u16(0x5566); // Ext Mode 1 (2 B)
    push_u8(0x07); // X Pulse (1 B)
    push_u8(0x08); // FOM (1 B)
    push_u16(0x99AA); // Mode 2 Code (2 B)

    // Compute REF length
    size_t ref_len = raw.size() - ref_len_offset;
    raw[ref_len_offset] = static_cast<uint8_t>(ref_len);

    // 49 – Special Purpose Field (explicit)
    push_u8(0x06); // length of SPF
    push_u8(0x80); // FSPEC
    push_u32(0x13371337);

    // Fill block length bytes 1-2
    uint16_t block_len = static_cast<uint16_t>(raw.size());
    raw[1] = static_cast<uint8_t>(block_len >> 8);
    raw[2] = static_cast<uint8_t>(block_len & 0xFF);

    return raw;
}

inline std::vector<uint8_t> build_cat062_message()
{
    std::vector<uint8_t> raw;
    raw.push_back(62);                 // Category byte

    // Reserve two length bytes (big-endian)
    raw.push_back(0);
    raw.push_back(0);

    // Record start – FSPEC for CAT062 (5 octets)
    // Octet 1: 0xBF (FRNs 1,3,4,5,6,7, FX=1)
    // Octet 2: 0xFF (FRNs 8-14, FX=1)
    // Octet 3: 0xFF (FRNs 15-21, FX=1)
    // Octet 4: 0xFF (FRNs 22-28, FX=1)
    // Octet 5: 0x06 (FRNs 34 [RE] & 35 [SP], FX=0)
    raw.push_back(0xBF);
    raw.push_back(0xFF);
    raw.push_back(0xFF);
    raw.push_back(0xFF);
    raw.push_back(0x06);

    // Helper lambdas
    auto push_u8  = [&](uint8_t v) { raw.push_back(v); };
    auto push_u16 = [&](uint16_t v) { raw.push_back(v >> 8); raw.push_back(v & 0xFF); };
    auto push_u24 = [&](uint32_t v)
    {
        raw.push_back((v >> 16) & 0xFF);
        raw.push_back((v >> 8) & 0xFF);
        raw.push_back(v & 0xFF);
    };
    auto push_u32 = [&](uint32_t v)
    {
        raw.push_back(v >> 24);
        raw.push_back((v >> 16) & 0xFF);
        raw.push_back((v >> 8) & 0xFF);
        raw.push_back(v & 0xFF);
    };

    // 1 – I062/010 Data Source Identifier (2 bytes)
    push_u16(0x0102);

    // 3 – I062/015 Service Identification (1 byte)
    push_u8(0x03);

    // 4 – I062/070 Time Of Track Information (3 bytes)
    push_u24(0x040506);

    // 5 – I062/105 Calculated Track Position (WGS-84) (8 bytes)
    raw.insert(raw.end(), {1, 2, 3, 4, 5, 6, 7, 8});

    // 6 – I062/100 Calculated Track Position (Cartesian) (6 bytes)
    raw.insert(raw.end(), {1, 2, 3, 4, 5, 6});

    // 7 – I062/185 Calculated Track Velocity (Cartesian) (4 bytes)
    push_u32(0x07070707);

    // 8 – I062/210 Calculated Acceleration (Cartesian) (2 bytes)
    push_u16(0x0808);

    // 9 – I062/060 Track Mode 3/A Code (2 bytes)
    push_u16(0x0909);

    // 10 – I062/245 Target Identification (7 bytes ASCII "CAT062 ")
    raw.insert(raw.end(), {'C', 'A', 'T', '0', '6', '2', ' '});

    // 11 – I062/380 Aircraft Derived Data (compound sub-UAP 380, 28 items)
    //      FSPEC: 0xFF 0xFF 0xFF 0xFE
    push_u8(0xFF);
    push_u8(0xFF);
    push_u8(0xFF);
    push_u8(0xFE);
    push_u24(0x010203); // 1: ADR (3 B)
    raw.insert(raw.end(), {'A', 'B', 'C', 'D', 'E', 'F'}); // 2: ID (6 B)
    push_u16(0x0300);   // 3: MHG (2 B)
    push_u16(0x0400);   // 4: IAS (2 B)
    push_u16(0x0500);   // 5: TAS (2 B)
    push_u16(0x0600);   // 6: SAL (2 B)
    push_u16(0x0700);   // 7: FSS (2 B)
    push_u8(0x01);      // 8: TIS (1 B)
    push_u8(1);         // 9: TID rep count (1)
    raw.insert(raw.end(), 15, 0x11); // 9: TID entry (15 B)
    push_u16(0x1000);   // 10: COM (2 B)
    push_u16(0x1100);   // 11: SAB (2 B)
    raw.insert(raw.end(), {1, 2, 3, 4, 5, 6, 7}); // 12: ACS (7 B)
    push_u16(0x1300);   // 13: BVR (2 B)
    push_u16(0x1400);   // 14: GVR (2 B)
    push_u16(0x1500);   // 15: RAN (2 B)
    push_u16(0x1600);   // 16: TAR (2 B)
    push_u16(0x1700);   // 17: TAN (2 B)
    push_u16(0x1800);   // 18: GSP (2 B)
    push_u8(0x19);      // 19: VUN (1 B)
    raw.insert(raw.end(), 8, 0x20); // 20: MET (8 B)
    push_u8(0x21);      // 21: EMC (1 B)
    raw.insert(raw.end(), {1, 2, 3, 4, 5, 6}); // 22: POS (6 B)
    push_u16(0x2300);   // 23: GAL (2 B)
    push_u8(0x24);      // 24: PUN (1 B)
    push_u8(1);         // 25: MB rep count (1)
    push_u32(0x11111111); push_u32(0x22222222); // 25: MB entry (8 B)
    push_u16(0x2600);   // 26: IAR (2 B)
    push_u16(0x2700);   // 27: MAC (2 B)
    push_u16(0x2800);   // 28: BPS (2 B)

    // 12 – I062/040 Track Number (2 bytes)
    push_u16(0x1212);

    // 13 – I062/080 Track Status (variable, 1 byte, FX=0)
    push_u8(0x80);

    // 14 – I062/290 System Track Update Ages (compound sub-UAP 290, 10 items)
    //      FSPEC: 0xFF 0xE0
    push_u8(0xFF);
    push_u8(0xE0);
    raw.insert(raw.end(), 10, 0x0A); // 10 age bytes

    // 15 – I062/200 Mode of Movement (1 byte)
    push_u8(0x15);

    // 16 – I062/295 Track Data Ages (compound sub-UAP 295, 31 items)
    //      FSPEC: 0xFF 0xFF 0xFF 0xFF 0xE0
    push_u8(0xFF);
    push_u8(0xFF);
    push_u8(0xFF);
    push_u8(0xFF);
    push_u8(0xE0);
    raw.insert(raw.end(), 31, 0x0B); // 31 age bytes

    // 17 – I062/136 Measured Flight Level (2 bytes)
    push_u16(0x1717);

    // 18 – I062/130 Calculated Track Geometric Altitude (2 bytes)
    push_u16(0x1818);

    // 19 – I062/135 Calculated Track Barometric Altitude (2 bytes)
    push_u16(0x1919);

    // 20 – I062/220 Calculated Rate Of Climb/Descent (2 bytes)
    push_u16(0x2020);

    // 21 – I062/390 Flight Plan Related Data (compound sub-UAP 390, 18 items)
    //      FSPEC: 0xFF 0xFF 0xF0
    push_u8(0xFF);
    push_u8(0xFF);
    push_u8(0xF0);
    push_u16(0x0100); // 1: TAG (2 B)
    raw.insert(raw.end(), {'C', 'S', 'N', '1', '2', '3'}); // 2: CSN (6 B)
    push_u32(0x03000000); // 3: IFI (4 B)
    push_u8(0x04);    // 4: FCT (1 B)
    raw.insert(raw.end(), {'A', '3', '2', '0'}); // 5: TAC (4 B)
    push_u8(0x06);    // 6: WTC (1 B)
    raw.insert(raw.end(), {'E', 'D', 'D', 'F'}); // 7: DEP (4 B)
    raw.insert(raw.end(), {'E', 'D', 'D', 'M'}); // 8: DST (4 B)
    raw.insert(raw.end(), {'2', '5', 'R'});     // 9: RDS (3 B)
    push_u16(0x1000); // 10: CFL (2 B)
    push_u16(0x1100); // 11: CTL (2 B)
    push_u8(1);       // 12: TOD rep count (1)
    push_u32(0x12000000); // 12: TOD entry (4 B)
    raw.insert(raw.end(), {'S', 'T', 'A', 'N', 'D', '1'}); // 13: AST (6 B)
    push_u8(0x14);    // 14: STS (1 B)
    raw.insert(raw.end(), {'S', 'I', 'D', '1', '2', '3', '4'}); // 15: STD (7 B)
    raw.insert(raw.end(), {'S', 'T', 'A', '1', '2', '3', '4'}); // 16: STA (7 B)
    push_u16(0x1700); // 17: PEM (2 B)
    raw.insert(raw.end(), {'P', 'R', 'E', 'C', 'A', 'L'});     // 18: PEC (6 B)

    // 22 – I062/270 Target Size & Orientation (variable, 1 byte, FX=0)
    push_u8(0x27);

    // 23 – I062/300 Vehicle Fleet Identification (1 byte)
    push_u8(0x30);

    // 24 – I062/110 Mode 5 Data reports & Extended Mode 1 Code (compound sub-UAP 110, 7 items)
    //      FSPEC: 0xFE
    push_u8(0xFE);
    push_u8(0x01);        // 1: SUM (1 B)
    push_u32(0x02000000); // 2: PMN (4 B)
    raw.insert(raw.end(), {1, 2, 3, 4, 5, 6}); // 3: POS (6 B)
    push_u16(0x0400);     // 4: GA (2 B)
    push_u16(0x0500);     // 5: EM1 (2 B)
    push_u8(0x06);        // 6: TOS (1 B)
    push_u8(0x07);        // 7: XP (1 B)

    // 25 – I062/120 Track Mode 2 Code (2 bytes)
    push_u16(0x1200);

    // 26 – I062/510 Composed Track Number (variable, 3 bytes, FX=0)
    push_u24(0x010200);

    // 27 – I062/500 Estimated Accuracies (compound sub-UAP 500, 8 items)
    //      FSPEC: 0xFF 0x80
    push_u8(0xFF);
    push_u8(0x80);
    push_u32(0x01000000); // 1: APC (4 B)
    push_u16(0x0200);     // 2: COV (2 B)
    push_u32(0x03000000); // 3: APW (4 B)
    push_u8(0x04);        // 4: AGA (1 B)
    push_u8(0x05);        // 5: ABA (1 B)
    push_u16(0x0600);     // 6: ATV (2 B)
    push_u16(0x0700);     // 7: AA (2 B)
    push_u8(0x08);        // 8: ARC (1 B)

    // 28 – I062/340 Measured Information (compound sub-UAP 340, 6 items)
    //      FSPEC: 0xFC
    push_u8(0xFC);
    push_u16(0x0100);     // 1: SID (2 B)
    push_u32(0x02000000); // 2: POS (4 B)
    push_u16(0x0300);     // 3: HEI (2 B)
    push_u16(0x0400);     // 4: MDC (2 B)
    push_u16(0x0500);     // 5: MDA (2 B)
    push_u8(0x06);        // 6: TYP (1 B)

    // 34 – Reserved Expansion Field (explicit, sub-UAP REF)
    size_t ref_len_offset = raw.size();
    raw.push_back(0); // length placeholder
    push_u8(0xFF);    // Explicit FSPEC for CAT062 REF (all 8 FRNs present, 8 bits/octet, no FX)

    // ---- CST (repetitive, 5 B per entry) ----
    push_u8(1); // rep count
    raw.insert(raw.end(), {1, 2, 3, 4, 5});

    // ---- CSN (repetitive, 3 B per entry) ----
    push_u8(1); // rep count
    raw.insert(raw.end(), {1, 2, 3});

    // ---- TVS (fixed, 4 B) ----
    push_u32(0x03030303);

    // ---- STS (variable, 1 B, FX=0) ----
    push_u8(0x04);

    // ---- V3 (compound sub-UAP, 4 items) ----
    push_u8(0xF0); // FSPEC (FRN 1-4 present, FX=0)
    push_u8(0x01); // PS3 (1 B)
    raw.insert(raw.end(), {1, 2, 3}); // AS (3 B)
    push_u8(0x03); // UAS (1 B)
    push_u8(0x04); // CASS (1 B)

    // ---- MOI (explicit sub-UAP, 25 items) ----
    size_t moi_len_offset = raw.size();
    raw.push_back(0); // MOI length placeholder
    push_u8(0xFF);    // Explicit FSPEC octet 1 (FRNs 1-8)
    push_u8(0xFF);    // Explicit FSPEC octet 2 (FRNs 9-16)
    push_u8(0xFF);    // Explicit FSPEC octet 3 (FRNs 17-24)
    push_u8(0x80);    // Explicit FSPEC octet 4 (FRN 25)
    push_u8(0x01); push_u8(0x02); push_u8(0x03); push_u8(0x04);
    push_u8(0x05); push_u8(0x06); push_u8(0x07); push_u8(0x08); push_u8(0x09);
    push_u8(1); push_u16(0x0A00); // 10: ABDS (rep 1, 2 B)
    push_u8(1); push_u8(0x0B);    // 11: MPID (rep 1, 1 B)
    push_u32(0x0C000000); // 12: LS (4 B)
    push_u32(0x0D000000); // 13: LSQI (4 B)
    push_u16(0x0E00); // 14: DTNH (2 B)
    push_u16(0x0F00); // 15: DMNH (2 B)
    push_u16(0x1000); // 16: DTNT (2 B)
    push_u16(0x1100); // 17: DMNT (2 B)
    push_u16(0x1200); // 18: TBP (2 B)
    push_u16(0x1300); // 19: ALTQCMFL (2 B)
    push_u16(0x1400); // 20: CTBA (2 B)
    push_u8(1); raw.insert(raw.end(), 8, 0x15); // 21: INPS (rep 1, 8 B)
    push_u16(0x1600); // 22: FPVHR (2 B)
    raw.insert(raw.end(), {'S', 'U', 'C', 'T', 'E', 'X', 'T'}); // 23: SCT (7 B)
    push_u16(0x1800); // 24: SCSM (2 B)
    push_u16(0x1900); // 25: TCAT (variable, 2 B, FX=0)

    size_t moi_len = raw.size() - moi_len_offset;
    raw[moi_len_offset] = static_cast<uint8_t>(moi_len);

    // ---- MTI (explicit sub-UAP, 9 items) ----
    size_t mti_len_offset = raw.size();
    raw.push_back(0); // MTI length placeholder
    push_u8(0xFF);    // Explicit FSPEC octet 1 (FRNs 1-8)
    push_u8(0x80);    // Explicit FSPEC octet 2 (FRN 9)
    push_u32(0x01000000); // 1: DATE (4 B)
    push_u16(0x0200);     // 2: TENTU (2 B)
    push_u32(0x03000000); // 3: IPMC (4 B)
    push_u8(1); push_u16(0x0400); // 4: IMP (rep 1, 2 B)
    push_u8(1); raw.insert(raw.end(), 15, 0x05); // 5: IST (rep 1, 15 B)
    push_u24(0x060000);   // 6: TTT (3 B)
    push_u16(0x0700);     // 7: EXM3A (2 B)
    push_u24(0x080000);   // 8: EXADDR (3 B)
    raw.insert(raw.end(), {'E', 'X', 'T', 'I', 'D', ' '}); // 9: EXTID (6 B)

    size_t mti_len = raw.size() - mti_len_offset;
    raw[mti_len_offset] = static_cast<uint8_t>(mti_len);

    // ---- GEN62 (compound sub-UAP, 1 item) ----
    push_u8(0x80); // FSPEC
    push_u8(0x00); // placeholder payload

    // Compute REF length
    size_t ref_len = raw.size() - ref_len_offset;
    raw[ref_len_offset] = static_cast<uint8_t>(ref_len);

    // 35 – Special Purpose Field (explicit)
    push_u8(0x06); // length of SPF
    push_u8(0x80); // FSPEC
    push_u32(0x13371337);

    // Fill block length bytes 1-2
    uint16_t block_len = static_cast<uint16_t>(raw.size());
    raw[1] = static_cast<uint8_t>(block_len >> 8);
    raw[2] = static_cast<uint8_t>(block_len & 0xFF);

    return raw;
}

