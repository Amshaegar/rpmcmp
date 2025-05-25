// SPDX-License-Identifier: MIT

#include <rpmcmp.hpp>

#include <gtest/gtest.h>

/* ======================================== VER ======================================== */

class RpmVerIsValid : public ::testing::TestWithParam<std::tuple<std::string, std::string>> {};
INSTANTIATE_TEST_SUITE_P(RpmVerIsValidValues,
                         RpmVerIsValid,
                         testing::Values(
                            std::make_tuple("1.2.3", ""),
                            std::make_tuple("1.2.3-", "Label can't have hyphen symbol!")
                         ));

TEST_P(RpmVerIsValid, RpmVerIsValidCheck) {
    // Arrange
    auto [version, expectedIsValidResult] = GetParam();

    // Act
    std::string actualIsValidResult = rpmcmplib::RpmVer::isValid(version);

    // Assert
    EXPECT_EQ(actualIsValidResult, expectedIsValidResult);
}

TEST(RpmCmp, VersionReleaseCantHaveHyphenSymbolConstructor) {
    // Arrange
    std::string result;
    
    // Act
    try {
        rpmcmplib::RpmVer ver = rpmcmplib::RpmVer("1.2.3-");
    } catch(const std::exception& e) {
        result = e.what();
    }

    // Assert
    EXPECT_EQ(result, std::string("Label can't have hyphen symbol!"));
}

TEST(RpmCmp, VersionReleaseCantHaveHyphenSymbolComparison) {
    // Arrange
    std::string result;
    int cmpResult = -2;

    // Act
    try {
        cmpResult = rpmcmplib::RpmVer::cmp("1.2.3-", "1.2.3");
    } catch(const std::exception& e) {
        result = e.what();
    }

    // Assert
    EXPECT_EQ(result, std::string("Label can't have hyphen symbol!"));
    EXPECT_EQ(cmpResult, -2);
}

TEST(RpmCmp, RpmVerSegments) {
    // Arrange
    std::string version = "1.002.3.abc.001ab.dd100";
    std::vector<std::string> expectedSegments = {"1","002","3","abc","001","ab", "dd", "100"};

    // Act
    std::vector<std::string> actualSegments = rpmcmplib::RpmVer::segments(version);

    // Assert
    EXPECT_EQ(expectedSegments, actualSegments);
}

TEST(RpmCmp, RpmVerCmpFuncLhsLowerRhs) {
    EXPECT_EQ(rpmcmplib::RpmVer::cmp("1.0",         "1.1"), -1) << "0 < 1";
    EXPECT_EQ(rpmcmplib::RpmVer::cmp("1.1",         "1.2.3"), -1) << "1 < 2";
    EXPECT_EQ(rpmcmplib::RpmVer::cmp("1.0a",        "1.0b"), -1) << "a < b";
    EXPECT_EQ(rpmcmplib::RpmVer::cmp("2.5",         "2.50"), -1) << "5 < 50";
    EXPECT_EQ(rpmcmplib::RpmVer::cmp("1.9",         "1.0010"), -1) << "9 < 10 - ignore leading zeroes";
    EXPECT_EQ(rpmcmplib::RpmVer::cmp("2.1.7A",      "2.1.7a"), -1) << "lexicographical comparison of the 'A' VS 'a': 'A' (ASCII 65) < 'a' (ASCII 97)";
    EXPECT_EQ(rpmcmplib::RpmVer::cmp("2a",          "2.0"), -1) << "numbers are considered newer than letters";
    EXPECT_EQ(rpmcmplib::RpmVer::cmp("0.5.0.post1", "0.5.0.1"), -1) << "numeric element 1 sorts higher than alphabetic element post";
    EXPECT_EQ(rpmcmplib::RpmVer::cmp("0.5.0.post1", "0.5.1"), -1) << "0 < 1";
    EXPECT_EQ(rpmcmplib::RpmVer::cmp("1.0",         "1.0a"), -1) << "rhs has one more element in the list, while previous elements are equal";
    EXPECT_EQ(rpmcmplib::RpmVer::cmp("1",           "1.0"), -1) << "rhs has one more element in the list, while previous elements are equal";
    EXPECT_EQ(rpmcmplib::RpmVer::cmp("1.1~201601",  "1.1"), -1) << "~ before version component means that version with it is earlier than version without it";
    EXPECT_EQ(rpmcmplib::RpmVer::cmp("1.1",         "1.1^201601"), -1) << "^ before version component means that version with it is later than version without it";
}

