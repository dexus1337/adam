#include <gtest/gtest.h>
#include <string/string-hashed.hpp>

/** @brief Tests the initialization of string_hashed objects. */
TEST(string_hashed, set)
{
    adam::string_hashed str1("Hello");
    adam::string_hashed str2("Hello");
    adam::string_hashed str3("World");

    EXPECT_STREQ(str1.c_str(), str2.c_str());
    EXPECT_STRNE(str1.c_str(), str3.c_str());
}

/** @brief Tests the comparison of string_hashed objects. */
TEST(string_hashed, comparison)
{
    adam::string_hashed str1("Hello");
    adam::string_hashed str2("Hello");
    adam::string_hashed str3("World");

    EXPECT_EQ(str1.get_hash(), str2.get_hash());
    EXPECT_NE(str1.get_hash(), str3.get_hash());

    EXPECT_EQ(str1, str2);
    EXPECT_NE(str1, str3);
}

/** @brief Tests the recalculation of hash values after modification. */
TEST(string_hashed, hash_recalculation)
{
    adam::string_hashed str1("Hello");
    adam::string_hashed str2("Hello");
    adam::string_hashed str3("Hello!");

    str2.append("!");

    EXPECT_NE(str2, str3);

    str2.calculate_hash();

    EXPECT_EQ(str2, str3);
}

/** @brief Tests the consistency of the standard library hash function with the custom hash function. */
TEST(string_hashed, copy) 
{
    adam::string_hashed sh("test_string");
    
    adam::string_hashed sh_copy("different_string");

    EXPECT_NE(sh_copy, sh);

    sh_copy = sh;

    EXPECT_EQ(sh_copy, sh);

    std::string std_str_copy = sh_copy;

    EXPECT_EQ(std_str_copy, sh);
}

/** @brief Tests the insertion and retrieval of string_hashed objects in a std::map. */
TEST(string_hashed, map_insertion_and_retrieval) 
{
    std::unordered_map<adam::string_hashed, int> test_map;

    adam::string_hashed key_1("param1");
    test_map[key_1] = 5000;

    EXPECT_EQ(test_map.count(key_1), 1u);
    EXPECT_EQ(test_map[key_1], 5000);

    // Separate object, same content
    adam::string_hashed key_2("param1");

    EXPECT_EQ(test_map.count(key_2), 1u);
    EXPECT_EQ(test_map[key_2], 5000);
}

/** @brief Tests the consistency of the standard library hash function with the custom hash function. */
TEST(string_hashed, std_hash_consistency) 
{
    adam::string_hashed sh("test_string");
    
    std::hash<adam::string_hashed> hasher;
    size_t system_hash = hasher(sh);

    EXPECT_EQ(system_hash, static_cast<size_t>(sh.get_hash()));
}