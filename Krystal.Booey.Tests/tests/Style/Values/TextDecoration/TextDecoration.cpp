#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/BuilderTestUtils.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/Values/CSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/NonStandard/WebKitBorderSpacing.hpp"
#include "Krystal.Booey/Style/Values/NonStandard/WebKitLineBoxContain.hpp"
#include "Krystal.Booey/Style/Values/NonStandard/WebKitLocale.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/Style/Values/TextDecoration/TextDecorationLine.hpp"
#include "Krystal.Booey/Style/Values/TextDecoration/TextDecorationThickness.hpp"
#include "Krystal.Booey/Style/Values/TextDecoration/TextEmphasisPosition.hpp"
#include "Krystal.Booey/Style/Values/TextDecoration/TextEmphasisStyle.hpp"
#include "Krystal.Booey/Style/Values/TextDecoration/TextShadow.hpp"
#include "Krystal.Booey/Style/Values/TextDecoration/TextUnderlineOffset.hpp"
#include "Krystal.Booey/Style/Values/TextDecoration/TextUnderlinePosition.hpp"

namespace krys::boo::style::tests
{
  using css::PropertyId;
  using namespace css::Literals;
  using Flag = TextDecorationLine::Flag;

  TEST_CASE("Style TextDecorationLine packing", "[Style][Values][TextDecoration]")
  {
    TextDecorationLine none {css::keywords::None {}};
    CHECK(none.IsNone());
    CHECK_FALSE(none);

    TextDecorationLine lines {EnumSet<Flag> {Flag::Underline, Flag::LineThrough}};
    CHECK(lines.IsFlags());
    CHECK(lines.HasUnderline());
    CHECK(lines.HasLineThrough());
    CHECK_FALSE(lines.HasOverline());
    CHECK(lines.ContainsAny(EnumSet<Flag> {Flag::Overline, Flag::LineThrough}));
    // The packed form is the type bit, then the flags.
    CHECK(lines.ToRaw() == ((1 << 4) | 0x1 | 0x4));
    CHECK(TextDecorationLine::FromRaw(lines.ToRaw()) == lines);

    lines.Remove(Flag::Underline);
    lines.Remove(Flag::LineThrough);
    CHECK(lines.IsNone());

    TextDecorationLine spelling {css::keywords::SpellingError {}};
    CHECK(spelling.IsSpellingError());
    TextDecorationLine combined {Flag::Overline};
    (void)combined.AddOrReplaceIfNotNone(TextDecorationLine {Flag::Blink});
    CHECK(combined.HasOverline());
    CHECK(combined.HasBlink());
    (void)combined.AddOrReplaceIfNotNone(spelling);
    CHECK(combined.IsSpellingError());
  }