TEST(RpmCmp, RpmVerCmpFuncLhsEqualRhs) {
    EXPECT_EQ(rpmcmplib::RpmVer::cmp("1.05",     "1.5"), 0) << "both 05 and 5 are treated as the number 5";
    EXPECT_EQ(rpmcmplib::RpmVer::cmp("1.5",      "1.05"), 0) << "both 05 and 5 are treated as the number 5";
    EXPECT_EQ(rpmcmplib::RpmVer::cmp("fc4",      "fc.4"), 0) << "the alphabetic and numeric sections will always get separated into different elements anyway";
    EXPECT_EQ(rpmcmplib::RpmVer::cmp("fc.4",     "fc4"), 0) << "the alphabetic and numeric sections will always get separated into different elements anyway";
    EXPECT_EQ(rpmcmplib::RpmVer::cmp("3.0.0_fc", "3.0.0.fc"), 0) << "the separators themselves are not important";
    EXPECT_EQ(rpmcmplib::RpmVer::cmp("3.0.0.fc", "3.0.0_fc"), 0) << "the separators themselves are not important";
}

TEST(RpmCmp, RpmVerCmpFuncLhsHigherRhs) {
    EXPECT_EQ(rpmcmplib::RpmVer::cmp("1.1",        "1.0"), 1) << "1 > 0";
    EXPECT_EQ(rpmcmplib::RpmVer::cmp("1.2.3",      "1.1"), 1) << "2 > 1";
    EXPECT_EQ(rpmcmplib::RpmVer::cmp("1.0b",       "1.0a"), 1) << "b > a";
    EXPECT_EQ(rpmcmplib::RpmVer::cmp("2.50",       "2.5"), 1) << "50 > 5";
    EXPECT_EQ(rpmcmplib::RpmVer::cmp("1.0010",     "1.9"), 1) << "10 > 9  - ignore leading zeroes";
    EXPECT_EQ(rpmcmplib::RpmVer::cmp("2.1.7a",     "2.1.7A"), 1) << "lexicographical comparison of the 'a' VS 'A': 'a' (ASCII 97) > 'A' (ASCII 65)";
    EXPECT_EQ(rpmcmplib::RpmVer::cmp("2.0",        "2a"), 1) << "numbers are considered newer than letters";
    EXPECT_EQ(rpmcmplib::RpmVer::cmp("0.5.0.1",    "0.5.0.post1"), 1) << "numeric element 1 sorts higher than alphabetic element post";
    EXPECT_EQ(rpmcmplib::RpmVer::cmp("0.5.1",      "0.5.0.post1"), 1) << "1 > 0";
    EXPECT_EQ(rpmcmplib::RpmVer::cmp("1.0a",       "1.0"), 1) << "lhs has one more element in the list, while previous elements are equal";
    EXPECT_EQ(rpmcmplib::RpmVer::cmp("1.0",        "1"), 1) << "lhs has one more element in the list, while previous elements are equal";
    EXPECT_EQ(rpmcmplib::RpmVer::cmp("1.1",        "1.1~201601"), 1) << "~ before version component means that version with it is earlier than version without it";
    EXPECT_EQ(rpmcmplib::RpmVer::cmp("1.1^201601", "1.1"), 1) << "^ before version component means that version with it is later than version without it";
}

