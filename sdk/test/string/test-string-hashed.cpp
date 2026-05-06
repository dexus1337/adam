#include <gtest/gtest.h>
#include <string/string-hashed.hpp>

#include <unordered_map>
#include <string_view>
#include <chrono>
#include <iostream>

#define GENERATE_STRING_HASHED_TESTS(CHAR_TYPE, PREFIX, TYPE_NAME, RT_ALIAS) \
    /** @brief Tests the initialization of string_hashed objects. */ \
    TEST(string_hashed, set_##TYPE_NAME) \
    { \
        adam::RT_ALIAS str1(PREFIX##"Hello"); \
        adam::RT_ALIAS str2(PREFIX##"Hello"); \
        adam::RT_ALIAS str3(PREFIX##"World"); \
        EXPECT_TRUE(std::basic_string_view<CHAR_TYPE>(str1.c_str()) == std::basic_string_view<CHAR_TYPE>(str2.c_str())); \
        EXPECT_TRUE(std::basic_string_view<CHAR_TYPE>(str1.c_str()) != std::basic_string_view<CHAR_TYPE>(str3.c_str())); \
    } \
    /** @brief Tests the comparison of string_hashed objects. */ \
    TEST(string_hashed, comparison_##TYPE_NAME) \
    { \
        adam::RT_ALIAS str1(PREFIX##"Hello"); \
        adam::RT_ALIAS str2(PREFIX##"Hello"); \
        adam::RT_ALIAS str3(PREFIX##"World"); \
        EXPECT_EQ(str1.get_hash(), str2.get_hash()); \
        EXPECT_NE(str1.get_hash(), str3.get_hash()); \
        EXPECT_TRUE(str1 == str2); \
        EXPECT_TRUE(str1 != str3); \
    } \
    /** @brief Tests the recalculation of hash values after modification. */ \
    TEST(string_hashed, hash_recalculation_##TYPE_NAME) \
    { \
        adam::RT_ALIAS str1(PREFIX##"Hello"); \
        adam::RT_ALIAS str2(PREFIX##"Hello"); \
        adam::RT_ALIAS str3(PREFIX##"Hello!"); \
        str2.append(PREFIX##"!"); \
        EXPECT_TRUE(str2 != str3); \
        str2.calculate_hash(); \
        EXPECT_TRUE(str2 == str3); \
    } \
    /** @brief Tests the copy constructor and string conversion. */ \
    TEST(string_hashed, copy_##TYPE_NAME) \
    { \
        adam::RT_ALIAS sh(PREFIX##"test_string"); \
        adam::RT_ALIAS sh_copy(PREFIX##"different_string"); \
        EXPECT_TRUE(sh_copy != sh); \
        sh_copy = sh; \
        EXPECT_TRUE(sh_copy == sh); \
        std::basic_string<CHAR_TYPE> std_str_copy = sh_copy; \
        EXPECT_TRUE(std_str_copy == sh.c_str()); \
    } \
    /** @brief Tests the insertion and retrieval of string_hashed objects in a std::map. */ \
    TEST(string_hashed, map_insertion_and_retrieval_##TYPE_NAME) \
    { \
        std::unordered_map<adam::RT_ALIAS, int> test_map; \
        adam::RT_ALIAS key_1(PREFIX##"param1"); \
        test_map[key_1] = 5000; \
        EXPECT_EQ(test_map.count(key_1), 1u); \
        EXPECT_EQ(test_map[key_1], 5000); \
        adam::RT_ALIAS key_2(PREFIX##"param1"); \
        EXPECT_EQ(test_map.count(key_2), 1u); \
        EXPECT_EQ(test_map[key_2], 5000); \
    } \
    /** @brief Tests the consistency of the standard library hash function with the custom hash function. */ \
    TEST(string_hashed, std_hash_consistency_##TYPE_NAME) \
    { \
        adam::RT_ALIAS sh(PREFIX##"test_string"); \
        std::hash<adam::RT_ALIAS> hasher; \
        size_t system_hash = hasher(sh); \
        EXPECT_EQ(system_hash, static_cast<size_t>(sh.get_hash())); \
    }

GENERATE_STRING_HASHED_TESTS(char, , char, string_hashed)
GENERATE_STRING_HASHED_TESTS(wchar_t, L, wchar, wstring_hashed)
#if (defined(ADAM_PLATFORM_LINUX) && defined(_GLIBCXX_USE_CHAR8_T)) || defined(ADAM_PLATFORM_WINDOWS)
GENERATE_STRING_HASHED_TESTS(char8_t, u8, char8, string_hashed_utf8)
#endif
GENERATE_STRING_HASHED_TESTS(char16_t, u, char16, string_hashed_utf16)
GENERATE_STRING_HASHED_TESTS(char32_t, U, char32, string_hashed_utf32)

/** @brief Benchmark test comparing string_hashed (rapidhash) to standard FNV-1a. */
TEST(string_hashed, benchmark_vs_fnv1a)
{
    const size_t num_iterations = 300000;
    adam::string_hashed sh("This is a moderately long string to test the hashing performance of rapidhash vs fnv1a!");
    std::string test_str(sh.c_str());

    // Benchmark adam::string_hashed (rapidhash)
    auto start_rapid = std::chrono::high_resolution_clock::now();
    uint64_t dummy_rapid = 0;
    for (size_t i = 0; i < num_iterations; ++i) {
        sh[0] = static_cast<char>(i % 256);
        sh.calculate_hash();
        dummy_rapid ^= sh.get_hash();
    }
    auto end_rapid = std::chrono::high_resolution_clock::now();
    auto duration_rapid = std::chrono::duration_cast<std::chrono::microseconds>(end_rapid - start_rapid).count();

    // Benchmark FNV-1a
    auto start_fnv = std::chrono::high_resolution_clock::now();
    uint64_t dummy_fnv = 0;
    for (size_t i = 0; i < num_iterations; ++i) {
        test_str[0] = static_cast<char>(i % 256);
        uint64_t hash = 0xcbf29ce484222325ULL;
        for (size_t j = 0; j < test_str.length(); ++j) {
            hash ^= static_cast<uint8_t>(test_str[j]);
            hash *= 0x100000001b3ULL;
        }
        dummy_fnv ^= hash;
    }
    auto end_fnv = std::chrono::high_resolution_clock::now();
    auto duration_fnv = std::chrono::duration_cast<std::chrono::microseconds>(end_fnv - start_fnv).count();

    std::cout << "[          ] string_hashed (rapidhash) duration: " << duration_rapid << " us\n";
    std::cout << "[          ] std::string (FNV-1a) duration:      " << duration_fnv << " us\n";

    if (duration_rapid > 0) {
        double times = static_cast<double>(duration_fnv) / static_cast<double>(duration_rapid);
        std::cout << "[          ] -> string_hashed with rapidhash is " << times << " times faster than FNV-1a hash\n";
    }

    // Output dummy values to EXPECT so the compiler doesn't optimize the loop code into nothingness
    EXPECT_NE(dummy_rapid, 0u);
    EXPECT_NE(dummy_fnv, 0u);

    // Assert that rapidhash is faster!
    EXPECT_LT(duration_rapid, duration_fnv);
}
