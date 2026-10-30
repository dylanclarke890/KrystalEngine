#include "Krystal.Booey/DOM/FragmentDirectiveRangeFinder.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/NavigationTestUtils.hpp"
#include "Krystal.Booey/DOM/FragmentDirectiveParser.hpp"
#include "Krystal.Booey/Editing/TextIterator.hpp"

namespace krys::boo::dom::tests
{
  using boo::tests::NavigationFixture;
  using boo::tests::Str;

  namespace
  {
    struct FinderFixture : NavigationFixture
    {
      using NavigationFixture::NavigationFixture;

      /// @brief The text the directive @p directive (as it follows `:~:`) matches, or "(none)".
      KRYS_NODISCARD std::string Find(utf8_stringview directive) noexcept
      {
        (void)Document->UpdateLayoutIgnorePendingStylesheets();
        FragmentDirectiveParser parser(directive);
        REQUIRE(parser.ParsedTextDirectives().size() == 1);
        auto range = FragmentDirectiveRangeFinder::FindRangeFromTextDirective(
          parser.ParsedTextDirectives()[0], *Document);
        if (!range)
        {
          return "(none)";
        }
        return Str(PlainText(*range));
      }

      /// @brief The id of the element the match of @p directive starts in.
      KRYS_NODISCARD std::string StartIn(utf8_stringview directive) noexcept
      {
        (void)Document->UpdateLayoutIgnorePendingStylesheets();
        FragmentDirectiveParser parser(directive);
        auto range = FragmentDirectiveRangeFinder::FindRangeFromTextDirective(
          parser.ParsedTextDirectives()[0], *Document);
        REQUIRE(range);
        // The nearest element with an ID.
        for (auto *element = range->StartContainer().ParentElement(); element;
             element = element->ParentElement())
        {
          if (auto id = element->GetAttribute(u8"id"))
          {
            return Str(*id);
          }
        }
        return "";
      }
    };
  }

  TEST_CASE("FragmentDirectiveRangeFinder", "[DOM][FragmentDirective]")
  {
    FinderFixture fixture(u8R"(<p id=one>The quick brown fox jumps over the lazy dog.</p>
      <p id=two>A <b>quick</b> sly fox, then a brown fox.</p><p id=three>It’s “quoted” text.</p>
      <p id=hidden style="display: none">secret word</p><p id=four>Foobar and foo here.</p>)");

    SECTION("start text, case-insensitively")
    {
      CHECK(fixture.Find(u8"text=lazy%20dog") == "lazy dog");
      CHECK(fixture.Find(u8"text=LAZY") == "lazy");
      CHECK(fixture.Find(u8"text=missing") == "(none)");
    }

    SECTION("a range from start text to end text, across elements")
    {
      CHECK(fixture.Find(u8"text=quick,fox") == "quick brown fox");
      CHECK(fixture.Find(u8"text=A,sly") == "A quick sly");
    }

    SECTION("a prefix and a suffix choose among matches")
    {
      CHECK(fixture.StartIn(u8"text=quick") == "one");
      CHECK(fixture.StartIn(u8"text=A-,quick") == "two");
      CHECK(fixture.StartIn(u8"text=brown,-fox.") == "two");
      CHECK(fixture.Find(u8"text=nothing-,quick") == "(none)");
    }

    SECTION("matches start and end at word boundaries")
    {
      CHECK(fixture.StartIn(u8"text=foo") == "four");
      CHECK(fixture.Find(u8"text=foo%20here") == "foo here");
      CHECK(fixture.Find(u8"text=oba") == "(none)");
    }

    SECTION("quotation marks match their straight forms")
    {
      CHECK(fixture.Find(u8"text=It's%20%22quoted%22") == "It’s “quoted”");
    }

    SECTION("hidden text isn't searched")
    {
      CHECK(fixture.Find(u8"text=secret") == "(none)");
    }

    SECTION("several directives, leaving out those that match nothing")
    {
      (void)fixture.Document->UpdateLayoutIgnorePendingStylesheets();
      FragmentDirectiveParser parser(u8"text=jumps&text=nope&text=sly");
      auto ranges = FragmentDirectiveRangeFinder::FindRangesFromTextDirectives(parser.ParsedTextDirectives(),
                                                                               *fixture.Document);
      REQUIRE(ranges.size() == 2);
      CHECK(Str(PlainText(ranges[0])) == "jumps");
      CHECK(Str(PlainText(ranges[1])) == "sly");
    }
  }
}