TEST(RpmCmp, RpmVerCmpObjLhsLowerRhs) {
    EXPECT_TRUE(rpmcmplib::RpmVer("1.0")         < rpmcmplib::RpmVer("1.1")) << "0 < 1";
    EXPECT_TRUE(rpmcmplib::RpmVer("1.1")         < rpmcmplib::RpmVer("1.2.3")) << "1 < 2";
    EXPECT_TRUE(rpmcmplib::RpmVer("1.0a")        < rpmcmplib::RpmVer("1.0b")) << "a < b";
    EXPECT_TRUE(rpmcmplib::RpmVer("2.5")         < rpmcmplib::RpmVer("2.50")) << "5 < 50";
    EXPECT_TRUE(rpmcmplib::RpmVer("1.9")         < rpmcmplib::RpmVer("1.0010")) << "9 < 10 - ignore leading zeroes";
    EXPECT_TRUE(rpmcmplib::RpmVer("2.1.7A")      < rpmcmplib::RpmVer("2.1.7a")) << "lexicographical comparison of the 'A' VS 'a': 'A' (ASCII 65) < 'a' (ASCII 97)";
    EXPECT_TRUE(rpmcmplib::RpmVer("2a")          < rpmcmplib::RpmVer("2.0")) << "numbers are considered newer than letters";
    EXPECT_TRUE(rpmcmplib::RpmVer("0.5.0.post1") < rpmcmplib::RpmVer("0.5.0.1")) << "numeric element 1 sorts higher than alphabetic element post";
    EXPECT_TRUE(rpmcmplib::RpmVer("0.5.0.post1") < rpmcmplib::RpmVer("0.5.1")) << "0 < 1";
    EXPECT_TRUE(rpmcmplib::RpmVer("1.0")         < rpmcmplib::RpmVer("1.0a")) << "rhs has one more element in the list, while previous elements are equal";
    EXPECT_TRUE(rpmcmplib::RpmVer("1")           < rpmcmplib::RpmVer("1.0")) << "rhs has one more element in the list, while previous elements are equal";
    EXPECT_TRUE(rpmcmplib::RpmVer("1.1~201601")  < rpmcmplib::RpmVer("1.1")) << "~ before version component means that version with it is earlier than version without it";
    EXPECT_TRUE(rpmcmplib::RpmVer("1.1")         < rpmcmplib::RpmVer("1.1^201601")) << "^ before version component means that version with it is later than version without it";
}

TEST(RpmCmp, RpmVerCmpObjLhsEqualRhs) {
    EXPECT_TRUE(rpmcmplib::RpmVer("1.05")     == rpmcmplib::RpmVer("1.5")) << "both 05 and 5 are treated as the number 5";
    EXPECT_TRUE(rpmcmplib::RpmVer("1.5")      == rpmcmplib::RpmVer("1.05")) << "both 05 and 5 are treated as the number 5";
    EXPECT_TRUE(rpmcmplib::RpmVer("fc4")      == rpmcmplib::RpmVer("fc.4")) << "the alphabetic and numeric sections will always get separated into different elements anyway";
    EXPECT_TRUE(rpmcmplib::RpmVer("fc.4")     == rpmcmplib::RpmVer("fc4")) << "the alphabetic and numeric sections will always get separated into different elements anyway";
    EXPECT_TRUE(rpmcmplib::RpmVer("3.0.0_fc") == rpmcmplib::RpmVer("3.0.0.fc")) << "the separators themselves are not important";
    EXPECT_TRUE(rpmcmplib::RpmVer("3.0.0.fc") == rpmcmplib::RpmVer("3.0.0_fc")) << "the separators themselves are not important";
}

TEST(RpmCmp, RpmVerCmpObjLhsHigherRhs) {
    EXPECT_TRUE(rpmcmplib::RpmVer("1.1")        > rpmcmplib::RpmVer("1.0")) << "1 > 0";
    EXPECT_TRUE(rpmcmplib::RpmVer("1.2.3")      > rpmcmplib::RpmVer("1.1")) << "2 > 1";
    EXPECT_TRUE(rpmcmplib::RpmVer("1.0b")       > rpmcmplib::RpmVer("1.0a")) << "b > a";
    EXPECT_TRUE(rpmcmplib::RpmVer("2.50")       > rpmcmplib::RpmVer("2.5")) << "50 > 5";
    EXPECT_TRUE(rpmcmplib::RpmVer("1.0010")     > rpmcmplib::RpmVer("1.9")) << "10 > 9  - ignore leading zeroes";
    EXPECT_TRUE(rpmcmplib::RpmVer("2.1.7a")     > rpmcmplib::RpmVer("2.1.7A")) << "lexicographical comparison of the 'a' VS 'A': 'a' (ASCII 97) > 'A' (ASCII 65)";
    EXPECT_TRUE(rpmcmplib::RpmVer("2.0")        > rpmcmplib::RpmVer("2a")) << "numbers are considered newer than letters";
    EXPECT_TRUE(rpmcmplib::RpmVer("0.5.0.1")    > rpmcmplib::RpmVer("0.5.0.post1")) << "numeric element 1 sorts higher than alphabetic element post";
    EXPECT_TRUE(rpmcmplib::RpmVer("0.5.1")      > rpmcmplib::RpmVer("0.5.0.post1")) << "1 > 0";
    EXPECT_TRUE(rpmcmplib::RpmVer("1.0a")       > rpmcmplib::RpmVer("1.0")) << "lhs has one more element in the list, while previous elements are equal";
    EXPECT_TRUE(rpmcmplib::RpmVer("1.0")        > rpmcmplib::RpmVer("1")) << "lhs has one more element in the list, while previous elements are equal";
    EXPECT_TRUE(rpmcmplib::RpmVer("1.1")        > rpmcmplib::RpmVer("1.1~201601")) << "~ before version component means that version with it is earlier than version without it";
    EXPECT_TRUE(rpmcmplib::RpmVer("1.1^201601") > rpmcmplib::RpmVer("1.1")) << "^ before version component means that version with it is later than version without it";
}

