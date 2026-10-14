#include "Krystal.Booey/CSS/Properties/ShorthandSerialiser.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/StylePropertiesTestUtils.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    struct ShorthandCase
    {
      utf8_stringview Css;
      PropertyId Shorthand;
      const char *Expected;
    };

    void CheckShorthands(std::initializer_list<ShorthandCase> cases)
    {
      for (auto &testCase : cases)
      {
        INFO(ToNarrow(testCase.Css) << " => " << ToNarrow(ToString(testCase.Shorthand)));
        CHECK(SerialiseShorthand(testCase.Css, testCase.Shorthand) == testCase.Expected);
      }
    }

    /// @brief Whether `shorthand` can be serialised from `css` at all (`null` rather than a string).
    KRYS_NODISCARD bool CanSerialise(utf8_stringview css, PropertyId shorthand)
    {
      auto properties = ParseDeclarations(css);
      return SerialiseShorthandValue(DefaultSerialisationContext(), *properties, shorthand).has_value();
    }
  }

  TEST_CASE_METHOD(ValueFixture, "SerialiseShorthandValue: common checks",
                   "[CSS][Properties][ShorthandSerialiser]")
  {
    CheckShorthands({
      {u8"margin: inherit", PropertyId::Margin, "inherit"},
      {u8"margin: initial; margin-top: initial", PropertyId::Margin, "initial"},
      {u8"margin: inherit; margin-top: initial", PropertyId::Margin, ""},
      {u8"margin: inherit; margin-top: 1px", PropertyId::Margin, ""},
      {u8"margin: 1px; margin-top: 2px !important", PropertyId::Margin, ""},
      {u8"margin: 1px !important", PropertyId::Margin, "1px"},
      {u8"margin: var(--x)", PropertyId::Margin, "var(--x)"},
      {u8"margin: 1px; margin-top: var(--x)", PropertyId::Margin, ""},
      {u8"padding: var(--x); margin: 1px", PropertyId::Margin, "1px"},
      {u8"all: inherit", PropertyId::All, "inherit"},
      {u8"color: red", PropertyId::All, ""},
    });

    CHECK_FALSE(CanSerialise(u8"margin-top: 1px", PropertyId::Margin));
    CHECK_FALSE(CanSerialise(u8"margin: 1px; margin-top: var(--x)", PropertyId::Margin));
    CHECK(CanSerialise(u8"margin: 1px", PropertyId::Margin));
  }

  TEST_CASE_METHOD(ValueFixture, "SerialiseShorthandValue: pairs and quads",
                   "[CSS][Properties][ShorthandSerialiser]")
  {
    CheckShorthands({
      {u8"margin: 1px", PropertyId::Margin, "1px"},
      {u8"margin: 1px 2px", PropertyId::Margin, "1px 2px"},
      {u8"margin: 1px 2px 3px", PropertyId::Margin, "1px 2px 3px"},
      {u8"margin: 1px 2px 3px 2px", PropertyId::Margin, "1px 2px 3px"},
      {u8"margin: 1px 2px 1px 3px", PropertyId::Margin, "1px 2px 1px 3px"},
      {u8"inset: auto", PropertyId::Inset, "auto"},
      {u8"gap: 1px 1px", PropertyId::Gap, "1px"},
      {u8"gap: 1px 2px", PropertyId::Gap, "1px 2px"},
      {u8"overflow: hidden", PropertyId::Overflow, "hidden"},
      {u8"overflow: hidden auto", PropertyId::Overflow, "hidden auto"},
      {u8"place-items: center", PropertyId::PlaceItems, "center"},
      {u8"margin-block: 1px 2px", PropertyId::MarginBlock, "1px 2px"},
      {u8"border-spacing: 1px", PropertyId::BorderSpacing, "1px"},
    });
  }

  TEST_CASE_METHOD(ValueFixture, "SerialiseShorthandValue: longhand lists",
                   "[CSS][Properties][ShorthandSerialiser]")
  {
    CheckShorthands({
      {u8"flex: 1", PropertyId::Flex, "1 1 0%"},
      {u8"flex: none", PropertyId::Flex, "0 0 auto"},
      {u8"flex-flow: column", PropertyId::FlexFlow, "column"},
      {u8"flex-flow: row", PropertyId::FlexFlow, "row"},
      {u8"flex-flow: column wrap", PropertyId::FlexFlow, "column wrap"},
      {u8"outline: 1px solid red", PropertyId::Outline, "red solid 1px"},
      {u8"outline: none", PropertyId::Outline, "currentcolor"},
      {u8"text-decoration: underline red", PropertyId::TextDecoration, "underline red"},
      {u8"list-style: square", PropertyId::ListStyle, "square"},
      {u8"columns: 3", PropertyId::Columns, "3"},
      {u8"columns: 10px 3", PropertyId::Columns, "10px 3"},
      {u8"container: foo", PropertyId::Container, "foo"},
      {u8"container: foo / size", PropertyId::Container, "foo / size"},
      {u8"transform-origin: left top", PropertyId::TransformOrigin, "left top"},
      {u8"transform-origin: 1px 2px 3px", PropertyId::TransformOrigin, "1px 2px 3px"},
      {u8"perspective-origin: left top", PropertyId::PerspectiveOrigin, "left top"},
      {u8"marker: none", PropertyId::Marker, "none"},
      {u8"marker: none; marker-start: url(#m)", PropertyId::Marker, ""},
      {u8"text-decoration-skip: none", PropertyId::TextDecorationSkip, "none"},
    });
  }

  TEST_CASE_METHOD(ValueFixture, "SerialiseShorthandValue: borders", "[CSS][Properties][ShorthandSerialiser]")
  {
    CheckShorthands({
      {u8"border: 1px solid red", PropertyId::Border, "1px solid red"},
      {u8"border: solid", PropertyId::Border, "solid"},
      {u8"border: red", PropertyId::Border, "red"},
      {u8"border: medium none currentcolor", PropertyId::Border, "medium"},
      {u8"border: 1px solid red; border-top-width: 2px", PropertyId::Border, ""},
      {u8"border: 1px solid red; border-image-slice: 50%", PropertyId::Border, ""},
      {u8"border-block: 1px solid", PropertyId::BorderBlock, "1px solid"},
      {u8"border-top: 1px solid red", PropertyId::BorderTop, "1px solid red"},
      {u8"border-top: solid", PropertyId::BorderTop, "solid"},
      {u8"border-width: 1px 2px", PropertyId::BorderWidth, "1px 2px"},
      {u8"border-radius: 1px", PropertyId::BorderRadius, "1px"},
      {u8"border-radius: 1px 2px", PropertyId::BorderRadius, "1px 2px"},
      {u8"border-radius: 1px 2px 3px", PropertyId::BorderRadius, "1px 2px 3px"},
      {u8"border-radius: 1px 2px 3px 4px", PropertyId::BorderRadius, "1px 2px 3px 4px"},
      {u8"border-radius: 1px / 2px", PropertyId::BorderRadius, "1px / 2px"},
      {u8"border-radius: 1px 2px / 3px", PropertyId::BorderRadius, "1px 2px / 3px"},
      {u8"border-image: none", PropertyId::BorderImage, "none"},
      {u8"border-image: url(a.png) 30 round", PropertyId::BorderImage, "url(\"a.png\") 30 round"},
      {u8"border-image: url(a.png) 30 / 2px", PropertyId::BorderImage, "url(\"a.png\") 30 / 2px"},
      {u8"border-image: url(a.png) 30 / / 1px", PropertyId::BorderImage, "url(\"a.png\") 30 / / 1px"},
      {u8"border-image: url(a.png); border-image-width: 2px", PropertyId::BorderImage, ""},
    });
  }

  TEST_CASE_METHOD(ValueFixture, "SerialiseShorthandValue: layered", "[CSS][Properties][ShorthandSerialiser]")
  {
    CheckShorthands({
      {u8"background: red", PropertyId::Background, "red"},
      {u8"background: none", PropertyId::Background, "none"},
      {u8"background: url(a.png) no-repeat", PropertyId::Background, "url(\"a.png\") no-repeat"},
      {u8"background: url(a.png), blue", PropertyId::Background, "url(\"a.png\"), blue"},
      {u8"background: left top / 10px", PropertyId::Background, "left top / 10px auto"},
      {u8"background: content-box", PropertyId::Background, "content-box"},
      {u8"background: padding-box content-box", PropertyId::Background, "padding-box content-box"},
      {u8"background-position: left top", PropertyId::BackgroundPosition, "left top"},
      {u8"background-position: 10px", PropertyId::BackgroundPosition, "10px center"},
      {u8"transition: opacity 1s", PropertyId::Transition, "opacity 1s"},
      {u8"transition: all", PropertyId::Transition, "all"},
      {u8"animation: foo 1s", PropertyId::Animation, "1s foo"},
      {u8"animation: 1s 2s foo", PropertyId::Animation, "1s 2s foo"},
      {u8"animation: none", PropertyId::Animation, "auto"},
      {u8"animation: 1s reverse", PropertyId::Animation, "1s reverse"},
      {u8"animation: 1s normal reverse", PropertyId::Animation, "1s normal reverse"},
      {u8"animation: 1s infinite", PropertyId::Animation, "1s infinite"},
      {u8"animation: 1s foo; animation-timeline: --t", PropertyId::Animation, ""},
      {u8"mask: url(a.svg)", PropertyId::Mask, "url(\"a.svg\")"},
    });
  }

  TEST_CASE_METHOD(ValueFixture, "SerialiseShorthandValue: fonts", "[CSS][Properties][ShorthandSerialiser]")
  {
    CheckShorthands({
      {u8"font: 12px serif", PropertyId::Font, "12px serif"},
      {u8"font: italic bold 12px/30px Georgia, serif", PropertyId::Font,
       "italic bold 12px / 30px Georgia, serif"},
      {u8"font: small-caps 12px serif", PropertyId::Font, "small-caps 12px serif"},
      {u8"font: condensed 12px serif", PropertyId::Font, "condensed 12px serif"},
      {u8"font: 12px serif; font-width: 75%", PropertyId::Font, "condensed 12px serif"},
      {u8"font: 12px serif; font-width: 62.5%", PropertyId::Font, "extra-condensed 12px serif"},
      {u8"font: 12px serif; font-width: 80%", PropertyId::Font, ""},
      {u8"font: 12px serif; font-variant-caps: all-small-caps", PropertyId::Font, ""},
      {u8"font: 12px serif; font-kerning: none", PropertyId::Font, ""},
      {u8"font: caption", PropertyId::Font, "caption"},
      {u8"font: caption; font-size: 12px", PropertyId::Font, ""},
      {u8"font-synthesis: none", PropertyId::FontSynthesis, "none"},
      {u8"font-synthesis: weight", PropertyId::FontSynthesis, "weight"},
      {u8"font-synthesis: style small-caps", PropertyId::FontSynthesis, "style small-caps"},
      {u8"font-synthesis: weight style small-caps", PropertyId::FontSynthesis, "weight style small-caps"},
      {u8"font-variant: normal", PropertyId::FontVariant, "normal"},
      {u8"font-variant: small-caps", PropertyId::FontVariant, "small-caps"},
      {u8"font-variant: none", PropertyId::FontVariant, "none"},
      {u8"font-variant: none; font-variant-caps: small-caps", PropertyId::FontVariant, ""},
      {u8"font: caption", PropertyId::FontVariant, ""},
    });
  }

  TEST_CASE_METHOD(ValueFixture, "SerialiseShorthandValue: grid", "[CSS][Properties][ShorthandSerialiser]")
  {
    CheckShorthands({
      {u8"grid-area: a", PropertyId::GridArea, "a"},
      {u8"grid-area: a / b", PropertyId::GridArea, "a / b"},
      {u8"grid-area: 1 / 2", PropertyId::GridArea, "1 / 2"},
      {u8"grid-area: 1 / 2 / 3 / 4", PropertyId::GridArea, "1 / 2 / 3 / 4"},
      {u8"grid-row: 1", PropertyId::GridRow, "1"},
      {u8"grid-row: a", PropertyId::GridRow, "a"},
      {u8"grid-row: 1 / 3", PropertyId::GridRow, "1 / 3"},
      {u8"grid-template: none", PropertyId::GridTemplate, "none"},
      {u8"grid-template: 10px / 20px", PropertyId::GridTemplate, "10px / 20px"},
      {u8"grid-template: \"a b\" 10px / 1fr 2fr", PropertyId::GridTemplate, "\"a b\" 10px / 1fr 2fr"},
      {u8"grid-template: [x] \"a\" [y] \"b\" 1fr", PropertyId::GridTemplate, "[x] \"a\" [y] \"b\" 1fr"},
      {u8"grid: none", PropertyId::Grid, "none"},
      {u8"grid: 10px / 20px", PropertyId::Grid, "10px / 20px"},
      {u8"grid: auto-flow / 10px", PropertyId::Grid, "none / 10px"},
      {u8"grid: auto-flow dense 5px / 10px", PropertyId::Grid, "auto-flow dense 5px / 10px"},
      {u8"grid: 10px / auto-flow", PropertyId::Grid, "10px / auto-flow"},
      {u8"grid: 10px / auto-flow dense 5px", PropertyId::Grid, "10px / auto-flow dense 5px"},
      {u8"grid: 10px / 20px; grid-auto-flow: column", PropertyId::Grid, ""},
    });
  }

  TEST_CASE_METHOD(ValueFixture, "SerialiseShorthandValue: keywords",
                   "[CSS][Properties][ShorthandSerialiser]")
  {
    CheckShorthands({
      {u8"page-break-after: always", PropertyId::PageBreakAfter, "always"},
      {u8"page-break-after: left", PropertyId::PageBreakAfter, "left"},
      {u8"break-after: column", PropertyId::PageBreakAfter, ""},
      {u8"page-break-inside: avoid", PropertyId::PageBreakInside, "avoid"},
      {u8"break-inside: avoid-column", PropertyId::PageBreakInside, ""},
      {u8"white-space: normal", PropertyId::WhiteSpace, "normal"},
      {u8"white-space: pre", PropertyId::WhiteSpace, "pre"},
      {u8"white-space: pre-wrap", PropertyId::WhiteSpace, "pre-wrap"},
      {u8"white-space: pre-line", PropertyId::WhiteSpace, "pre-line"},
      {u8"white-space: nowrap", PropertyId::WhiteSpace, "nowrap"},
      {u8"white-space: break-spaces", PropertyId::WhiteSpace, "break-spaces"},
      {u8"white-space: break-spaces nowrap", PropertyId::WhiteSpace, "break-spaces nowrap"},
      {u8"text-wrap: wrap", PropertyId::TextWrap, "wrap"},
      {u8"text-wrap: nowrap", PropertyId::TextWrap, "nowrap"},
      {u8"text-wrap: balance", PropertyId::TextWrap, "balance"},
      {u8"text-wrap: nowrap balance", PropertyId::TextWrap, "nowrap balance"},
      {u8"text-box: normal", PropertyId::TextBox, "normal"},
      {u8"text-box: trim-start", PropertyId::TextBox, "trim-start"},
      {u8"text-box: cap alphabetic", PropertyId::TextBox, "cap alphabetic"},
      {u8"text-box: trim-end cap alphabetic", PropertyId::TextBox, "trim-end cap alphabetic"},
      {u8"line-clamp: none", PropertyId::LineClamp, "none"},
      {u8"line-clamp: 3", PropertyId::LineClamp, "3"},
      {u8"position-try: --foo", PropertyId::PositionTry, "--foo"},
      {u8"position-try: most-width --foo", PropertyId::PositionTry, "most-width --foo"},
    });
  }

  TEST_CASE_METHOD(ValueFixture, "SerialiseShorthandValue: offset and timelines",
                   "[CSS][Properties][ShorthandSerialiser]")
  {
    CheckShorthands({
      {u8"offset: none", PropertyId::Offset, "normal"},
      {u8"offset: left top", PropertyId::Offset, "left top"},
      {u8"offset: ray(45deg)", PropertyId::Offset, "ray(45deg)"},
      {u8"offset: ray(45deg) 10px", PropertyId::Offset, "ray(45deg) 10px"},
      {u8"offset: left top ray(45deg) 10px 30deg / left top", PropertyId::Offset,
       "left top ray(45deg) 10px 30deg / left top"},
      {u8"scroll-timeline: --foo", PropertyId::ScrollTimeline, "--foo"},
      {u8"scroll-timeline: --foo x", PropertyId::ScrollTimeline, "--foo x"},
      {u8"scroll-timeline: --a, --b inline", PropertyId::ScrollTimeline, "--a, --b inline"},
      {u8"view-timeline: --foo", PropertyId::ViewTimeline, "--foo"},
      {u8"animation-range: normal", PropertyId::AnimationRange, "normal"},
      {u8"animation-range: entry", PropertyId::AnimationRange, "entry"},
      {u8"animation-range: entry 10% exit 90%", PropertyId::AnimationRange, "entry 10% exit 90%"},
      {u8"animation-range: 10% 90%", PropertyId::AnimationRange, "10% 90%"},
      {u8"animation-range: entry, exit", PropertyId::AnimationRange, "entry, exit"},
    });
  }

  TEST_CASE_METHOD(ValueFixture, "SerialiseShorthandValue: animation-range with mismatched lists",
                   "[CSS][Properties][ShorthandSerialiser]")
  {
    // The longhands can't be represented by the shorthand, so the declaration block keeps them.
    auto properties =
      ParseDeclarations(u8"animation-range-start: normal, normal; animation-range-end: normal");
    CHECK(ValueOf(properties, PropertyId::AnimationRange).empty());
    CHECK(AsText(properties) == "animation-range-start: normal, normal; animation-range-end: normal;");
    CHECK_FALSE(CanSerialise(u8"animation-range-start: normal, normal; animation-range-end: normal",
                             PropertyId::AnimationRange));
    CHECK_FALSE(CanSerialise(u8"animation-range-start: normal; animation-range-end: normal, normal",
                             PropertyId::AnimationRange));
    CHECK_FALSE(
      CanSerialise(u8"animation-range-start: normal, normal; animation-range-end: normal, normal, normal",
                   PropertyId::AnimationRange));
  }
}
