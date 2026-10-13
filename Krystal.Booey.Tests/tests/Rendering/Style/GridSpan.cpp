#include "Krystal.Booey/Rendering/Style/GridSpan.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Rendering/Style/GridArea.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridPosition.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::tests
{
  TEST_CASE("GridSpan(Factories)", "[Rendering][GridSpan]")
  {
    auto translated = GridSpan::TranslatedDefiniteGridSpan(1, 4);
    CHECK(translated.IsTranslatedDefinite());
    CHECK_FALSE(translated.IsIndefinite());
    CHECK(translated.StartLine() == 1);
    CHECK(translated.EndLine() == 4);
    CHECK(translated.IntegerSpan() == 3);

    auto untranslated = GridSpan::UntranslatedDefiniteGridSpan(-3, -1);
    CHECK_FALSE(untranslated.IsTranslatedDefinite());
    CHECK_FALSE(untranslated.IsIndefinite());
    CHECK(untranslated.UntranslatedStartLine() == -3);
    CHECK(untranslated.UntranslatedEndLine() == -1);
    CHECK(untranslated.IntegerSpan() == 2);

    CHECK(GridSpan::IndefiniteGridSpan().IsIndefinite());
    CHECK(GridSpan::MasonryAxisTranslatedDefiniteGridSpan() == GridSpan::TranslatedDefiniteGridSpan(0, 1));

    CHECK(translated == GridSpan::TranslatedDefiniteGridSpan(1, 4));
    CHECK(translated != GridSpan::TranslatedDefiniteGridSpan(1, 5));
    CHECK(GridSpan::UntranslatedDefiniteGridSpan(1, 4) != translated);
  }

  TEST_CASE("GridSpan(Iteration)", "[Rendering][GridSpan]")
  {
    List<unsigned> lines;
    for (unsigned line : GridSpan::TranslatedDefiniteGridSpan(2, 5))
    {
      lines.push_back(line);
    }
    CHECK(lines == List<unsigned> {2, 3, 4});
  }

  TEST_CASE("GridSpan(Translate)", "[Rendering][GridSpan]")
  {
    auto span = GridSpan::UntranslatedDefiniteGridSpan(-2, 1);
    span.Translate(3);
    CHECK(span == GridSpan::TranslatedDefiniteGridSpan(1, 4));

    SECTION("into a parent")
    {
      auto parent = GridSpan::TranslatedDefiniteGridSpan(2, 10);
      span.TranslateTo(parent, false);
      CHECK(span == GridSpan::TranslatedDefiniteGridSpan(3, 6));
    }

    SECTION("into a parent, reversed")
    {
      auto parent = GridSpan::TranslatedDefiniteGridSpan(2, 10);
      span.TranslateTo(parent, true);
      CHECK(span == GridSpan::TranslatedDefiniteGridSpan(6, 9));
    }
  }

  TEST_CASE("GridSpan(Clamp)", "[Rendering][GridSpan]")
  {
    SECTION("to a maximum")
    {
      auto span = GridSpan::UntranslatedDefiniteGridSpan(-2, 8);
      span.Clamp(5);
      CHECK(span.UntranslatedStartLine() == 0);
      CHECK(span.UntranslatedEndLine() == 5);

      // A span past the maximum keeps its last line.
      auto past = GridSpan::UntranslatedDefiniteGridSpan(6, 8);
      past.Clamp(5);
      CHECK(past.UntranslatedStartLine() == 4);
      CHECK(past.UntranslatedEndLine() == 5);
    }

    SECTION("to a range")
    {
      auto span = GridSpan::TranslatedDefiniteGridSpan(1, 8);
      CHECK(span.Clamp(3, 5));
      CHECK(span == GridSpan::TranslatedDefiniteGridSpan(3, 5));

      auto outside = GridSpan::TranslatedDefiniteGridSpan(1, 3);
      CHECK_FALSE(outside.Clamp(3, 5));
      CHECK(outside == GridSpan::TranslatedDefiniteGridSpan(1, 3));
    }

    SECTION("to the grid position limits")
    {
      style::GridPosition::SetMaxPositionForTesting(10);
      auto span = GridSpan::UntranslatedDefiniteGridSpan(-20, 20);
      CHECK(span.UntranslatedStartLine() == -10);
      CHECK(span.UntranslatedEndLine() == 10);

      auto start = GridSpan::UntranslatedDefiniteGridSpan(15, 20);
      CHECK(start.UntranslatedStartLine() == 9);
      CHECK(start.UntranslatedEndLine() == 10);
      style::GridPosition::SetMaxPositionForTesting(1'000'000);
    }
  }

  TEST_CASE("GridArea", "[Rendering][GridArea]")
  {
    GridArea indefinite;
    CHECK(indefinite.Rows.IsIndefinite());
    CHECK(indefinite.Columns.IsIndefinite());

    GridArea area {GridSpan::TranslatedDefiniteGridSpan(0, 2), GridSpan::TranslatedDefiniteGridSpan(1, 3)};
    CHECK(area.Span(style::GridTrackSizingDirection::Rows) == GridSpan::TranslatedDefiniteGridSpan(0, 2));
    CHECK(area.Span(style::GridTrackSizingDirection::Columns) == GridSpan::TranslatedDefiniteGridSpan(1, 3));
    CHECK(
      area
      == GridArea {GridSpan::TranslatedDefiniteGridSpan(0, 2), GridSpan::TranslatedDefiniteGridSpan(1, 3)});
    CHECK(area != indefinite);

    CHECK(style::OrthogonalDirection(style::GridTrackSizingDirection::Rows)
          == style::GridTrackSizingDirection::Columns);
    CHECK(style::OrthogonalDirection(style::GridTrackSizingDirection::Columns)
          == style::GridTrackSizingDirection::Rows);
  }
}
