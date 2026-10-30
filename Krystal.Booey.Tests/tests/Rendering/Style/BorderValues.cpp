#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Rendering/Style/BorderValue.hpp"
#include "Krystal.Booey/Rendering/Style/CollapsedBorderValue.hpp"
#include "Krystal.Booey/Rendering/Style/CounterDirectives.hpp"
#include "Krystal.Booey/Rendering/Style/OutlineValue.hpp"
#include "Krystal.Booey/Rendering/Style/PositionTryOrder.hpp"

namespace krys::boo::tests
{
  TEST_CASE("Border values", "[Rendering][Style]")
  {
    BorderValue initial;
    CHECK(initial.Color == style::Color::GetCurrentColor());
    CHECK(static_cast<BorderStyle>(initial.Style) == BorderStyle::None);
    // A `none` border has no width to paint.
    CHECK_FALSE(initial.NonZero());
    CHECK_FALSE(initial.IsVisible());
    CHECK_FALSE(initial.HasVisibleStyle());

    BorderValue solid;
    solid.Style = static_cast<uint8>(BorderStyle::Solid);
    solid.Color = style::Color {boo::Color {boo::Color::black}};
    CHECK(solid.NonZero());
    CHECK(solid.HasVisibleStyle());
    CHECK(solid.IsVisible());
    CHECK_FALSE(solid == initial);

    // Transparent and hidden borders aren't visible.
    auto transparent = solid;
    transparent.Color = style::Color {boo::Color {boo::Color::transparentBlack}};
    CHECK(transparent.IsTransparent());
    CHECK_FALSE(transparent.IsVisible());

    auto hidden = solid;
    hidden.Style = static_cast<uint8>(BorderStyle::Hidden);
    CHECK(hidden.HasHiddenStyle());
    CHECK_FALSE(hidden.IsVisible());

    // An unresolved (current) colour isn't known to be transparent.
    CHECK_FALSE(initial.IsTransparent());

    SECTION("collapsed borders")
    {
      CollapsedBorderValue none;
      CHECK_FALSE(none.Exists());
      CHECK(none.Width() == LayoutUnit {});

      CollapsedBorderValue cell {solid, boo::Color {boo::Color::black}, BorderPrecedence::Cell,
                                 style::ZoomFactor {1, 1}};
      CHECK(cell.Exists());
      CHECK(cell.Style() == BorderStyle::Solid);
      CHECK(cell.Width() == LayoutUnit {3});
      CHECK(cell.Precedence() == BorderPrecedence::Cell);
      CHECK_FALSE(cell.IsTransparent());

      CollapsedBorderValue row {solid, boo::Color {boo::Color::white}, BorderPrecedence::Row,
                                style::ZoomFactor {1, 1}};
      CHECK_FALSE(cell.IsSameIgnoringColor(row));
      CollapsedBorderValue otherCell {solid, boo::Color {boo::Color::white}, BorderPrecedence::Cell,
                                      style::ZoomFactor {1, 1}};
      CHECK(cell.IsSameIgnoringColor(otherCell));

      // Hidden borders have no width.
      CHECK(CollapsedBorderValue {hidden, boo::Color {boo::Color::black}, BorderPrecedence::Cell,
                                  style::ZoomFactor {1, 1}}
              .Width()
            == LayoutUnit {});

      CHECK(CollapsedBorderValue::AdjustedCollapsedBorderWidth(3, 1, false) == LayoutUnit {1});
      CHECK(CollapsedBorderValue::AdjustedCollapsedBorderWidth(3, 1, true) == LayoutUnit {2});
    }
  }

  TEST_CASE("Outline values", "[Rendering][Style]")
  {
    OutlineValue initial;
    CHECK(static_cast<OutlineStyle>(initial.OutlineStyle) == OutlineStyle::None);
    CHECK_FALSE(initial.NonZero());
    CHECK_FALSE(initial.IsVisible());

    OutlineValue outline;
    outline.OutlineStyle = static_cast<uint8>(OutlineStyle::Auto);
    outline.OutlineColor = style::Color {boo::Color {boo::Color::black}};
    CHECK(outline.IsVisible());

    CHECK(ToBorderStyle(OutlineStyle::Dashed) == BorderStyle::Dashed);
    CHECK(ToBorderStyle(OutlineStyle::None) == BorderStyle::None);
    // `auto` has no border style.
    CHECK_FALSE(ToBorderStyle(OutlineStyle::Auto));
  }

  TEST_CASE("Counter directives", "[Rendering][Style]")
  {
    CounterDirectiveMap map;
    map.Map[css::CSSOMStringAtom {u8"chapter"}] = CounterDirectives {.ResetValue = 1};
    CHECK_FALSE(map == CounterDirectiveMap {});

    auto copy = map;
    CHECK(copy == map);
    copy.Map[css::CSSOMStringAtom {u8"chapter"}].IncrementValue = 2;
    CHECK_FALSE(copy == map);
  }

  TEST_CASE("Position try order axes", "[Rendering][Style]")
  {
    WritingMode horizontal {StyleWritingMode::HorizontalTb, TextDirection::LTR, TextOrientation::Mixed};
    WritingMode vertical {StyleWritingMode::VerticalLr, TextDirection::LTR, TextOrientation::Mixed};

    using style::PositionTryOrder;
    CHECK(style::BoxAxisForPositionTryOrder(PositionTryOrder::MostWidth, horizontal)
          == LogicalBoxAxis::Inline);
    CHECK(style::BoxAxisForPositionTryOrder(PositionTryOrder::MostWidth, vertical) == LogicalBoxAxis::Block);
    CHECK(style::BoxAxisForPositionTryOrder(PositionTryOrder::MostHeight, horizontal)
          == LogicalBoxAxis::Block);
    CHECK(style::BoxAxisForPositionTryOrder(PositionTryOrder::MostBlockSize, vertical)
          == LogicalBoxAxis::Block);
    CHECK(style::BoxAxisForPositionTryOrder(PositionTryOrder::MostInlineSize, vertical)
          == LogicalBoxAxis::Inline);
  }
}