/* ======================================== EVR ======================================== */

class RpmEvrIsValid : public ::testing::TestWithParam<std::tuple<std::string, std::string>> {};
INSTANTIATE_TEST_SUITE_P(RpmEvrIsValidValues,
                         RpmEvrIsValid,
                         testing::Values(
                            std::make_tuple("1.2.3-a", ""),
                            std::make_tuple("1.2.3-a-", "EVR must contain only one hyphen symbol!"),
                            std::make_tuple("1:1.2.3-a", ""),
                            std::make_tuple("1:1.2.3-a:", "EVR must contain only one colon symbol!"),
                            std::make_tuple("1:1.2.3", ""),
                            std::make_tuple("0:1.2.3", ""),
                            std::make_tuple("-1:1.2.3", "Epoch must be a positive number!")
                         ));

TEST_P(RpmEvrIsValid, RpmEvrIsValidCheck) {
    // Arrange
    auto [version, expectedIsValidResult] = GetParam();

    // Act
    std::string actualIsValidResult = rpmcmplib::RpmEvr::isValid(version);

    // Assert
    EXPECT_EQ(actualIsValidResult, expectedIsValidResult);
}

TEST(RpmCmp, EvrMustContainOnlyOneHyphenSymbolConstructor) {
    // Arrange
    std::string result;

    // Act
    try {
        rpmcmplib::RpmEvr evr = rpmcmplib::RpmEvr("1.2.3-a-");
    } catch(const std::exception& e) {
        result = e.what();
    }

    // Assert
    EXPECT_EQ(result, std::string("EVR must contain only one hyphen symbol!"));
}

TEST(RpmCmp, EvrMustContainOnlyOneHyphenSymbolComparison) {
    // Arrange
    std::string result;
    int cmpResult = -2;

    // Act
    try {
        cmpResult = rpmcmplib::RpmEvr::cmp("1.2.3-a-", "1.2.3-a");
    } catch(const std::exception& e) {
        result = e.what();
    }

    // Assert
    EXPECT_EQ(result, std::string("EVR must contain only one hyphen symbol!"));
    EXPECT_EQ(cmpResult, -2);
}

TEST(RpmCmp, EvrMustContainOnlyOneColonSymbolConstructor) {
    // Arrange
    std::string result;

    // Act
    try {
        rpmcmplib::RpmEvr evr = rpmcmplib::RpmEvr("1:1.2.3-a:");
    } catch(const std::exception& e) {
        result = e.what();
    }

    // Assert
    EXPECT_EQ(result, std::string("EVR must contain only one colon symbol!"));
}

TEST(RpmCmp, EvrMustContainOnlyOneColonSymbolComparison) {
    // Arrange
    std::string result;
    int cmpResult = -2;

    // Act
    try {
        cmpResult = rpmcmplib::RpmEvr::cmp("1:1.2.3-a:", "1:1.2.3-a");
    } catch(const std::exception& e) {
        result = e.what();
    }

    // Assert
    EXPECT_EQ(result, std::string("EVR must contain only one colon symbol!"));
    EXPECT_EQ(cmpResult, -2);
}

TEST(RpmCmp, EvrEpochMustBeAPositiveNumberConstructor) {
    // Arrange
    std::string result;

    // Act
    try {
        rpmcmplib::RpmEvr evr = rpmcmplib::RpmEvr("-1:1.2.3");
    } catch(const std::exception& e) {
        result = e.what();
    }

    // Assert
    EXPECT_EQ(result, std::string("Epoch must be a positive number!"));
}

