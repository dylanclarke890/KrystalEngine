#include "Krystal.Booey/CSS/Properties/PropertyParserCustom.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParsing.hpp"
#include <string>

namespace krys::boo::css::tests
{
  namespace
  {
    /// @brief Parses `css` as the value of `shorthand` and lists the longhands it sets.
    /// @return "<null>" if parsing failed, otherwise "longhand: value" pairs in the order they were added,
    /// joined by "; ". A longhand left at its implicit initial value is written as "longhand: -".
    KRYS_NODISCARD std::string ShorthandToText(utf8_stringview css, PropertyId shorthand,
                                               const ParserContext &context)
    {
      PropertyParserState state {.Context = context, .CurrentProperty = shorthand};
      ParsedPropertyList properties;
      PropertyParserResult result {.ParsedProperties = properties};

      Tokens tokens {css};
      auto range = tokens.Tokenizer.Tokens();
      if (!PropertyParsing::ParseStylePropertyShorthand(range, shorthand, state, result))
      {
        return "<null>";
      }

      std::string text;
      for (const auto &property : properties)
      {
        if (!text.empty())
        {
          text += "; ";
        }

        text += ToNarrow(ToString(property.Id())) + ": ";
        text += property.Value().IsImplicitInitialValue() ? "-" : CssText(property.Value());
      }

      return text;
    }

    /// @brief As above, with a standard-mode context.
    KRYS_NODISCARD std::string ShorthandToText(utf8_stringview css, PropertyId shorthand)
    {
      ParserContext context {ParserMode::HTMLStandard};
      return ShorthandToText(css, shorthand, context);
    }

    /// @brief Parses `css` as the value of the longhand `property`.
    KRYS_NODISCARD std::string LonghandToText(utf8_stringview css, PropertyId property)
    {
      return ConsumeToText(css, [property](TokenRange &range, PropertyParserState &state)
                           { return PropertyParsing::ParseStylePropertyLonghand(range, property, state); });
    }
  }

  TEST_CASE_METHOD(ValueFixture, "PropertyParsing longhands", "[CSS][Properties][PropertyParsing]")
  {
    CHECK(LonghandToText(u8"10px", PropertyId::Width) == "10px");
    CHECK(LonghandToText(u8"red", PropertyId::Color) == "red");
    CHECK(LonghandToText(u8"10px", PropertyId::Color) == "<null>");
  }

  TEST_CASE_METHOD(ValueFixture, "PropertyParsing border-spacing", "[CSS][Properties][PropertyParsing]")
  {
    REQUIRE_FALSE(IsShorthand(PropertyId::BorderSpacing));

    CHECK(LonghandToText(u8"1px", PropertyId::BorderSpacing) == "1px");
    CHECK(LonghandToText(u8"1px 2px", PropertyId::BorderSpacing) == "1px 2px");
    CHECK(LonghandToText(u8"1px 1px", PropertyId::BorderSpacing) == "1px");
    CHECK(LonghandToText(u8"0 calc(1px + 1em)", PropertyId::BorderSpacing) == "0px calc(1em + 1px)");
    CHECK(LonghandToText(u8"1px 2px 3px", PropertyId::BorderSpacing) == "<partial>");
    CHECK(LonghandToText(u8"-1px", PropertyId::BorderSpacing) == "<null>");
    CHECK(LonghandToText(u8"1px -2px", PropertyId::BorderSpacing) == "<partial>");
    CHECK(LonghandToText(u8"10%", PropertyId::BorderSpacing) == "<null>");

    // Unitless lengths are accepted only in quirks mode.
    auto parse = [](TokenRange &range, PropertyParserState &state)
    {
      return PropertyParsing::ParseStylePropertyLonghand(range, PropertyId::BorderSpacing, state);
    };
    ParserContext standard {ParserMode::HTMLStandard};
    ParserContext quirks {ParserMode::HTMLQuirks};
    CHECK(ConsumeToText(u8"1 2", parse, standard, PropertyId::BorderSpacing) == "<null>");
    CHECK(ConsumeToText(u8"1 2", parse, quirks, PropertyId::BorderSpacing) == "1px 2px");
  }

