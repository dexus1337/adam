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