TEST(RpmCmp, EvrEpochMustBeAPositiveNumberComparison) {
    // Arrange
    std::string result;
    int cmpResult = -2;

    // Act
    try {
        cmpResult = rpmcmplib::RpmEvr::cmp("-1:1.2.3", "1:1.2.3");
    } catch(const std::exception& e) {
        result = e.what();
    }

    // Assert
    EXPECT_EQ(result, std::string("Epoch must be a positive number!"));
    EXPECT_EQ(cmpResult, -2);
}

class RpmCmpEvrSplit : public ::testing::TestWithParam<std::tuple<std::string, unsigned long long, std::string, std::string>> {};
INSTANTIATE_TEST_SUITE_P(RpmCmpEvrSplitValues,
                         RpmCmpEvrSplit,
                         testing::Values(
                            std::make_tuple("1:1.2.3-1", 1, "1.2.3", "1"),
                            std::make_tuple("999:1.2.3.4.5.6-1", 999, "1.2.3.4.5.6", "1"),
                            std::make_tuple("009:1.2.3.4.5.6-a.b.c.d.e.f", 9, "1.2.3.4.5.6", "a.b.c.d.e.f"),
                            std::make_tuple("1:1.2.3", 1, "1.2.3", ""),
                            std::make_tuple("999:1.2.3.4.5.6", 999, "1.2.3.4.5.6", ""),
                            std::make_tuple("009:1.2.3.4.5.6", 9, "1.2.3.4.5.6", ""),
                            std::make_tuple("1.2.3", 0, "1.2.3", ""),
                            std::make_tuple("1.2.3.4.5.6", 0, "1.2.3.4.5.6", "")
                         ));

TEST_P(RpmCmpEvrSplit, RpmCmpEvrSplitCheck) {
    // Arrange
    auto [evrValue, expectedEpoch, expectedVersion, expectedRelease] = GetParam();

    // Act
    rpmcmplib::RpmEvr evr = rpmcmplib::RpmEvr(evrValue);

    // Assert
    EXPECT_EQ(evr.epoch(), expectedEpoch);
    EXPECT_EQ(evr.version(), expectedVersion);
    EXPECT_EQ(evr.release(), expectedRelease);
}

TEST(RpmCmp, RpmEvrCmpFuncLhsLowerRhs) {
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("0:1.2.3-1",   "1:1.2.3-1"), -1) << "0 epoch < 1 epoch, other is equal";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("0:1.2.3-1",   "1:foo.bar-1"), -1) << "0 epoch < 1 epoch";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("0:1.2.3",     "1:foo.bar"), -1) << "0 epoch < 1 epoch";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("0:3",         "1:2"), -1) << "0 epoch < 1 epoch";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("1.2.3-1",     "1:1.2.3-1"), -1) << "if there is no epoch than it's equal 0, other is equal";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("1.2.3-1",     "1:foo.bar-1"), -1) << "if there is no epoch than it's equal 0";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("1.2.3",       "1:foo.bar"), -1) << "if there is no epoch than it's equal 0";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("3",           "1:2"), -1) << "if there is no epoch than it's equal 0";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("888:1.2.3-1", "999:foo.bar-1"), -1) << "888 < 999";
    
    // if there is no epoch than it's equal 0, compare other parts
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("1.0",         "1.1"), -1) << "0 < 1";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("1.1",         "1.2.3"), -1) << "1 < 2";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("1.0a",        "1.0b"), -1) << "a < b";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("2.5",         "2.50"), -1) << "5 < 50";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("1.9",         "1.0010"), -1) << "9 < 10 - ignore leading zeroes";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("2.1.7A",      "2.1.7a"), -1) << "lexicographical comparison of the 'A' VS 'a': 'A' (ASCII 65) < 'a' (ASCII 97)";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("2a",          "2.0"), -1) << "numbers are considered newer than letters";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("0.5.0.post1", "0.5.0.1"), -1) << "numeric element 1 sorts higher than alphabetic element post";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("0.5.0.post1", "0.5.1"), -1) << "0 < 1";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("1.0",         "1.0a"), -1) << "rhs has one more element in the list, while previous elements are equal";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("1",           "1.0"), -1) << "rhs has one more element in the list, while previous elements are equal";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("1.1~201601",  "1.1"), -1) << "~ before version component means that version with it is earlier than version without it";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("1.1",         "1.1^201601"), -1) << "^ before version component means that version with it is later than version without it";
}

