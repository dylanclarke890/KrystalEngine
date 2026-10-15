#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Rendering/Style/BorderData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleBoxData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleInheritedData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleMiscNonInheritedData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleNonInheritedData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleRareInheritedData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleRareNonInheritedData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleSurroundData.hpp"
#include "Krystal.Booey/Rendering/Style/SVGRenderStyle.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyle.hpp"

namespace krys::boo::tests
{
  using namespace css::Literals;
  using style::ComputedStyle;

  TEST_CASE("Style data groups: initial values", "[Rendering][Style]")
  {
    auto box = StyleBoxData::Create();
    CHECK(box->Width == ComputedStyle::InitialWidth());
    CHECK(box->MaxHeight == ComputedStyle::InitialMaxHeight());

    auto inherited = StyleInheritedData::Create();
    CHECK(inherited->Color == ComputedStyle::InitialColor());
    CHECK(inherited->BorderHorizontalSpacing == ComputedStyle::InitialBorderHorizontalSpacing());

    auto rareInherited = StyleRareInheritedData::Create();
    CHECK(rareInherited->TabSize == ComputedStyle::InitialTabSize());
    CHECK(rareInherited->MathDepth == ComputedStyle::InitialMathDepth());

    auto rareNonInherited = StyleRareNonInheritedData::Create();
    CHECK(rareNonInherited->Zoom == ComputedStyle::InitialZoom());
    CHECK(rareNonInherited->AnchorScope == ComputedStyle::InitialAnchorScope());
    CHECK(rareNonInherited->ScrollTimelines.isEmpty());

    auto misc = StyleMiscNonInheritedData::Create();
    CHECK(misc->Opacity == ComputedStyle::InitialOpacity());
  }

  TEST_CASE("Style data groups: copies and equality", "[Rendering][Style]")
  {
    auto data = StyleRareInheritedData::Create();
    auto copy = data->Copy();
    CHECK(copy.get() != data.get());
    CHECK(*copy == *data);

    copy->TabSize = style::TabSize {4_css_number};
    CHECK_FALSE(*copy == *data);

    // Nested groups compare by value, not identity.
    auto nonInherited = StyleNonInheritedData::Create();
    auto other = StyleNonInheritedData::Create();
    CHECK(*nonInherited == *other);
  }

  TEST_CASE("Style data groups: copy on write", "[Rendering][Style]")
  {
    DataRef<StyleNonInheritedData> first {StyleNonInheritedData::Create()};
    auto second = first;
    CHECK(first.Ptr() == second.Ptr());

    // Writing through one reference copies the shared data first, down the nested groups that are written.
    second.Access().BoxData.Access().Width = style::PreferredSize {10_css_px};
    CHECK(first.Ptr() != second.Ptr());
    CHECK(first->BoxData.Ptr() != second->BoxData.Ptr());
    CHECK(first->RareData.Ptr() == second->RareData.Ptr());
    CHECK(first->BoxData->Width == ComputedStyle::InitialWidth());
    CHECK(second->BoxData->Width == style::PreferredSize {10_css_px});
    CHECK_FALSE(first == second);
  }

  TEST_CASE("Style data groups: fast path inheritance", "[Rendering][Style]")
  {
    auto parent = StyleInheritedData::Create();
    parent->Color = boo::Color {boo::Color::white};
    parent->LineHeight = style::LineHeight {20_css_px};

    auto child = StyleInheritedData::Create();
    CHECK_FALSE(child->FastPathInheritedEqual(*parent));
    CHECK_FALSE(child->NonFastPathInheritedEqual(*parent));

    child->FastPathInheritFrom(*parent);
    CHECK(child->FastPathInheritedEqual(*parent));
    CHECK(child->Color == boo::Color {boo::Color::white});
    // The line height isn't inherited through the fast path.
    CHECK_FALSE(child->NonFastPathInheritedEqual(*parent));
  }

  TEST_CASE("Border data", "[Rendering][Style]")
  {
    BorderData border;
    CHECK_FALSE(border.HasBorder());
    CHECK_FALSE(border.HasVisibleBorder());
    CHECK_FALSE(border.HasBorderImage());
    CHECK_FALSE(border.HasBorderRadius());
    // A `none` border uses no width.
    CHECK(border.BorderTopWidth() == style::LineWidth {0_css_px});

    border.Styles().SetTop(static_cast<uint8>(BorderStyle::Solid));
    CHECK(static_cast<BorderStyle>(border.Styles().Top()) == BorderStyle::Solid);
    CHECK(static_cast<BorderStyle>(border.Top().Style) == BorderStyle::Solid);
    CHECK(border.HasBorder());
    CHECK(border.BorderTopWidth() == border.Top().Width);
    CHECK(border.BorderWidth().Top() == border.Top().Width);
    CHECK(border.BorderBottomWidth() == style::LineWidth {0_css_px});

    border.Colors().SetLeft(style::Color {boo::Color {boo::Color::black}});
    CHECK(border.Left().Color == style::Color {boo::Color {boo::Color::black}});
    CHECK(std::as_const(border).Colors().Left() == style::Color {boo::Color {boo::Color::black}});
    // Only the visible top edge uses the current colour.
    CHECK(border.ContainsCurrentColor());

    border.TopLeftRadius() = style::BorderRadiusValue {4_css_px, 4_css_px};
    CHECK(border.HasBorderRadius());

    auto copy = border;
    CHECK(copy == border);
    copy.Bottom().Style = static_cast<uint8>(BorderStyle::Dashed);
    CHECK_FALSE(copy == border);
  }

  TEST_CASE("SVG render style", "[Rendering][Style]")
  {
    auto defaults = SVGRenderStyle::CreateDefaultStyle();
    auto style = SVGRenderStyle::Create();
    // New styles share the groups of a single default style.
    CHECK(style->FillData.Ptr() == SVGRenderStyle::Create()->FillData.Ptr());
    CHECK(style->FillData.Ptr() != defaults->FillData.Ptr());
    CHECK(*style == *defaults);
    CHECK(style->InheritedEqual(*defaults));
    CHECK(style->NonInheritedEqual(*defaults));

    auto copy = style->Copy();
    copy->InheritedFlags.FillRule = ToUnderlying(WindRule::EvenOdd);
    CHECK_FALSE(copy->InheritedEqual(*style));
    CHECK(copy->NonInheritedEqual(*style));

    style->InheritFrom(*copy);
    CHECK(style->InheritedEqual(*copy));
  }
}
