#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/BuilderTestUtils.hpp"
#include "Krystal.Booey/CSS/PrimitiveValueMappings.hpp"
#include "Krystal.Booey/Style/Values/Box/Margin.hpp"
#include "Krystal.Booey/Style/Values/Box/MarginTrim.hpp"
#include "Krystal.Booey/Style/Values/Box/Padding.hpp"
#include "Krystal.Booey/Style/Values/Position/Inset.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"

namespace krys::boo::style::tests
{
  using css::PropertyId;
  using namespace css::Literals;

  TEST_CASE_METHOD(BuilderFixture, "Style Margin", "[Style][Values][Box]")
  {
    SECTION("Keywords, lengths and percentages")
    {
      CHECK(Convert<MarginEdge>(PropertyId::MarginTop, u8"auto").IsAuto());
      CHECK(Convert<MarginEdge>(PropertyId::MarginTop, u8"10px") == MarginEdge {10_css_px});
      CHECK(Convert<MarginEdge>(PropertyId::MarginTop, u8"2em") == MarginEdge {32_css_px});
      CHECK(Convert<MarginEdge>(PropertyId::MarginTop, u8"25%") == MarginEdge {25_css_percentage});
      CHECK(Convert<MarginEdge>(PropertyId::MarginTop, u8"-4px") == MarginEdge {-4_css_px});
      CHECK(Convert<MarginEdge>(PropertyId::MarginTop, u8"calc(10% + 1px)").IsCalculated());
      CHECK_FALSE(IsInvalid());
    }

    SECTION("Serialisation")
    {
      CHECK(Serialised(MarginEdge {css::keywords::Auto {}}) == "auto");
      CHECK(Serialised(MarginEdge {10_css_px}) == "10px");
      CHECK(Serialised(MarginBox {MarginEdge {1_css_px}, MarginEdge {1_css_px}, MarginEdge {1_css_px},
                                  MarginEdge {1_css_px}})
            == "1px");
    }

    SECTION("Quirky ems keep their quirk")
    {
      auto quirky =
        ConvertCSSValueToStyle<MarginEdge>(*State, *css::PrimitiveValue::Create(1, css::UnitType::QuirkyEm));
      CHECK(quirky.HasQuirk());
    }

    SECTION("A non-primitive value is invalid at computed-value time")
    {
      CHECK(ConvertCSSValueToStyle<MarginEdge>(*State, *css::ValueList::CreateSpaceSeparated()).IsAuto());
      CHECK(IsInvalid());
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "Style MarginTrim", "[Style][Values][Box]")
  {
    // `none` isn't one of the converted keywords, so it's invalid at computed-value time and computes to the
    // initial `none`.
    CHECK(Convert<MarginTrim>(PropertyId::MarginTrim, u8"none").IsNone());
    CHECK(IsInvalid());
    Reset();

    auto block = Convert<MarginTrim>(PropertyId::MarginTrim, u8"block");
    CHECK(block.ContainsAll({MarginTrimSide::BlockStart, MarginTrimSide::BlockEnd}));
    CHECK_FALSE(block.ContainsAny({MarginTrimSide::InlineStart, MarginTrimSide::InlineEnd}));
    CHECK(Serialised(block) == "block");

    auto sides = Convert<MarginTrim>(PropertyId::MarginTrim, u8"inline-start block-end");
    CHECK(sides.ContainsOnly({MarginTrimSide::InlineStart, MarginTrimSide::BlockEnd}));
    CHECK(Serialised(sides) == "inline-start block-end");

    CHECK(Serialised(MarginTrim {MarginTrimSide::BlockStart, MarginTrimSide::BlockEnd,
                                 MarginTrimSide::InlineStart, MarginTrimSide::InlineEnd})
          == "block inline");
    CHECK(Serialised(MarginTrim {css::keywords::None {}}) == "none");
    CHECK_FALSE(IsInvalid());
  }

  TEST_CASE_METHOD(BuilderFixture, "Style Padding and Inset", "[Style][Values][Box]")
  {
    CHECK(Convert<PaddingEdge>(PropertyId::PaddingLeft, u8"3px") == PaddingEdge {3_css_px});
    CHECK(Convert<PaddingEdge>(PropertyId::PaddingLeft, u8"10%") == PaddingEdge {10_css_percentage});
    CHECK(Convert<InsetEdge>(PropertyId::Top, u8"auto").IsAuto());
    CHECK(Convert<InsetEdge>(PropertyId::Top, u8"-1em") == InsetEdge {-16_css_px});
    CHECK(Serialised(InsetBox {InsetEdge {css::keywords::Auto {}}, InsetEdge {1_css_px},
                               InsetEdge {css::keywords::Auto {}}, InsetEdge {1_css_px}})
          == "auto 1px");
  }
}