TEST(RpmCmp, RpmEvrCmpFuncLhsEqualRhs) {
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("1:1.2.3-1", "1:1.2.3-1"), 0);
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("1.2.3-1",   "1.2.3-1"), 0);

    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("3.0.0_fc", "3.0.0.fc"), 0)
        << "if there is no epoch than it's equal 0, compare other parts: the separators themselves are not important";

    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("3.0.0.fc", "3.0.0_fc"), 0)
        << "if there is no epoch than it's equal 0, compare other parts: the separators themselves are not important";

    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("3.0.0_fc-3.0.0_fc", "3.0.0.fc-3.0.0.fc"), 0)
        << "if there is no epoch than it's equal 0, compare other parts: the separators themselves are not important";

    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("3.0.0.fc-3.0.0.fc", "3.0.0_fc-3.0.0_fc"), 0)
        << "if there is no epoch than it's equal 0, compare other parts: the separators themselves are not important";
}

TEST(RpmCmp, RpmEvrCmpFuncLhsHigherRhs) {
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("1:1.2.3-1",     "0:1.2.3-1"), 1) << "1 epoch > 0 epoch, other is equal";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("1:foo.bar-1",   "0:1.2.3-1"), 1) << "1 epoch > 0 epoch";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("1:foo.bar",     "0:1.2.3"), 1) << "1 epoch > 0 epoch";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("1:2",           "0:3"), 1) << "1 epoch > 0 epoch";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("1:1.2.3-1",     "1.2.3-1"), 1) << "if there is no epoch than it's equal 0, other is equal";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("1:foo.bar-1",   "1.2.3-1"), 1) << "if there is no epoch than it's equal 0";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("1:foo.bar",     "1.2.3"), 1) << "if there is no epoch than it's equal 0";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("1:2",           "3"), 1) << "if there is no epoch than it's equal 0";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("999:foo.bar-1", "888:1.2.3-1"), 1) << "999 > 888";
    
    // if there is no epoch than it's equal 0, compare other parts
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("1.1",        "1.0"), 1) << "1 > 0";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("1.2.3",      "1.1"), 1) << "2 > 1";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("1.0b",       "1.0a"), 1) << "b > a";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("2.50",       "2.5"), 1) << "50 > 5";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("1.0010",     "1.9"), 1) << "10 > 9  - ignore leading zeroes";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("2.1.7a",     "2.1.7A"), 1) << "lexicographical comparison of the 'a' VS 'A': 'a' (ASCII 97) > 'A' (ASCII 65)";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("2.0",        "2a"), 1) << "numbers are considered newer than letters";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("0.5.0.1",    "0.5.0.post1"), 1) << "numeric element 1 sorts higher than alphabetic element post";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("0.5.1",      "0.5.0.post1"), 1) << "1 > 0";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("1.0a",       "1.0"), 1) << "lhs has one more element in the list, while previous elements are equal";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("1.0",        "1"), 1) << "lhs has one more element in the list, while previous elements are equal";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("1.1",        "1.1~201601"), 1) << "~ before version component means that version with it is earlier than version without it";
    EXPECT_EQ(rpmcmplib::RpmEvr::cmp("1.1^201601", "1.1"), 1) << "^ before version component means that version with it is later than version without it";
}

