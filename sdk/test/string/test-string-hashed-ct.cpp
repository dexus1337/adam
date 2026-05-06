#include <gtest/gtest.h>
#include <string/string-hashed-ct.hpp>
#include <string/string-hashed.hpp>

#include <unordered_map>

/** @brief Tests the initialization of string_hashed_ct objects. */
TEST(string_hashed_ct, set)
{
    constexpr adam::string_hashed_ct str1("Hello");
    constexpr adam::string_hashed_ct str2("Hello");
    constexpr adam::string_hashed_ct str3("World");
    
    EXPECT_STREQ(str1.c_str(), str2.c_str());
    EXPECT_STRNE(str1.c_str(), str3.c_str());
}

/** @brief Tests the comparison of string_hashed_ct objects. */
TEST(string_hashed_ct, comparison)
{
    constexpr adam::string_hashed_ct str1("Hello");
    constexpr adam::string_hashed_ct str2("Hello");
    constexpr adam::string_hashed_ct str3("World");

    EXPECT_EQ(str1.get_hash(), str2.get_hash());
    EXPECT_NE(str1.get_hash(), str3.get_hash());

    EXPECT_EQ(str1, str2);
    EXPECT_NE(str1, str3);
}

/** @brief Tests the comparison of string_hashed_ct objects to runtime string_hashed. */
TEST(string_hashed_ct, comparison_runtime)
{
    constexpr adam::string_hashed_ct str1("Hello");
    constexpr adam::string_hashed_ct str2("Hello");
    constexpr adam::string_hashed_ct str3("World");

    adam::string_hashed str_rt1("Hello");
    adam::string_hashed str_rt2("Hello");
    adam::string_hashed str_rt3("World");

    EXPECT_EQ(str1.get_hash(), str2.get_hash());
    EXPECT_EQ(str1.get_hash(), str_rt1.get_hash());
    EXPECT_EQ(str1.get_hash(), str_rt2.get_hash());
    EXPECT_NE(str1.get_hash(), str3.get_hash());
    EXPECT_NE(str1.get_hash(), str_rt3.get_hash());

    EXPECT_EQ(str1, str2);
    EXPECT_NE(str1, str3);
}

/** @brief Tests the insertion and retrieval of string_hashed_ct objects in a std::map. */
TEST(string_hashed_ct, map_insertion_and_retrieval) 
{
    std::unordered_map<adam::string_hashed_ct, int> test_map;

    constexpr adam::string_hashed_ct key_1("param1");
    test_map[key_1] = 5000;

    EXPECT_EQ(test_map.count(key_1), 1u);
    EXPECT_EQ(test_map[key_1], 5000);

    // Separate object, same content
    constexpr adam::string_hashed_ct key_2("param1");

    EXPECT_EQ(test_map.count(key_2), 1u);
    EXPECT_EQ(test_map[key_2], 5000);
}

/** @brief Tests the consistency of the standard library hash function with the custom hash function. */
TEST(string_hashed_ct, std_hash_consistency) 
{
    constexpr adam::string_hashed_ct sh("test_string");
    
    std::hash<adam::string_hashed_ct> hasher;
    size_t system_hash = hasher(sh);

    EXPECT_EQ(system_hash, static_cast<size_t>(sh.get_hash()));
}
