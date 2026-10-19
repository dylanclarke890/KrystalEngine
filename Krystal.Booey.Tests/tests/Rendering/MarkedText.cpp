#include "Krystal.Booey/Rendering/MarkedText.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Rendering/TextBoxSelectableRange.hpp"

namespace krys::boo::tests
{
  using Type = MarkedText::Type;

  TEST_CASE("MarkedText: subdivides overlapping marked texts in paint order", "[Rendering][MarkedText]")
  {
    CHECK(MarkedText::Subdivide({}).empty());

    // Unmarked text with a selection over its middle.
    auto pieces = MarkedText::Subdivide({{0, 10, Type::Unmarked}, {3, 6, Type::Selection}});
    REQUIRE(pieces.size() == 4);
    CHECK(pieces[0] == MarkedText {0, 3, Type::Unmarked});
    CHECK(pieces[1] == MarkedText {3, 6, Type::Unmarked});
    CHECK(pieces[2] == MarkedText {3, 6, Type::Selection});
    CHECK(pieces[3] == MarkedText {6, 10, Type::Unmarked});

    // Only the frontmost marked text of each range.
    auto frontmost =
      MarkedText::Subdivide({{0, 10, Type::Unmarked}, {2, 8, Type::SpellingError}, {4, 6, Type::Selection}},
                            MarkedText::OverlapStrategy::Frontmost);
    REQUIRE(frontmost.size() == 5);
    CHECK(frontmost[0] == MarkedText {0, 2, Type::Unmarked});
    CHECK(frontmost[1] == MarkedText {2, 4, Type::SpellingError});
    CHECK(frontmost[2] == MarkedText {4, 6, Type::Selection});
    CHECK(frontmost[3] == MarkedText {6, 8, Type::SpellingError});
    CHECK(frontmost[4] == MarkedText {8, 10, Type::Unmarked});

    // Touching ranges stay apart, and empty ones vanish.
    auto touching =
      MarkedText::Subdivide({{0, 4, Type::Unmarked}, {4, 4, Type::Selection}, {4, 9, Type::Unmarked}});
    REQUIRE(touching.size() == 2);
    CHECK(touching[0] == MarkedText {0, 4, Type::Unmarked});
    CHECK(touching[1] == MarkedText {4, 9, Type::Unmarked});
  }

  TEST_CASE("TextBoxSelectableRange: clamps offsets into the box", "[Rendering][MarkedText]")
  {
    TextBoxSelectableRange range {.Start = 5, .Length = 4};
    CHECK(range.Clamp(0) == 0);
    CHECK(range.Clamp(7) == 2);
    CHECK(range.Clamp(20) == 4);
    CHECK(range.Clamp(6, 12) == std::pair<uint32, uint32> {1, 4});
    CHECK(range.Intersects(6, 7));
    CHECK_FALSE(range.Intersects(0, 5));

    // A hyphen past the end, and truncated text.
    TextBoxSelectableRange hyphenated {.Start = 0, .Length = 4, .AdditionalLengthAtEnd = 1};
    CHECK(hyphenated.Clamp(4) == 5);
    TextBoxSelectableRange truncated {.Start = 0, .Length = 8, .Truncation = 3};
    CHECK(truncated.Clamp(6) == 3);
    TextBoxSelectableRange mismatched {
      .Start = 0, .Length = 8, .Truncation = 3, .HasMismatchingContentDirection = true};
    CHECK(mismatched.Clamp(2) == 5);
  }
}