TEST(RpmCmp, RpmEvrCmpObjLhsLowerRhs) {
    EXPECT_TRUE(rpmcmplib::RpmEvr("0:1.2.3-1")   < rpmcmplib::RpmEvr("1:1.2.3-1")) << "0 epoch < 1 epoch, other is equal";
    EXPECT_TRUE(rpmcmplib::RpmEvr("0:1.2.3-1")   < rpmcmplib::RpmEvr("1:foo.bar-1")) << "0 epoch < 1 epoch";
    EXPECT_TRUE(rpmcmplib::RpmEvr("0:1.2.3")     < rpmcmplib::RpmEvr("1:foo.bar")) << "0 epoch < 1 epoch";
    EXPECT_TRUE(rpmcmplib::RpmEvr("0:3")         < rpmcmplib::RpmEvr("1:2")) << "0 epoch < 1 epoch";
    EXPECT_TRUE(rpmcmplib::RpmEvr("1.2.3-1")     < rpmcmplib::RpmEvr("1:1.2.3-1")) << "if there is no epoch than it's equal 0, other is equal";
    EXPECT_TRUE(rpmcmplib::RpmEvr("1.2.3-1")     < rpmcmplib::RpmEvr("1:foo.bar-1")) << "if there is no epoch than it's equal 0";
    EXPECT_TRUE(rpmcmplib::RpmEvr("1.2.3")       < rpmcmplib::RpmEvr("1:foo.bar")) << "if there is no epoch than it's equal 0";
    EXPECT_TRUE(rpmcmplib::RpmEvr("3")           < rpmcmplib::RpmEvr("1:2")) << "if there is no epoch than it's equal 0";
    EXPECT_TRUE(rpmcmplib::RpmEvr("888:1.2.3-1") < rpmcmplib::RpmEvr("999:foo.bar-1")) << "888 < 999";
    
    // if there is no epoch than it's equal 0, compare other parts
    EXPECT_TRUE(rpmcmplib::RpmEvr("1.0")         < rpmcmplib::RpmEvr("1.1")) << "0 < 1";
    EXPECT_TRUE(rpmcmplib::RpmEvr("1.1")         < rpmcmplib::RpmEvr("1.2.3")) << "1 < 2";
    EXPECT_TRUE(rpmcmplib::RpmEvr("1.0a")        < rpmcmplib::RpmEvr("1.0b")) << "a < b";
    EXPECT_TRUE(rpmcmplib::RpmEvr("2.5")         < rpmcmplib::RpmEvr("2.50")) << "5 < 50";
    EXPECT_TRUE(rpmcmplib::RpmEvr("1.9")         < rpmcmplib::RpmEvr("1.0010")) << "9 < 10 - ignore leading zeroes";
    EXPECT_TRUE(rpmcmplib::RpmEvr("2.1.7A")      < rpmcmplib::RpmEvr("2.1.7a")) << "lexicographical comparison of the 'A' VS 'a': 'A' (ASCII 65) < 'a' (ASCII 97)";
    EXPECT_TRUE(rpmcmplib::RpmEvr("2a")          < rpmcmplib::RpmEvr("2.0")) << "numbers are considered newer than letters";
    EXPECT_TRUE(rpmcmplib::RpmEvr("0.5.0.post1") < rpmcmplib::RpmEvr("0.5.0.1")) << "numeric element 1 sorts higher than alphabetic element post";
    EXPECT_TRUE(rpmcmplib::RpmEvr("0.5.0.post1") < rpmcmplib::RpmEvr("0.5.1")) << "0 < 1";
    EXPECT_TRUE(rpmcmplib::RpmEvr("1.0")         < rpmcmplib::RpmEvr("1.0a")) << "rhs has one more element in the list, while previous elements are equal";
    EXPECT_TRUE(rpmcmplib::RpmEvr("1")           < rpmcmplib::RpmEvr("1.0")) << "rhs has one more element in the list, while previous elements are equal";
    EXPECT_TRUE(rpmcmplib::RpmEvr("1.1~201601")  < rpmcmplib::RpmEvr("1.1")) << "~ before version component means that version with it is earlier than version without it";
    EXPECT_TRUE(rpmcmplib::RpmEvr("1.1")         < rpmcmplib::RpmEvr("1.1^201601")) << "^ before version component means that version with it is later than version without it";
}

TEST(RpmCmp, RpmEvrCmpObjLhsEqualRhs) {
    EXPECT_TRUE(rpmcmplib::RpmEvr("1:1.2.3-1") == rpmcmplib::RpmEvr("1:1.2.3-1"));
    EXPECT_TRUE(rpmcmplib::RpmEvr("1.2.3-1")   == rpmcmplib::RpmEvr("1.2.3-1"));

    EXPECT_TRUE(rpmcmplib::RpmEvr("3.0.0_fc") == rpmcmplib::RpmEvr("3.0.0.fc"))
        << "if there is no epoch than it's equal 0, compare other parts: the separators themselves are not important";

    EXPECT_TRUE(rpmcmplib::RpmEvr("3.0.0.fc") == rpmcmplib::RpmEvr("3.0.0_fc"))
        << "if there is no epoch than it's equal 0, compare other parts: the separators themselves are not important";

    EXPECT_TRUE(rpmcmplib::RpmEvr("3.0.0_fc-3.0.0_fc") == rpmcmplib::RpmEvr("3.0.0.fc-3.0.0.fc"))
        << "if there is no epoch than it's equal 0, compare other parts: the separators themselves are not important";

    EXPECT_TRUE(rpmcmplib::RpmEvr("3.0.0.fc-3.0.0.fc") == rpmcmplib::RpmEvr("3.0.0_fc-3.0.0_fc"))
        << "if there is no epoch than it's equal 0, compare other parts: the separators themselves are not important";
}