  TEST_CASE_METHOD(BuilderFixture, "Style text decoration", "[Style][Values][TextDecoration]")
  {
    SECTION("text-decoration-line")
    {
      auto lines = Convert<TextDecorationLine>(PropertyId::TextDecorationLine, u8"line-through underline");
      CHECK(lines.HasUnderline());
      CHECK(lines.HasLineThrough());
      CHECK(Serialised(lines) == "underline line-through");
      CHECK(Created(lines) == "underline line-through");
      CHECK(Serialised(Convert<TextDecorationLine>(PropertyId::TextDecorationLine, u8"none")) == "none");
      CHECK(Serialised(Convert<TextDecorationLine>(PropertyId::TextDecorationLine, u8"grammar-error"))
            == "grammar-error");
    }

    SECTION("text-decoration-thickness")
    {
      CHECK(Convert<TextDecorationThickness>(PropertyId::TextDecorationThickness, u8"auto").IsAuto());
      CHECK(
        Convert<TextDecorationThickness>(PropertyId::TextDecorationThickness, u8"from-font").IsFromFont());
      auto thickness = Convert<TextDecorationThickness>(PropertyId::TextDecorationThickness, u8"3px");
      CHECK(thickness.IsLength());
      CHECK(Serialised(thickness) == "3px");
      CHECK(Serialised(Convert<TextDecorationThickness>(PropertyId::TextDecorationThickness, u8"10%"))
            == "10%");
    }

    SECTION("text-underline-offset and position")
    {
      CHECK(Convert<TextUnderlineOffset>(PropertyId::TextUnderlineOffset, u8"auto").IsAuto());
      CHECK(Serialised(Convert<TextUnderlineOffset>(PropertyId::TextUnderlineOffset, u8"2px")) == "2px");

      CHECK(Convert<TextUnderlinePosition>(PropertyId::TextUnderlinePosition, u8"auto").IsAuto());
      CHECK(Convert<TextUnderlinePosition>(PropertyId::TextUnderlinePosition, u8"from-font").IsFromFont());
      auto under = Convert<TextUnderlinePosition>(PropertyId::TextUnderlinePosition, u8"under left");
      CHECK(under.IsUnder());
      CHECK(Serialised(under) == "under left");
    }

    SECTION("text-emphasis")
    {
      auto filled = Convert<TextEmphasisStyle>(PropertyId::TextEmphasisStyle, u8"filled");
      // `filled` alone is a circle in horizontal writing.
      CHECK(Serialised(filled) == "circle");
      CHECK(filled.MarkString() == css::CSSOMStringAtom {u8"●"});

      auto open = Convert<TextEmphasisStyle>(PropertyId::TextEmphasisStyle, u8"open triangle");
      CHECK(Serialised(open) == "open triangle");
      CHECK(open.MarkString() == css::CSSOMStringAtom {u8"△"});

      auto custom = Convert<TextEmphasisStyle>(PropertyId::TextEmphasisStyle, u8"\"x\"");
      CHECK(custom.MarkString() == css::CSSOMStringAtom {u8"x"});
      CHECK(Convert<TextEmphasisStyle>(PropertyId::TextEmphasisStyle, u8"none").IsNone());

      CHECK(Serialised(Convert<TextEmphasisPosition>(PropertyId::TextEmphasisPosition, u8"under left"))
            == "under left");
      CHECK(Serialised(Convert<TextEmphasisPosition>(PropertyId::TextEmphasisPosition, u8"over")) == "over");
    }

    SECTION("text-shadow")
    {
      auto shadows = Convert<TextShadows>(PropertyId::TextShadow, u8"1px 2px 3px red, 0 0 blue");
      CHECK(Serialised(shadows) == "rgb(255, 0, 0) 1px 2px 3px, rgb(0, 0, 255) 0px 0px 0px");
      CHECK(Created(shadows) == "rgb(255, 0, 0) 1px 2px 3px, rgb(0, 0, 255) 0px 0px 0px");

      auto from = Convert<TextShadows>(PropertyId::TextShadow, u8"0 0 0 black");
      auto to = Convert<TextShadows>(PropertyId::TextShadow, u8"4px 8px 2px black");
      CHECK(Serialised(Blending<TextShadows> {}.Blend(from, to, Style, Style, BlendingContext {0.5}))
            == "rgb(0, 0, 0) 2px 4px 1px");
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "Style non-standard values", "[Style][Values][NonStandard]")
  {
    CHECK(Convert<WebkitLocale>(PropertyId::WebkitLocale, u8"auto").IsAuto());
    auto locale = Convert<WebkitLocale>(PropertyId::WebkitLocale, u8"\"en-GB\"");
    CHECK_FALSE(locale.IsAuto());
    CHECK(ConvertToPlatform(locale) == css::CSSOMStringAtom {u8"en-GB"});
    // Serialised as a custom ident, although it parses as a string.
    CHECK(Serialised(locale) == "en-GB");

    CHECK(Convert<WebkitLineBoxContain>(PropertyId::WebkitLineBoxContain, u8"none").IsNone());
    CHECK(Serialised(Convert<WebkitLineBoxContain>(PropertyId::WebkitLineBoxContain, u8"block inline"))
          == "block inline");

    CHECK(Convert<WebkitBorderSpacing>(PropertyId::WebkitBorderHorizontalSpacing, u8"4px") == 4_css_px);
  }
}
