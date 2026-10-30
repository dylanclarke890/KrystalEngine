#include "Krystal.Booey/CSS/PrimitiveValueMappings.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    template <typename T>
    KRYS_NODISCARD bool RoundTrips(std::initializer_list<T> values)
    {
      for (auto value : values)
      {
        if (FromValueId<T>(boo::ToValueId(value)) != value)
        {
          return false;
        }
      }
      return true;
    }

    template <typename T>
    KRYS_NODISCARD ValueId Represented(T value)
    {
      return style::valueRepresentation(value, []<ValueId Id>(KeywordValueConstant<Id>) { return Id; });
    }
  }

  TEST_CASE_METHOD(ValueFixture, "PrimitiveValueMappings", "[CSS][PrimitiveValueMappings]")
  {
    SECTION("Generated mappings")
    {
      CHECK(ToValueId(ColumnFill::Balance) == ValueId::Balance);
      CHECK(FromValueId<ColumnFill>(ValueId::Auto) == ColumnFill::Auto);
      CHECK(RoundTrips({ColumnSpan::All, ColumnSpan::None}));
      CHECK(RoundTrips({Visibility::Visible, Visibility::Hidden, Visibility::Collapse}));
      CHECK(RoundTrips({FlexDirection::Row, FlexDirection::RowReverse, FlexDirection::Column,
                        FlexDirection::ColumnReverse}));
      CHECK(RoundTrips({TextOrientation::Mixed, TextOrientation::Upright, TextOrientation::Sideways}));
      CHECK(RoundTrips({PointerEvents::Auto, PointerEvents::None, PointerEvents::Stroke}));
      CHECK(RoundTrips({ScrollAxis::Block, ScrollAxis::Inline, ScrollAxis::X, ScrollAxis::Y}));
      CHECK(
        RoundTrips({CompositeOperation::Replace, CompositeOperation::Add, CompositeOperation::Accumulate}));
      CHECK(RoundTrips({FontVariantEmoji::Normal, FontVariantEmoji::Text, FontVariantEmoji::Emoji,
                        FontVariantEmoji::Unicode}));
      CHECK(RoundTrips({BlendMode::Normal, BlendMode::Multiply, BlendMode::PlusLighter}));
    }

    SECTION("Value representations")
    {
      CHECK(Represented(ColumnFill::Balance) == ValueId::Balance);
      CHECK(Represented(Visibility::Collapse) == ValueId::Collapse);
      CHECK(Represented(OverflowWrap::Anywhere) == ValueId::Anywhere);
    }

    SECTION("Hand-written mappings")
    {
      CHECK(RoundTrips({BorderStyle::None, BorderStyle::Hidden, BorderStyle::Inset, BorderStyle::Groove,
                        BorderStyle::Outset, BorderStyle::Ridge, BorderStyle::Dotted, BorderStyle::Dashed,
                        BorderStyle::Solid, BorderStyle::Double}));
      CHECK(RoundTrips({DisplayType::Inline, DisplayType::Block, DisplayType::ListItem,
                        DisplayType::InlineBlock, DisplayType::Table, DisplayType::Flex, DisplayType::Grid,
                        DisplayType::None, DisplayType::Contents}));
      CHECK(RoundTrips(
        {CursorType::Auto, CursorType::Pointer, CursorType::ZoomOut, CursorType::Copy, CursorType::None}));
      CHECK(RoundTrips({StyleAppearance::None, StyleAppearance::Auto, StyleAppearance::Checkbox,
                        StyleAppearance::TextField}));
      CHECK(RoundTrips({UnicodeBidi::Normal, UnicodeBidi::Embed, UnicodeBidi::Override, UnicodeBidi::Isolate,
                        UnicodeBidi::Plaintext, UnicodeBidi::IsolateOverride}));
      CHECK(RoundTrips({StyleWritingMode::HorizontalTb, StyleWritingMode::VerticalRl,
                        StyleWritingMode::VerticalLr, StyleWritingMode::SidewaysRl,
                        StyleWritingMode::SidewaysLr}));
      CHECK(RoundTrips({TextDirection::LTR, TextDirection::RTL}));
      CHECK(RoundTrips({WindRule::NonZero, WindRule::EvenOdd}));
      CHECK(RoundTrips({CSSBoxType::MarginBox, CSSBoxType::BorderBox, CSSBoxType::PaddingBox,
                        CSSBoxType::ContentBox, CSSBoxType::FillBox, CSSBoxType::StrokeBox,
                        CSSBoxType::ViewBox}));
      CHECK(RoundTrips({ImageRendering::Auto, ImageRendering::CrispEdges, ImageRendering::Pixelated}));
      CHECK(RoundTrips({Kerning::Auto, Kerning::Normal, Kerning::NoShift}));
      CHECK(RoundTrips({FontVariantCaps::Normal, FontVariantCaps::Small, FontVariantCaps::AllSmall,
                        FontVariantCaps::Petite, FontVariantCaps::AllPetite, FontVariantCaps::Unicase,
                        FontVariantCaps::Titling}));

      // Aliases map to their canonical value.
      CHECK(FromValueId<Overflow>(ValueId::Overlay) == Overflow::Auto);
      CHECK(FromValueId<Kerning>(ValueId::None) == Kerning::NoShift);
      CHECK(FromValueId<BreakBetween>(ValueId::Left) == BreakBetween::LeftPage);

      CHECK(FromValueId<css::FontTechnology>(ValueId::Palettes) == css::FontTechnology::Palettes);
      CHECK(FromValueId<css::FontTechnology>(ValueId::Auto) == css::FontTechnology::Invalid);
    }

    SECTION("FromCSSValue")
    {
      CHECK(FromCSSValue<Visibility>(*PrimitiveValue::Create(ValueId::Hidden)) == Visibility::Hidden);
    }
  }
}
