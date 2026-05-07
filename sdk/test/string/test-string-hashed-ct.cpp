#include <gtest/gtest.h>
#include <types/string-hashed-ct.hpp>
#include <types/string-hashed.hpp>

#include <unordered_map>
#include <string_view>

#define GENERATE_STRING_HASHED_CT_TESTS(CHAR_TYPE, PREFIX, TYPE_NAME, CT_ALIAS, RT_ALIAS) \
    /** @brief Tests the initialization of string_hashed_ct objects. */ \
    TEST(string_hashed_ct, set_##TYPE_NAME) \
    { \
        constexpr adam::CT_ALIAS str1(PREFIX##"Hello"); \
        constexpr adam::CT_ALIAS str2(PREFIX##"Hello"); \
        constexpr adam::CT_ALIAS str3(PREFIX##"World"); \
        EXPECT_TRUE(std::basic_string_view<CHAR_TYPE>(str1.c_str()) == std::basic_string_view<CHAR_TYPE>(str2.c_str())); \
        EXPECT_TRUE(std::basic_string_view<CHAR_TYPE>(str1.c_str()) != std::basic_string_view<CHAR_TYPE>(str3.c_str())); \
    } \
    /** @brief Tests the comparison of string_hashed_ct objects. */ \
    TEST(string_hashed_ct, comparison_##TYPE_NAME) \
    { \
        constexpr adam::CT_ALIAS str1(PREFIX##"Hello"); \
        constexpr adam::CT_ALIAS str2(PREFIX##"Hello"); \
        constexpr adam::CT_ALIAS str3(PREFIX##"World"); \
        EXPECT_EQ(str1.get_hash(), str2.get_hash()); \
        EXPECT_NE(str1.get_hash(), str3.get_hash()); \
        EXPECT_TRUE(str1 == str2); \
        EXPECT_TRUE(str1 != str3); \
    } \
    /** @brief Tests the comparison of string_hashed_ct objects to runtime string_hashed. */ \
    TEST(string_hashed_ct, comparison_runtime_##TYPE_NAME) \
    { \
        constexpr adam::CT_ALIAS str1(PREFIX##"Hello"); \
        constexpr adam::CT_ALIAS str2(PREFIX##"Hello"); \
        constexpr adam::CT_ALIAS str3(PREFIX##"World"); \
        adam::RT_ALIAS str_rt1(PREFIX##"Hello"); \
        adam::RT_ALIAS str_rt2(PREFIX##"Hello"); \
        adam::RT_ALIAS str_rt3(PREFIX##"World"); \
        EXPECT_EQ(str1.get_hash(), str2.get_hash()); \
        EXPECT_EQ(str1.get_hash(), str_rt1.get_hash()); \
        EXPECT_EQ(str1.get_hash(), str_rt2.get_hash()); \
        EXPECT_NE(str1.get_hash(), str3.get_hash()); \
        EXPECT_NE(str1.get_hash(), str_rt3.get_hash()); \
        EXPECT_TRUE(str1 == str2); \
        EXPECT_TRUE(str1 != str3); \
    } \
    /** @brief Tests the insertion and retrieval of string_hashed_ct objects in a std::map. */ \
    TEST(string_hashed_ct, map_insertion_and_retrieval_##TYPE_NAME) \
    { \
        std::unordered_map<adam::CT_ALIAS, int> test_map; \
        constexpr adam::CT_ALIAS key_1(PREFIX##"param1"); \
        test_map[key_1] = 5000; \
        EXPECT_EQ(test_map.count(key_1), 1u); \
        EXPECT_EQ(test_map[key_1], 5000); \
        constexpr adam::CT_ALIAS key_2(PREFIX##"param1"); \
        EXPECT_EQ(test_map.count(key_2), 1u); \
        EXPECT_EQ(test_map[key_2], 5000); \
    } \
    /** @brief Tests the consistency of the standard library hash function with the custom hash function. */ \
    TEST(string_hashed_ct, std_hash_consistency_##TYPE_NAME) \
    { \
        constexpr adam::CT_ALIAS sh(PREFIX##"test_string"); \
        std::hash<adam::CT_ALIAS> hasher; \
        size_t system_hash = hasher(sh); \
        EXPECT_EQ(system_hash, static_cast<size_t>(sh.get_hash())); \
    }

GENERATE_STRING_HASHED_CT_TESTS(char, , char, string_hashed_ct, string_hashed)
GENERATE_STRING_HASHED_CT_TESTS(wchar_t, L, wchar, wstring_hashed_ct, wstring_hashed)
#if (defined(ADAM_PLATFORM_LINUX) && defined(_GLIBCXX_USE_CHAR8_T)) || defined(ADAM_PLATFORM_WINDOWS)
GENERATE_STRING_HASHED_CT_TESTS(char8_t, u8, char8, string_hashed_ct_utf8, string_hashed_utf8)
#endif
GENERATE_STRING_HASHED_CT_TESTS(char16_t, u, char16, string_hashed_ct_utf16, string_hashed_utf16)
GENERATE_STRING_HASHED_CT_TESTS(char32_t, U, char32, string_hashed_ct_utf32, string_hashed_utf32)
