#include "Krystal.HTML.Tests/TreeConstructionTestParser.hpp"
#include <catch_all.hpp>
#include <sstream>

namespace Krys::HTML::Tests
{
  static List<TreeConstructionTest> Parse(const char *dat) noexcept
  {
    std::istringstream stream(dat);
    return ParseTreeConstructionTests(stream);
  }

  TEST_CASE("ParseTreeConstructionTests: single test case with errors",
            "[HTML][TreeConstruction][Parser]")
  {
    auto tests = Parse(
      "#data\n"
      "<div></div>\n"
      "#errors\n"
      "(1,1): expected-doctype-but-got-start-tag\n"
      "#document\n"
      "| <html>\n"
      "|   <head>\n"
      "|   <body>\n"
      "|     <div>\n");

    REQUIRE(tests.size() == 1);

    CHECK(tests[0].Input == U"<div></div>\n");
    CHECK(tests[0].ExpectedDocument == u8"#document\n"
                                       u8"| <html>\n"
                                       u8"|   <head>\n"
                                       u8"|   <body>\n"
                                       u8"|     <div>\n");
  }

  TEST_CASE("ParseTreeConstructionTests: errors section may be empty",
            "[HTML][TreeConstruction][Parser]")
  {
    auto tests = Parse(
      "#data\n"
      "<!DOCTYPE html>\n"
      "#errors\n"
      "#document\n"
      "| <!DOCTYPE html>\n"
      "| <html>\n"
      "|   <head>\n"
      "|   <body>\n");

    REQUIRE(tests.size() == 1);

    CHECK(tests[0].Input == U"<!DOCTYPE html>\n");
    CHECK(tests[0].ExpectedDocument == u8"#document\n"
                                       u8"| <!DOCTYPE html>\n"
                                       u8"| <html>\n"
                                       u8"|   <head>\n"
                                       u8"|   <body>\n");
  }

  TEST_CASE("ParseTreeConstructionTests: #new-errors section is skipped",
            "[HTML][TreeConstruction][Parser]")
  {
    auto tests = Parse(
      "#data\n"
      "<div foo=`bar`>\n"
      "#errors\n"
      "(1,10): equals-in-unquoted-attribute-value\n"
      "#new-errors\n"
      "(1:10) unexpected-character-in-unquoted-attribute-value\n"
      "#document\n"
      "| <html>\n"
      "|   <head>\n"
      "|   <body>\n"
      "|     <div>\n"
      "|       foo=\"`bar`\"\n");

    REQUIRE(tests.size() == 1);

    CHECK(tests[0].Input == U"<div foo=`bar`>\n");
    CHECK(tests[0].ExpectedDocument == u8"#document\n"
                                       u8"| <html>\n"
                                       u8"|   <head>\n"
                                       u8"|   <body>\n"
                                       u8"|     <div>\n"
                                       u8"|       foo=\"`bar`\"\n");
  }

  TEST_CASE("ParseTreeConstructionTests: unknown # keyword is skipped",
            "[HTML][TreeConstruction][Parser]")
  {
    auto tests = Parse(
      "#data\n"
      "<p>hello</p>\n"
      "#errors\n"
      "#unknown-section\n"
      "this line should be ignored\n"
      "#document\n"
      "| <html>\n"
      "|   <head>\n"
      "|   <body>\n"
      "|     <p>\n"
      "|       \"hello\"\n");

    REQUIRE(tests.size() == 1);

    CHECK(tests[0].Input == U"<p>hello</p>\n");
    CHECK(tests[0].ExpectedDocument == u8"#document\n"
                                       u8"| <html>\n"
                                       u8"|   <head>\n"
                                       u8"|   <body>\n"
                                       u8"|     <p>\n"
                                       u8"|       \"hello\"\n");
  }

  TEST_CASE("ParseTreeConstructionTests: multiple test cases separated by blank lines",
            "[HTML][TreeConstruction][Parser]")
  {
    auto tests = Parse(
      "#data\n"
      "<p>one</p>\n"
      "#errors\n"
      "#document\n"
      "| <html>\n"
      "|   <head>\n"
      "|   <body>\n"
      "|     <p>\n"
      "|       \"one\"\n"
      "\n"
      "#data\n"
      "<p>two</p>\n"
      "#errors\n"
      "#document\n"
      "| <html>\n"
      "|   <head>\n"
      "|   <body>\n"
      "|     <p>\n"
      "|       \"two\"\n");

    REQUIRE(tests.size() == 2);

    CHECK(tests[0].Input == U"<p>one</p>\n");
    CHECK(tests[0].ExpectedDocument == u8"#document\n"
                                       u8"| <html>\n"
                                       u8"|   <head>\n"
                                       u8"|   <body>\n"
                                       u8"|     <p>\n"
                                       u8"|       \"one\"\n");

    CHECK(tests[1].Input == U"<p>two</p>\n");
    CHECK(tests[1].ExpectedDocument == u8"#document\n"
                                       u8"| <html>\n"
                                       u8"|   <head>\n"
                                       u8"|   <body>\n"
                                       u8"|     <p>\n"
                                       u8"|       \"two\"\n");
  }

  TEST_CASE("ParseTreeConstructionTests: last test case without trailing blank line",
            "[HTML][TreeConstruction][Parser]")
  {
    auto tests = Parse(
      "#data\n"
      "<span></span>\n"
      "#errors\n"
      "#document\n"
      "| <html>\n"
      "|   <head>\n"
      "|   <body>\n"
      "|     <span>");

    REQUIRE(tests.size() == 1);

    CHECK(tests[0].Input == U"<span></span>\n");
    CHECK(tests[0].ExpectedDocument == u8"#document\n"
                                       u8"| <html>\n"
                                       u8"|   <head>\n"
                                       u8"|   <body>\n"
                                       u8"|     <span>\n");
  }

  TEST_CASE("ParseTreeConstructionTests: multi-line data is preserved",
            "[HTML][TreeConstruction][Parser]")
  {
    auto tests = Parse(
      "#data\n"
      "<div>\n"
      "  text\n"
      "</div>\n"
      "#errors\n"
      "#document\n"
      "| <html>\n"
      "|   <head>\n"
      "|   <body>\n"
      "|     <div>\n"
      "|       \"\\n  text\\n\"\n");

    REQUIRE(tests.size() == 1);

    CHECK(tests[0].Input == U"<div>\n  text\n</div>\n");
  }

  TEST_CASE("ParseTreeConstructionTests: empty stream produces no tests",
            "[HTML][TreeConstruction][Parser]")
  {
    auto tests = Parse("");
    REQUIRE(tests.empty());
  }

  TEST_CASE("ParseTreeConstructionTests: element attributes are captured in expected document",
            "[HTML][TreeConstruction][Parser]")
  {
    auto tests = Parse(
      "#data\n"
      "<div bar=\"ZZ&gt;YY\"></div>\n"
      "#errors\n"
      "(1,20): expected-doctype-but-got-start-tag\n"
      "#document\n"
      "| <html>\n"
      "|   <head>\n"
      "|   <body>\n"
      "|     <div>\n"
      "|       bar=\"ZZ>YY\"\n");

    REQUIRE(tests.size() == 1);

    CHECK(tests[0].Input == U"<div bar=\"ZZ&gt;YY\"></div>\n");
    CHECK(tests[0].ExpectedDocument == u8"#document\n"
                                       u8"| <html>\n"
                                       u8"|   <head>\n"
                                       u8"|   <body>\n"
                                       u8"|     <div>\n"
                                       u8"|       bar=\"ZZ>YY\"\n");
  }
}