  TEST_CASE_METHOD(ValueFixture, "PropertyParsing stroke-linejoin", "[CSS][Properties][PropertyParsing]")
  {
    // SVG 2's values.
    CHECK(LonghandToText(u8"miter", PropertyId::StrokeLinejoin) == "miter");
    CHECK(LonghandToText(u8"round", PropertyId::StrokeLinejoin) == "round");
    CHECK(LonghandToText(u8"bevel", PropertyId::StrokeLinejoin) == "bevel");

    // Fill and Stroke 3's other values and two-part form aren't supported.
    CHECK(LonghandToText(u8"crop", PropertyId::StrokeLinejoin) == "<null>");
    CHECK(LonghandToText(u8"arcs", PropertyId::StrokeLinejoin) == "<null>");
    CHECK(LonghandToText(u8"fallback", PropertyId::StrokeLinejoin) == "<null>");
    CHECK(LonghandToText(u8"miter-clip", PropertyId::StrokeLinejoin) == "<null>");
    CHECK(LonghandToText(u8"miter bevel", PropertyId::StrokeLinejoin) == "<partial>");
  }

  TEST_CASE_METHOD(ValueFixture, "PropertyParserCustom", "[CSS][Properties][PropertyParserCustom]")
  {
    SECTION("border-radius")
    {
      CHECK(ShorthandToText(u8"1px calc(2px + 10%)", PropertyId::BorderRadius)
            == "border-top-left-radius: 1px; border-top-right-radius: calc(10% + 2px); "
               "border-bottom-right-radius: 1px; border-bottom-left-radius: calc(10% + 2px)");
    }

    SECTION("animation and transition timing functions")
    {
      CHECK(ShorthandToText(u8"1s ease-in", PropertyId::Animation)
              .contains("animation-timing-function: ease-in"));
      CHECK(ShorthandToText(u8"opacity 1s linear", PropertyId::Transition)
            == "transition-duration: 1s; transition-timing-function: linear; transition-delay: initial; "
               "transition-behavior: initial; transition-property: opacity");
    }

    SECTION("mask with a position")
    {
      CHECK(ShorthandToText(u8"url(a.png) 10px 20px", PropertyId::Mask)
              .contains("-internal-mask-position-x: 10px; -internal-mask-position-y: 20px"));
    }

    SECTION("overflow")
    {
      CHECK(ShorthandToText(u8"hidden", PropertyId::Overflow) == "overflow-x: hidden; overflow-y: hidden");
    }

    SECTION("grid-template")
    {
      CHECK(ShorthandToText(u8"none", PropertyId::GridTemplate)
            == "grid-template-rows: none; grid-template-columns: none; grid-template-areas: none");
      CHECK(ShorthandToText(u8"\"a\" / 1fr", PropertyId::GridTemplate)
            == "grid-template-rows: auto; grid-template-columns: 1fr; grid-template-areas: \"a\"");
      CHECK(ShorthandToText(u8"[x] \"a\" [y] [z] \"b\"", PropertyId::GridTemplate)
            == "grid-template-rows: [x] auto [y z] auto; grid-template-columns: none; "
               "grid-template-areas: \"a\" \"b\"");
    }

    SECTION("grid-area")
    {
      CHECK(ShorthandToText(u8"a / b", PropertyId::GridArea)
            == "grid-row-start: a; grid-column-start: b; grid-row-end: a; grid-column-end: b");
    }

    SECTION("font")
    {
      CHECK(ShorthandToText(u8"caption", PropertyId::Font).starts_with("font-style: caption"));
      CHECK(ShorthandToText(u8"caption 10px serif", PropertyId::Font) == "<null>");
    }

    SECTION("font-variant")
    {
      CHECK(ShorthandToText(u8"small-caps common-ligatures slashed-zero", PropertyId::FontVariant)
            == "font-variant-ligatures: common-ligatures; font-variant-caps: small-caps; "
               "font-variant-alternates: -; font-variant-numeric: slashed-zero; font-variant-east-asian: -; "
               "font-variant-position: -; font-variant-emoji: -");
    }

    SECTION("flex")
    {
      CHECK(ShorthandToText(u8"stretch", PropertyId::Flex)
            == "flex-grow: 1; flex-shrink: 1; flex-basis: stretch");
    }

    SECTION("page-break-*")
    {
      // The legacy shorthands map onto break-*: always is page, and the other values are the same.
      CHECK(ShorthandToText(u8"always", PropertyId::PageBreakAfter) == "break-after: page");
      CHECK(ShorthandToText(u8"auto", PropertyId::PageBreakAfter) == "break-after: auto");
      CHECK(ShorthandToText(u8"avoid", PropertyId::PageBreakAfter) == "break-after: avoid");
      CHECK(ShorthandToText(u8"left", PropertyId::PageBreakAfter) == "break-after: left");
      CHECK(ShorthandToText(u8"right", PropertyId::PageBreakAfter) == "break-after: right");
      CHECK(ShorthandToText(u8"always", PropertyId::PageBreakBefore) == "break-before: page");
      CHECK(ShorthandToText(u8"left", PropertyId::PageBreakBefore) == "break-before: left");
      CHECK(ShorthandToText(u8"avoid", PropertyId::PageBreakInside) == "break-inside: avoid");
      CHECK(ShorthandToText(u8"auto", PropertyId::PageBreakInside) == "break-inside: auto");

      // break-*'s other values have no legacy keyword.
      CHECK(ShorthandToText(u8"page", PropertyId::PageBreakAfter) == "<null>");
      CHECK(ShorthandToText(u8"column", PropertyId::PageBreakBefore) == "<null>");
      CHECK(ShorthandToText(u8"recto", PropertyId::PageBreakBefore) == "<null>");
      CHECK(ShorthandToText(u8"always left", PropertyId::PageBreakAfter) == "<null>");
      CHECK(ShorthandToText(u8"always", PropertyId::PageBreakInside) == "<null>");
      CHECK(ShorthandToText(u8"left", PropertyId::PageBreakInside) == "<null>");
      CHECK(ShorthandToText(u8"avoid-page", PropertyId::PageBreakInside) == "<null>");
    }

    SECTION("overscroll-behavior")
    {
      CHECK(ShorthandToText(u8"contain", PropertyId::OverscrollBehavior)
            == "overscroll-behavior-x: contain; overscroll-behavior-y: contain");
      CHECK(ShorthandToText(u8"contain none", PropertyId::OverscrollBehavior)
            == "overscroll-behavior-x: contain; overscroll-behavior-y: none");
      CHECK(ShorthandToText(u8"auto none contain", PropertyId::OverscrollBehavior) == "<null>");
      CHECK(ShorthandToText(u8"scroll", PropertyId::OverscrollBehavior) == "<null>");
    }

    SECTION("contain-intrinsic-size")
    {
      CHECK(ShorthandToText(u8"10px", PropertyId::ContainIntrinsicSize)
            == "contain-intrinsic-width: 10px; contain-intrinsic-height: 10px");
      CHECK(ShorthandToText(u8"auto 10px", PropertyId::ContainIntrinsicSize)
            == "contain-intrinsic-width: auto 10px; contain-intrinsic-height: auto 10px");
      CHECK(ShorthandToText(u8"10px auto none", PropertyId::ContainIntrinsicSize)
            == "contain-intrinsic-width: 10px; contain-intrinsic-height: auto none");
      CHECK(ShorthandToText(u8"none 10px", PropertyId::ContainIntrinsicSize)
            == "contain-intrinsic-width: none; contain-intrinsic-height: 10px");
      CHECK(ShorthandToText(u8"1px 2px 3px", PropertyId::ContainIntrinsicSize) == "<null>");
      CHECK(ShorthandToText(u8"-1px", PropertyId::ContainIntrinsicSize) == "<null>");
      CHECK(ShorthandToText(u8"auto", PropertyId::ContainIntrinsicSize) == "<null>");
    }

    SECTION("view-timeline")
    {
      ParserContext context {ParserMode::HTMLStandard};
      CHECK(ShorthandToText(u8"--a x 10px", PropertyId::ViewTimeline, context)
            == "view-timeline-name: --a; view-timeline-axis: x; view-timeline-inset: 10px");
    }
  }
}
