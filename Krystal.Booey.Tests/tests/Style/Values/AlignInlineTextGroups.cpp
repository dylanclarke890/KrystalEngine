#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/BuilderTestUtils.hpp"
#include "Krystal.Booey/CSS/PrimitiveValueMappings.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/Values/Align/AlignContent.hpp"
#include "Krystal.Booey/Style/Values/Align/AlignItems.hpp"
#include "Krystal.Booey/Style/Values/Align/AlignSelf.hpp"
#include "Krystal.Booey/Style/Values/Align/GapGutter.hpp"
#include "Krystal.Booey/Style/Values/Align/JustifyContent.hpp"
#include "Krystal.Booey/Style/Values/Align/JustifyItems.hpp"
#include "Krystal.Booey/Style/Values/Align/JustifySelf.hpp"
#include "Krystal.Booey/Style/Values/CSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Inline/LineFitEdge.hpp"
#include "Krystal.Booey/Style/Values/Inline/LineHeight.hpp"
#include "Krystal.Booey/Style/Values/Inline/TextBoxEdge.hpp"
#include "Krystal.Booey/Style/Values/Inline/TextEdgeCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Inline/VerticalAlign.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/Style/Values/ScrollSnap/ScrollMargin.hpp"
#include "Krystal.Booey/Style/Values/ScrollSnap/ScrollPadding.hpp"
#include "Krystal.Booey/Style/Values/ScrollSnap/ScrollSnapAlign.hpp"
#include "Krystal.Booey/Style/Values/ScrollSnap/ScrollSnapType.hpp"
#include "Krystal.Booey/Style/Values/Text/HangingPunctuation.hpp"
#include "Krystal.Booey/Style/Values/Text/HyphenateCharacter.hpp"
#include "Krystal.Booey/Style/Values/Text/HyphenateLimitEdge.hpp"
#include "Krystal.Booey/Style/Values/Text/HyphenateLimitLines.hpp"
#include "Krystal.Booey/Style/Values/Text/LetterSpacing.hpp"
#include "Krystal.Booey/Style/Values/Text/TabSize.hpp"
#include "Krystal.Booey/Style/Values/Text/TextAlign.hpp"
#include "Krystal.Booey/Style/Values/Text/TextAlignLast.hpp"
#include "Krystal.Booey/Style/Values/Text/TextAutospace.hpp"
#include "Krystal.Booey/Style/Values/Text/TextIndent.hpp"
#include "Krystal.Booey/Style/Values/Text/TextSpacingTrim.hpp"
#include "Krystal.Booey/Style/Values/Text/TextTransform.hpp"
#include "Krystal.Booey/Style/Values/Text/WordSpacing.hpp"
#include "Krystal.Booey/Style/Values/UI/AccentColor.hpp"
#include "Krystal.Booey/Style/Values/UI/Resize.hpp"

namespace krys::boo::style::tests
{
  using css::PropertyId;
  using namespace css::Literals;

