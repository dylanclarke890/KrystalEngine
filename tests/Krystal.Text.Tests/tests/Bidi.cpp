#include "Krystal.Text/Bidi.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::text::tests
{
  namespace
  {
    KRYS_NODISCARD bool RunIs(const BidiLevelRun &run, size_t start, size_t end, BidiLevel level)
    {
      return run.Start == start && run.End == end && run.Level == level;
    }
  }

  TEST_CASE("Bidi: character properties", "[Text][Bidi]")
  {
    CHECK(GetBidiClass(U'a') == BidiClass::LeftToRight);
    CHECK(GetBidiClass(U'א') == BidiClass::RightToLeft);
    CHECK(GetBidiClass(U'ا') == BidiClass::RightToLeftArabic);
    CHECK(GetBidiClass(U'1') == BidiClass::EuropeanNumber);
    CHECK(GetBidiClass(U' ') == BidiClass::WhiteSpaceNeutral);

    CHECK(MirroredCharacter(U'(') == U')');
    CHECK(MirroredCharacter(U'[') == U']');
    CHECK(MirroredCharacter(U'a') == U'a');
  }

  TEST_CASE("Bidi: ResolveBidiLevelRuns", "[Text][Bidi]")
  {
    SECTION("An empty paragraph has no runs")
    {
      CHECK(ResolveBidiLevelRuns(u8"", LeftToRightBidiLevel).empty());
    }

    SECTION("Left-to-right text is one run")
    {
      auto runs = ResolveBidiLevelRuns(u8"abc", LeftToRightBidiLevel);
      REQUIRE(runs.size() == 1);
      CHECK(RunIs(runs[0], 0, 3, 0));
    }

    SECTION("Right-to-left text in a left-to-right paragraph")
    {
      // "abc " then three Hebrew letters (two bytes each).
      auto runs = ResolveBidiLevelRuns(u8"abc אבג", LeftToRightBidiLevel);
      REQUIRE(runs.size() == 2);
      CHECK(RunIs(runs[0], 0, 4, 0));
      CHECK(RunIs(runs[1], 4, 10, 1));
    }

    SECTION("The default level comes from the first strong character")
    {
      auto runs = ResolveBidiLevelRuns(u8"אב a", DefaultLeftToRightBidiLevel);
      REQUIRE(runs.size() == 2);
      CHECK(RunIs(runs[0], 0, 5, 1));
      CHECK(RunIs(runs[1], 5, 6, 2));
    }

    SECTION("Offsets are bytes, past characters outside the BMP")
    {
      auto runs = ResolveBidiLevelRuns(u8"a\U0001F600b א", LeftToRightBidiLevel);
      REQUIRE(runs.size() == 2);
      CHECK(RunIs(runs[0], 0, 7, 0));
      CHECK(RunIs(runs[1], 7, 9, 1));
    }
  }

  TEST_CASE("Bidi: ReorderBidiRunsVisually", "[Text][Bidi]")
  {
    SECTION("Right-to-left runs are reversed")
    {
      BidiLevel levels[] {0, 1, 1, 0};
      int32 visualToLogical[4] {};
      ReorderBidiRunsVisually(levels, visualToLogical);
      CHECK(visualToLogical[0] == 0);
      CHECK(visualToLogical[1] == 2);
      CHECK(visualToLogical[2] == 1);
      CHECK(visualToLogical[3] == 3);
    }

    SECTION("Nested levels are reversed from the highest down")
    {
      BidiLevel levels[] {1, 2, 2, 1};
      int32 visualToLogical[4] {};
      ReorderBidiRunsVisually(levels, visualToLogical);
      // Level 2 reverses runs 1 and 2, then level 1 reverses the whole line.
      CHECK(visualToLogical[0] == 3);
      CHECK(visualToLogical[1] == 1);
      CHECK(visualToLogical[2] == 2);
      CHECK(visualToLogical[3] == 0);
    }
  }
}
