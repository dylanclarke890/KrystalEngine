#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Core/UnicodeBidi.hpp"
#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/Graphics/TabSize.hpp"

namespace krys::boo::tests
{
  TEST_CASE("RenderStyleConstants", "[Rendering][Style][RenderStyleConstants]")
  {
    SECTION("Parent pseudo-elements")
    {
      CHECK(ParentPseudoElement(PseudoElementType::FirstLetter) == PseudoElementType::FirstLine);
      CHECK(ParentPseudoElement(PseudoElementType::ViewTransitionNew)
            == PseudoElementType::ViewTransitionImagePair);
      CHECK(ParentPseudoElement(PseudoElementType::ViewTransitionGroup) == PseudoElementType::ViewTransition);
      CHECK_FALSE(ParentPseudoElement(PseudoElementType::Before));
      CHECK(AllPseudoElementTypes.Contains(PseudoElementType::Marker));
    }

    SECTION("Clip boxes")
    {
      CHECK(ClipMax(FillBox::ContentBox, FillBox::PaddingBox) == FillBox::PaddingBox);
      CHECK(ClipMax(FillBox::BorderBox, FillBox::ContentBox) == FillBox::BorderBox);
      CHECK(ClipMax(FillBox::Text, FillBox::ContentBox) == FillBox::ContentBox);
      CHECK(ClipMax(FillBox::Text, FillBox::NoClip) == FillBox::NoClip);
    }

    SECTION("Page breaks")
    {
      CHECK(AlwaysPageBreak(BreakBetween::Page));
      CHECK(AlwaysPageBreak(BreakBetween::VersoPage));
      CHECK_FALSE(AlwaysPageBreak(BreakBetween::Column));
      CHECK_FALSE(AlwaysPageBreak(BreakBetween::Auto));
    }

    SECTION("Transform boxes")
    {
      CHECK(TransformBoxToCSSBoxType(TransformBox::FillBox) == CSSBoxType::FillBox);
      CHECK(TransformBoxToCSSBoxType(TransformBox::ViewBox) == CSSBoxType::ViewBox);
      CHECK(TransformBoxToCSSBoxType(TransformBox::ContentBox) == CSSBoxType::ContentBox);
    }
  }

  TEST_CASE("UnicodeBidi", "[Core][UnicodeBidi]")
  {
    CHECK(IsIsolated(UnicodeBidi::Plaintext));
    CHECK(IsIsolated(UnicodeBidi::IsolateOverride));
    CHECK_FALSE(IsIsolated(UnicodeBidi::Embed));
    CHECK(IsOverride(UnicodeBidi::Override));
    CHECK_FALSE(IsOverride(UnicodeBidi::Isolate));
  }

  TEST_CASE("TabSize", "[Graphics][TabSize]")
  {
    TabSize spaces {4};
    CHECK(spaces.IsSpaces());
    CHECK(spaces.WidthInPixels(10) == 40);

    TabSize length {30, LengthValueType};
    CHECK_FALSE(length.IsSpaces());
    CHECK(length.WidthInPixels(10) == 30);
    CHECK_FALSE(static_cast<bool>(TabSize {}));
  }

  TEST_CASE("WritingMode: text orientation keeps the writing mode", "[Core][WritingMode]")
  {
    WritingMode mode {StyleWritingMode::VerticalLr, TextDirection::LTR, TextOrientation::Mixed};
    mode.SetTextOrientation(TextOrientation::Sideways);
    CHECK(mode.ComputedWritingMode() == StyleWritingMode::VerticalLr);
    CHECK(mode.ComputedTextOrientation() == TextOrientation::Sideways);
    mode.SetTextOrientation(TextOrientation::Upright);
    CHECK(mode.ComputedTextOrientation() == TextOrientation::Upright);
    CHECK(mode.ComputedWritingMode() == StyleWritingMode::VerticalLr);
  }
}
