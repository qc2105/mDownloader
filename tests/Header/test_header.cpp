#include <iostream>
#include <gtest/gtest.h>
#include "header.h"

using namespace std;
using ::testing::InitGoogleTest;
using ::testing::Test;


TEST(TestHeader, test_set_and_get_attr)
{
    HeadData hd;

    hd.set_attr("name", "mDownloader");

    const char* value = hd.get_attr("name");

    ASSERT_STREQ(value, "mDownloader");

    hd.set_attr("language", "C++");

    value = hd.get_attr("language");

    ASSERT_STREQ(value, "C++");
}
