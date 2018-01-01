#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "../play.src/play.h"


TEST_CASE("Trimming empty strings", "[trim]") {
    REQUIRE(trim("") == "");
}

TEST_CASE("Trimming strings with only whitespace", "[trim]") {
    REQUIRE(trim("     ") == "");
}

TEST_CASE("Trimming string without leading/trailing space", "[trim]") {
    REQUIRE(trim("hello there") == "hello there");
}

TEST_CASE("Trimming string with extra space", "[trim]") {
    REQUIRE(trim("   hello there   ") == "hello there");
}



TEST_CASE("UpperFirsting empty strings", "[toUpperFirst]") {
    REQUIRE(toUpperFirst("") == "");
}

TEST_CASE("UpperFirsting multiword strings", "[toUpperFirst]") {
    REQUIRE(toUpperFirst("this is a test") == "This is a test");
}



TEST_CASE("TitleCasing empty strings", "[toTitleCase]") {
    REQUIRE(toTitleCase("") == "");
}

TEST_CASE("TitleCasing multiword strings", "[toTitleCase]") {
    REQUIRE(toTitleCase("this is a test") == "This Is A Test");
}



TEST_CASE("Exploding empty strings", "[explodeString]" ) {
    auto lines = explodeString("", ':');
    REQUIRE(lines.size() == 1);
    REQUIRE(lines[0] == "");
}

TEST_CASE("Exploding unbroken strings", "[explodeString]" ) {
    auto lines = explodeString("this is a test", ':');
    REQUIRE(lines.size() == 1);
    REQUIRE(lines[0] == "this is a test");
}

TEST_CASE("Exploding multi-part strings", "[explodeString]" ) {
    auto lines = explodeString(":this:is:a:test:", ':');
    REQUIRE(lines.size() == 5);
    REQUIRE(lines[0] == "this");
    REQUIRE(lines[1] == "is");
    REQUIRE(lines[2] == "a");
    REQUIRE(lines[3] == "test");
    REQUIRE(lines[4] == "");
}

TEST_CASE("Exploding with empty sections", "[explodeString]" ) {
    auto lines = explodeString("this::is:a::test", ':');
    REQUIRE(lines.size() == 4);
    REQUIRE(lines[0] == "this");
    REQUIRE(lines[1] == "is");
    REQUIRE(lines[2] == "a");
    REQUIRE(lines[3] == "test");
}



TEST_CASE("Wrapping of empty strings", "[wrapText]" ) {
    std::string testString = "";
    auto lines = wrapString(testString, 80);
    REQUIRE(lines.size() == 1);
    REQUIRE(lines[0] == "");
}

TEST_CASE("Short lines are wrapped/unchanged", "[wrapText]" ) {
    std::string testString = "Hello world!";
    auto lines = wrapString(testString, 80);
    REQUIRE(lines.size() == 1);
    REQUIRE(lines[0] == testString);
}

TEST_CASE("Long text is wordwrapped correctly", "[wrapText]" ) {
    std::string testString = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";
    auto lines = wrapString(testString, 80);
    REQUIRE(lines.size() == 6);
    REQUIRE(lines[0] == "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor");
    REQUIRE(lines[1] == "incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis");
    REQUIRE(lines[2] == "nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.");
    REQUIRE(lines[3] == "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu");
    REQUIRE(lines[4] == "fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in");
    REQUIRE(lines[5] == "culpa qui officia deserunt mollit anim id est laborum.");
}
