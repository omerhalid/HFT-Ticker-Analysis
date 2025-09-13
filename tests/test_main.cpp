/**
 * @file test_main.cpp
 * @brief Main test runner for all unit tests
 * @author HFT Developer
 * @date 2024
 */

#include <gtest/gtest.h>

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