  namespace
  {
    template <typename T>
    std::string RoundTrip(BuilderFixture &fixture, PropertyId property, utf8_stringview value) noexcept
    {
      return fixture.Serialised(fixture.Convert<T>(property, value));
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "Style alignment values", "[Style][Values][AlignInlineTextGroups]")
  {
    SECTION("align-items and justify-items")
    {
      CHECK(Convert<AlignItems>(PropertyId::AlignItems, u8"normal").IsNormal());
      CHECK(RoundTrip<AlignItems>(*this, PropertyId::AlignItems, u8"center") == "center");
      CHECK(RoundTrip<AlignItems>(*this, PropertyId::AlignItems, u8"last baseline") == "last baseline");
      CHECK(RoundTrip<AlignItems>(*this, PropertyId::AlignItems, u8"unsafe end") == "unsafe end");
      CHECK(RoundTrip<JustifyItems>(*this, PropertyId::JustifyItems, u8"legacy left") == "legacy left");
      CHECK(RoundTrip<JustifyItems>(*this, PropertyId::JustifyItems, u8"stretch") == "stretch");
    }

    SECTION("align-self and justify-self")
    {
      CHECK(RoundTrip<AlignSelf>(*this, PropertyId::AlignSelf, u8"auto") == "auto");
      CHECK(RoundTrip<AlignSelf>(*this, PropertyId::AlignSelf, u8"safe flex-start") == "safe flex-start");
      CHECK(RoundTrip<JustifySelf>(*this, PropertyId::JustifySelf, u8"self-end") == "self-end");
      CHECK(RoundTrip<JustifySelf>(*this, PropertyId::JustifySelf, u8"anchor-center") == "anchor-center");
    }

    SECTION("align-content and justify-content")
    {
      CHECK(RoundTrip<AlignContent>(*this, PropertyId::AlignContent, u8"space-between") == "space-between");
      CHECK(RoundTrip<AlignContent>(*this, PropertyId::AlignContent, u8"first baseline") == "baseline");
      CHECK(RoundTrip<JustifyContent>(*this, PropertyId::JustifyContent, u8"safe center") == "safe center");
      CHECK(RoundTrip<JustifyContent>(*this, PropertyId::JustifyContent, u8"normal") == "normal");
    }

    SECTION("gaps")
    {
      CHECK(Convert<GapGutter>(PropertyId::ColumnGap, u8"normal").IsNormal());
      CHECK(RoundTrip<GapGutter>(*this, PropertyId::RowGap, u8"12px") == "12px");
      CHECK(RoundTrip<GapGutter>(*this, PropertyId::RowGap, u8"10%") == "10%");
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "Style inline layout values", "[Style][Values][AlignInlineTextGroups]")
  {
    SECTION("line-height")
    {
      CHECK(Convert<LineHeight>(PropertyId::LineHeight, u8"normal").IsNormal());
      CHECK(RoundTrip<LineHeight>(*this, PropertyId::LineHeight, u8"20px") == "20px");
      // Numbers are stored as percentages, and percentages are resolved against the font size.
      CHECK(RoundTrip<LineHeight>(*this, PropertyId::LineHeight, u8"1.5") == "150%");
      CHECK(Convert<LineHeight>(PropertyId::LineHeight, u8"150%").IsFixed());
    }

    SECTION("vertical-align")
    {
      CHECK(Convert<VerticalAlign>(PropertyId::VerticalAlign, u8"text-top").IsTextTop());
      CHECK(RoundTrip<VerticalAlign>(*this, PropertyId::VerticalAlign, u8"middle") == "middle");
      CHECK(RoundTrip<VerticalAlign>(*this, PropertyId::VerticalAlign, u8"-3px") == "-3px");
      CHECK(RoundTrip<VerticalAlign>(*this, PropertyId::VerticalAlign, u8"25%") == "25%");
    }

    SECTION("text-box-edge and line-fit-edge")
    {
      CHECK(RoundTrip<TextBoxEdge>(*this, PropertyId::TextBoxEdge, u8"auto") == "auto");
      CHECK(RoundTrip<TextBoxEdge>(*this, PropertyId::TextBoxEdge, u8"cap alphabetic") == "cap alphabetic");
      CHECK(RoundTrip<TextBoxEdge>(*this, PropertyId::TextBoxEdge, u8"text text") == "text");
      CHECK(RoundTrip<LineFitEdge>(*this, PropertyId::LineFitEdge, u8"leading") == "leading");
      CHECK(RoundTrip<LineFitEdge>(*this, PropertyId::LineFitEdge, u8"ex") == "ex");
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "Style text values", "[Style][Values][AlignInlineTextGroups]")
  {
    SECTION("text-align")
    {
      CHECK(Convert<TextAlign>(PropertyId::TextAlign, u8"justify") == TextAlign::Justify);
      CHECK(Convert<TextAlign>(PropertyId::TextAlign, u8"start") == TextAlign::Start);
      CHECK(Serialised(TextAlign::Center) == "center");

      // `match-parent` resolves the parent's `start`/`end` against its direction.
      CHECK(Convert<TextAlign>(PropertyId::TextAlign, u8"match-parent") == TextAlign::Left);
      ParentStyle.SetTextAlign(TextAlign::End);
      CHECK(Convert<TextAlign>(PropertyId::TextAlign, u8"match-parent") == TextAlign::Right);
      ParentStyle.SetTextAlign(TextAlign::Justify);
      CHECK(Convert<TextAlign>(PropertyId::TextAlign, u8"match-parent") == TextAlign::Justify);
    }

    SECTION("text-align-last")
    {
      CHECK(Convert<TextAlignLast>(PropertyId::TextAlignLast, u8"auto") == TextAlignLast::Auto);
      ParentStyle.SetTextAlignLast(TextAlignLast::Start);
      CHECK(Convert<TextAlignLast>(PropertyId::TextAlignLast, u8"match-parent") == TextAlignLast::Left);
      CHECK(Serialised(TextAlignLast::Justify) == "justify");
    }

    SECTION("text-indent")
    {
      CHECK(RoundTrip<TextIndent>(*this, PropertyId::TextIndent, u8"10px") == "10px");
      CHECK(RoundTrip<TextIndent>(*this, PropertyId::TextIndent, u8"5% hanging") == "5% hanging");
      CHECK(RoundTrip<TextIndent>(*this, PropertyId::TextIndent, u8"each-line 2px") == "2px each-line");
    }

    SECTION("text-transform and hanging-punctuation")
    {
      CHECK(RoundTrip<TextTransform>(*this, PropertyId::TextTransform, u8"none") == "none");
      CHECK(RoundTrip<TextTransform>(*this, PropertyId::TextTransform, u8"full-width uppercase")
            == "uppercase full-width");
      CHECK(RoundTrip<HangingPunctuation>(*this, PropertyId::HangingPunctuation, u8"last first")
            == "first last");
      CHECK(RoundTrip<HangingPunctuation>(*this, PropertyId::HangingPunctuation, u8"allow-end")
            == "allow-end");
    }

    SECTION("spacing")
    {
      // `normal` computes to zero.
      CHECK(RoundTrip<LetterSpacing>(*this, PropertyId::LetterSpacing, u8"normal") == "0px");
      CHECK(RoundTrip<LetterSpacing>(*this, PropertyId::LetterSpacing, u8"2px") == "2px");
      CHECK(RoundTrip<WordSpacing>(*this, PropertyId::WordSpacing, u8"10%") == "10%");
      CHECK(RoundTrip<TextAutospace>(*this, PropertyId::TextAutospace, u8"ideograph-numeric ideograph-alpha")
            == "ideograph-alpha ideograph-numeric");
      CHECK(RoundTrip<TextSpacingTrim>(*this, PropertyId::TextSpacingTrim, u8"trim-all") == "trim-all");
    }

    SECTION("tab-size and hyphenation")
    {
      CHECK(RoundTrip<TabSize>(*this, PropertyId::TabSize, u8"4") == "4");
      CHECK(RoundTrip<TabSize>(*this, PropertyId::TabSize, u8"20px") == "20px");
      CHECK(RoundTrip<HyphenateCharacter>(*this, PropertyId::HyphenateCharacter, u8"auto") == "auto");
      CHECK(RoundTrip<HyphenateCharacter>(*this, PropertyId::HyphenateCharacter, u8"\"-\"") == "\"-\"");
      CHECK(RoundTrip<HyphenateLimitEdge>(*this, PropertyId::WebkitHyphenateLimitBefore, u8"3") == "3");
      CHECK(RoundTrip<HyphenateLimitLines>(*this, PropertyId::WebkitHyphenateLimitLines, u8"no-limit")
            == "no-limit");
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "Style scroll snap and UI values",
                   "[Style][Values][AlignInlineTextGroups]")
  {
    SECTION("scroll-snap-type and scroll-snap-align")
    {
      CHECK(Convert<ScrollSnapType>(PropertyId::ScrollSnapType, u8"none").IsNone());
      CHECK(RoundTrip<ScrollSnapType>(*this, PropertyId::ScrollSnapType, u8"x mandatory") == "x mandatory");
      CHECK(RoundTrip<ScrollSnapType>(*this, PropertyId::ScrollSnapType, u8"both proximity") == "both");
      CHECK(RoundTrip<ScrollSnapAlign>(*this, PropertyId::ScrollSnapAlign, u8"start end") == "start end");
      CHECK(RoundTrip<ScrollSnapAlign>(*this, PropertyId::ScrollSnapAlign, u8"center center") == "center");
    }

    SECTION("scroll-margin and scroll-padding")
    {
      CHECK(RoundTrip<ScrollMarginEdge>(*this, PropertyId::ScrollMarginTop, u8"4px") == "4px");
      CHECK(RoundTrip<ScrollPaddingEdge>(*this, PropertyId::ScrollPaddingLeft, u8"auto") == "auto");
      CHECK(RoundTrip<ScrollPaddingEdge>(*this, PropertyId::ScrollPaddingLeft, u8"10%") == "10%");
    }

    SECTION("resize")
    {
      CHECK(Convert<Resize>(PropertyId::Resize, u8"both") == Resize::Both);
      CHECK(Convert<Resize>(PropertyId::Resize, u8"block") == Resize::Block);
      CHECK(Serialised(Resize::Horizontal) == "horizontal");
    }

    SECTION("accent-color")
    {
      CHECK(Convert<AccentColor>(PropertyId::AccentColor, u8"auto").IsAuto());
      CHECK(RoundTrip<AccentColor>(*this, PropertyId::AccentColor, u8"red") == "rgb(255, 0, 0)");
      CHECK(AccentColor {css::keywords::Auto {}}.ColorOrCurrentColor().IsCurrentColor());
    }
  }
}
