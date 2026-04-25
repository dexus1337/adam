#include <gtest/gtest.h>
#include <data/format/data-format.hpp>

TEST(data_format, transparent)
{
    EXPECT_STREQ("transparent", adam::data_format_transparent.get_name().c_str());

    EXPECT_EQ(adam::data_format_transparent.get_parser(), nullptr);
    EXPECT_EQ(adam::data_format_transparent.get_serializer(), nullptr);
}