TEST(RpmCmp, RpmEvrCmpObjLhsHigherRhs) {
    EXPECT_TRUE(rpmcmplib::RpmEvr("1:1.2.3-1")     > rpmcmplib::RpmEvr("0:1.2.3-1")) << "1 epoch > 0 epoch, other is equal";
    EXPECT_TRUE(rpmcmplib::RpmEvr("1:foo.bar-1")   > rpmcmplib::RpmEvr("0:1.2.3-1")) << "1 epoch > 0 epoch";
    EXPECT_TRUE(rpmcmplib::RpmEvr("1:foo.bar")     > rpmcmplib::RpmEvr("0:1.2.3")) << "1 epoch > 0 epoch";
    EXPECT_TRUE(rpmcmplib::RpmEvr("1:2")           > rpmcmplib::RpmEvr("0:3")) << "1 epoch > 0 epoch";
    EXPECT_TRUE(rpmcmplib::RpmEvr("1:1.2.3-1")     > rpmcmplib::RpmEvr("1.2.3-1")) << "if there is no epoch than it's equal 0, other is equal";
    EXPECT_TRUE(rpmcmplib::RpmEvr("1:foo.bar-1")   > rpmcmplib::RpmEvr("1.2.3-1")) << "if there is no epoch than it's equal 0";
    EXPECT_TRUE(rpmcmplib::RpmEvr("1:foo.bar")     > rpmcmplib::RpmEvr("1.2.3")) << "if there is no epoch than it's equal 0";
    EXPECT_TRUE(rpmcmplib::RpmEvr("1:2")           > rpmcmplib::RpmEvr("3")) << "if there is no epoch than it's equal 0";
    EXPECT_TRUE(rpmcmplib::RpmEvr("999:foo.bar-1") > rpmcmplib::RpmEvr("888:1.2.3-1")) << "999 > 888";
    
    // if there is no epoch than it's equal 0, compare other parts
    EXPECT_TRUE(rpmcmplib::RpmEvr("1.1")        > rpmcmplib::RpmEvr("1.0")) << "1 > 0";
    EXPECT_TRUE(rpmcmplib::RpmEvr("1.2.3")      > rpmcmplib::RpmEvr("1.1")) << "2 > 1";
    EXPECT_TRUE(rpmcmplib::RpmEvr("1.0b")       > rpmcmplib::RpmEvr("1.0a")) << "b > a";
    EXPECT_TRUE(rpmcmplib::RpmEvr("2.50")       > rpmcmplib::RpmEvr("2.5")) << "50 > 5";
    EXPECT_TRUE(rpmcmplib::RpmEvr("1.0010")     > rpmcmplib::RpmEvr("1.9")) << "10 > 9  - ignore leading zeroes";
    EXPECT_TRUE(rpmcmplib::RpmEvr("2.1.7a")     > rpmcmplib::RpmEvr("2.1.7A")) << "lexicographical comparison of the 'a' VS 'A': 'a' (ASCII 97) > 'A' (ASCII 65)";
    EXPECT_TRUE(rpmcmplib::RpmEvr("2.0")        > rpmcmplib::RpmEvr("2a")) << "numbers are considered newer than letters";
    EXPECT_TRUE(rpmcmplib::RpmEvr("0.5.0.1")    > rpmcmplib::RpmEvr("0.5.0.post1")) << "numeric element 1 sorts higher than alphabetic element post";
    EXPECT_TRUE(rpmcmplib::RpmEvr("0.5.1")      > rpmcmplib::RpmEvr("0.5.0.post1")) << "1 > 0";
    EXPECT_TRUE(rpmcmplib::RpmEvr("1.0a")       > rpmcmplib::RpmEvr("1.0")) << "lhs has one more element in the list, while previous elements are equal";
    EXPECT_TRUE(rpmcmplib::RpmEvr("1.0")        > rpmcmplib::RpmEvr("1")) << "lhs has one more element in the list, while previous elements are equal";
    EXPECT_TRUE(rpmcmplib::RpmEvr("1.1")        > rpmcmplib::RpmEvr("1.1~201601")) << "~ before version component means that version with it is earlier than version without it";
    EXPECT_TRUE(rpmcmplib::RpmEvr("1.1^201601") > rpmcmplib::RpmEvr("1.1")) << "^ before version component means that version with it is later than version without it";
}
