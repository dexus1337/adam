#include <gtest/gtest.h>
#include <types/string-hashed-ct.hpp>
#include <types/string-hashed.hpp>

#include <unordered_map>
#include <string_view>

#define GENERATE_STRING_HASHED_CT_TESTS(CHAR_TYPE, PREFIX, TYPE_NAME) \
    /** @brief Tests the initialization of string_hashed_ct_template objects. */ \
    TEST(string_hashed_ct, set_##TYPE_NAME) \
    { \
        constexpr adam::string_hashed_ct_template<CHAR_TYPE> str1(PREFIX##"Hello"); \
        constexpr adam::string_hashed_ct_template<CHAR_TYPE> str2(PREFIX##"Hello"); \
        constexpr adam::string_hashed_ct_template<CHAR_TYPE> str3(PREFIX##"World"); \
        EXPECT_TRUE(std::basic_string_view<CHAR_TYPE>(str1.c_str()) == std::basic_string_view<CHAR_TYPE>(str2.c_str())); \
        EXPECT_TRUE(std::basic_string_view<CHAR_TYPE>(str1.c_str()) != std::basic_string_view<CHAR_TYPE>(str3.c_str())); \
    } \
    /** @brief Tests the comparison of string_hashed_ct_template objects. */ \
    TEST(string_hashed_ct, comparison_##TYPE_NAME) \
    { \
        constexpr adam::string_hashed_ct_template<CHAR_TYPE> str1(PREFIX##"Hello"); \
        constexpr adam::string_hashed_ct_template<CHAR_TYPE> str2(PREFIX##"Hello"); \
        constexpr adam::string_hashed_ct_template<CHAR_TYPE> str3(PREFIX##"World"); \
        EXPECT_EQ(str1.get_hash(), str2.get_hash()); \
        EXPECT_NE(str1.get_hash(), str3.get_hash()); \
        EXPECT_TRUE(str1 == str2); \
        EXPECT_TRUE(str1 != str3); \
    } \
    /** @brief Tests the comparison of string_hashed_ct_template objects to runtime string_hashed_template. */ \
    TEST(string_hashed_ct, comparison_runtime_##TYPE_NAME) \
    { \
        constexpr adam::string_hashed_ct_template<CHAR_TYPE> str1(PREFIX##"Hello"); \
        constexpr adam::string_hashed_ct_template<CHAR_TYPE> str2(PREFIX##"Hello"); \
        constexpr adam::string_hashed_ct_template<CHAR_TYPE> str3(PREFIX##"World"); \
        adam::string_hashed_template<CHAR_TYPE> str_rt1(PREFIX##"Hello"); \
        adam::string_hashed_template<CHAR_TYPE> str_rt2(PREFIX##"Hello"); \
        adam::string_hashed_template<CHAR_TYPE> str_rt3(PREFIX##"World"); \
        EXPECT_EQ(str1.get_hash(), str2.get_hash()); \
        EXPECT_EQ(str1.get_hash(), str_rt1.get_hash()); \
        EXPECT_EQ(str1.get_hash(), str_rt2.get_hash()); \
        EXPECT_NE(str1.get_hash(), str3.get_hash()); \
        EXPECT_NE(str1.get_hash(), str_rt3.get_hash()); \
        EXPECT_TRUE(str1 == str2); \
        EXPECT_TRUE(str1 != str3); \
    } \
    /** @brief Tests the insertion and retrieval of string_hashed_ct_template objects in a std::map. */ \
    TEST(string_hashed_ct, map_insertion_and_retrieval_##TYPE_NAME) \
    { \
        std::unordered_map<adam::string_hashed_ct_template<CHAR_TYPE>, int> test_map; \
        constexpr adam::string_hashed_ct_template<CHAR_TYPE> key_1(PREFIX##"param1"); \
        test_map[key_1] = 5000; \
        EXPECT_EQ(test_map.count(key_1), 1u); \
        EXPECT_EQ(test_map[key_1], 5000); \
        constexpr adam::string_hashed_ct_template<CHAR_TYPE> key_2(PREFIX##"param1"); \
        EXPECT_EQ(test_map.count(key_2), 1u); \
        EXPECT_EQ(test_map[key_2], 5000); \
    } \
    /** @brief Tests the consistency of the standard library hash function with the custom hash function. */ \
    TEST(string_hashed_ct, std_hash_consistency_##TYPE_NAME) \
    { \
        constexpr adam::string_hashed_ct_template<CHAR_TYPE> sh(PREFIX##"test_string"); \
        std::hash<adam::string_hashed_ct_template<CHAR_TYPE>> hasher; \
        size_t system_hash = hasher(sh); \
        EXPECT_EQ(system_hash, static_cast<size_t>(sh.get_hash())); \
    } \
    /** @brief Tests heterogeneous lookup using uint64_t hash for compile-time strings. */ \
    TEST(string_hashed_ct, transparent_lookup_##TYPE_NAME) \
    { \
        std::unordered_map<adam::string_hashed_ct_template<CHAR_TYPE>, int> test_map; \
        constexpr adam::string_hashed_ct_template<CHAR_TYPE> key_1(PREFIX##"transparent_ct_test"); \
        test_map.emplace(key_1, 84); \
        \
        uint64_t raw_hash = key_1.get_hash(); \
        auto it = test_map.find(raw_hash); \
        ASSERT_NE(it, test_map.end()); \
        EXPECT_EQ(it->second, 84); \
        EXPECT_TRUE(test_map.contains(raw_hash)); \
        \
        adam::string_hashed_template<CHAR_TYPE> rt_key(PREFIX##"transparent_ct_test"); \
        auto it_rt = test_map.find(rt_key.get_hash()); \
        ASSERT_NE(it_rt, test_map.end()); \
        EXPECT_EQ(it_rt->second, 84); \
        EXPECT_TRUE(test_map.contains(rt_key.get_hash())); \
    }

GENERATE_STRING_HASHED_CT_TESTS(char, , char)
GENERATE_STRING_HASHED_CT_TESTS(wchar_t, L, wchar)
#if (defined(ADAM_PLATFORM_LINUX) && defined(_GLIBCXX_USE_CHAR8_T)) || defined(ADAM_PLATFORM_WINDOWS)
GENERATE_STRING_HASHED_CT_TESTS(char8_t, u8, char8)
#endif
GENERATE_STRING_HASHED_CT_TESTS(char16_t, u, char16)
GENERATE_STRING_HASHED_CT_TESTS(char32_t, U, char32)
