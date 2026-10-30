#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/StyleBoxData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleNonInheritedData.hpp"

namespace krys::boo::tests
{
  using namespace css::Literals;
  using style::ComputedStyle;

  namespace
  {
    const boo::Color Red {SRGBA<uint8> {255, 0, 0}};
    const boo::Color Blue {SRGBA<uint8> {0, 0, 255}};
  }

  TEST_CASE("RenderStyle: initial values", "[Rendering][Style][RenderStyle]")
  {
    auto style = RenderStyle::Create();
    CHECK(style.Display() == DisplayType::Inline);
    CHECK(style.GetColor() == boo::Color {boo::Color::black});
    CHECK(style.GetOpacity() == ComputedStyle::InitialOpacity());
    CHECK(style.Width() == ComputedStyle::InitialWidth());
    CHECK(style.MarginLeft() == ComputedStyle::InitialMarginLeft());
    CHECK(static_cast<BorderStyle>(style.BorderTopStyle()) == BorderStyle::None);
    CHECK(style.GetMathDepth() == ComputedStyle::InitialMathDepth());
    CHECK(style.GetWritingMode().IsHorizontal());
    CHECK(style.GetWritingMode().IsBidiLTR());
    CHECK_FALSE(style.HasExplicitlySetColor());
    CHECK_FALSE(style.GetPseudoElementType());
    // A `none` border uses no width.
    CHECK(style.BorderTopWidth() == style::LineWidth {0_css_px});

    // Every style starts as a copy of the default style.
    CHECK(style == RenderStyle::DefaultStyleSingleton());
  }

  TEST_CASE("RenderStyle: getters and setters", "[Rendering][Style][RenderStyle]")
  {
    auto style = RenderStyle::Create();

    style.SetDisplay(DisplayType::Block);
    CHECK(style.Display() == DisplayType::Block);
    CHECK(style.OriginalDisplay() == DisplayType::Block);

    style.SetColor(boo::Color {Red});
    style.SetHasExplicitlySetColor(true);
    CHECK(style.GetColor() == Red);
    CHECK(style.HasExplicitlySetColor());

    style.SetWidth(style::PreferredSize {10_css_px});
    CHECK(style.Width() == style::PreferredSize {10_css_px});

    style.SetOpacity(style::Opacity {0.5f});
    CHECK(style.GetOpacity() == style::Opacity {0.5f});

    style.SetMathDepth(style::MathDepth {3});
    CHECK(style.GetMathDepth() == style::MathDepth {3});

    // Border edges are stored together per edge.
    style.SetBorderTopStyle(BorderStyle::Solid);
    style.SetBorderTopColor(style::Color {boo::Color {Blue}});
    CHECK(static_cast<BorderStyle>(style.BorderTopStyle()) == BorderStyle::Solid);
    CHECK(style.BorderTopColor() == style::Color {boo::Color {Blue}});
    CHECK(style.BorderTop().Color == style::Color {boo::Color {Blue}});
    CHECK(style.HasBorder());

    // Visited link colours are separate.
    style.SetVisitedLinkBackgroundColor(style::Color {boo::Color {Blue}});
    CHECK(style.VisitedLinkBackgroundColor() == style::Color {boo::Color {Blue}});
    CHECK(style.BackgroundColor() == ComputedStyle::InitialBackgroundColor());

    // Flags that aren't properties.
    style.SetUsesViewportUnits();
    CHECK(style.UsesViewportUnits());
    style.SetHasAttrContent();
    CHECK(style.HasAttrContent());
  }

  TEST_CASE("RenderStyle: logical properties follow the writing mode", "[Rendering][Style][RenderStyle]")
  {
    auto style = RenderStyle::Create();
    style.SetMarginStart(style::MarginEdge {4_css_px});
    CHECK(style.MarginLeft() == style::MarginEdge {4_css_px});
    CHECK(style.MarginStart() == style::MarginEdge {4_css_px});

    CHECK(style.SetDirection(TextDirection::RTL));
    style.SetMarginStart(style::MarginEdge {6_css_px});
    CHECK(style.MarginRight() == style::MarginEdge {6_css_px});

    CHECK(style.SetWritingMode(StyleWritingMode::VerticalLr));
    CHECK(style.GetWritingMode().IsVertical());
    style.SetMarginBefore(style::MarginEdge {8_css_px});
    CHECK(style.MarginLeft() == style::MarginEdge {8_css_px});
    // The logical width is the physical height in a vertical writing mode.
    style.SetLogicalWidth(style::PreferredSize {20_css_px});
    CHECK(style.Height() == style::PreferredSize {20_css_px});
    CHECK(style.LogicalWidth() == style::PreferredSize {20_css_px});
  }

  TEST_CASE("RenderStyle: clones share their data until written", "[Rendering][Style][RenderStyle]")
  {
    auto style = RenderStyle::Create();
    style.SetWidth(style::PreferredSize {10_css_px});

    auto clone = RenderStyle::Clone(style);
    CHECK(clone == style);
    CHECK(&clone.NonInheritedData() == &style.NonInheritedData());

    clone.SetWidth(style::PreferredSize {20_css_px});
    CHECK(&clone.NonInheritedData() != &style.NonInheritedData());
    CHECK(style.Width() == style::PreferredSize {10_css_px});
    CHECK_FALSE(clone == style);
    CHECK(clone.InheritedEqual(style));
    CHECK_FALSE(clone.NonInheritedEqual(style));

    // Replacing returns the old style.
    auto old = clone.Replace(RenderStyle::Clone(style));
    CHECK(old.Width() == style::PreferredSize {20_css_px});
    CHECK(clone == style);
  }

  TEST_CASE("RenderStyle: inheritance", "[Rendering][Style][RenderStyle]")
  {
    auto parent = RenderStyle::Create();
    parent.SetColor(boo::Color {Red});
    parent.SetWidth(style::PreferredSize {10_css_px});
    parent.SetDirection(TextDirection::RTL);

    auto child = RenderStyle::Create();
    child.InheritFrom(parent);
    CHECK(child.GetColor() == Red);
    CHECK(child.GetWritingMode().IsBidiRTL());
    CHECK(child.Width() == ComputedStyle::InitialWidth());
    CHECK(child.InheritedEqual(parent));
    CHECK_FALSE(child.NonInheritedEqual(parent));

    auto copy = RenderStyle::Create();
    copy.CopyNonInheritedFrom(parent);
    CHECK(copy.Width() == style::PreferredSize {10_css_px});
    CHECK(copy.GetColor() == boo::Color {boo::Color::black});
    CHECK(copy.NonInheritedEqual(parent));

    auto anonymous = RenderStyle::CreateAnonymousStyleWithDisplay(parent, DisplayType::Block);
    CHECK(anonymous.Display() == DisplayType::Block);
    CHECK(anonymous.GetColor() == Red);

    // Fast path inheritance copies only visibility and the inherited data's fast path properties.
    auto fast = RenderStyle::Create();
    CHECK_FALSE(fast.FastPathInheritedEqual(parent));
    fast.FastPathInheritFrom(parent);
    CHECK(fast.FastPathInheritedEqual(parent));
    CHECK(fast.GetColor() == Red);
  }

  TEST_CASE("RenderStyle: pseudo-element styles", "[Rendering][Style][RenderStyle]")
  {
    auto style = RenderStyle::Create();
    CHECK_FALSE(style.HasCachedPseudoStyles());

    auto before = RenderStyle::CreatePtr();
    before->SetPseudoElementIdentifier(style::PseudoElementIdentifier {PseudoElementType::Before});
    before->SetColor(boo::Color {Blue});
    auto *cached = style.AddCachedPseudoStyle(krys::move(before));
    REQUIRE(cached);
    CHECK(style.HasCachedPseudoStyles());
    CHECK(style.GetCachedPseudoStyle(style::PseudoElementIdentifier {PseudoElementType::Before}) == cached);
    CHECK_FALSE(style.GetCachedPseudoStyle(style::PseudoElementIdentifier {PseudoElementType::After}));
    CHECK(cached->GetPseudoElementType() == PseudoElementType::Before);

    // Cloning includes the pseudo-element styles only when asked.
    CHECK_FALSE(RenderStyle::Clone(style).HasCachedPseudoStyles());
    auto clone = RenderStyle::CloneIncludingPseudoElements(style);
    const auto *clonedBefore =
      clone.GetCachedPseudoStyle(style::PseudoElementIdentifier {PseudoElementType::Before});
    REQUIRE(clonedBefore);
    CHECK(clonedBefore != cached);
    CHECK(clonedBefore->GetColor() == Blue);

    auto inheriting = RenderStyle::CreateStyleInheritingFromPseudoStyle(*cached);
    CHECK(inheriting.GetColor() == Blue);
  }

  TEST_CASE("RenderStyle: colours", "[Rendering][Style][RenderStyle]")
  {
    auto style = RenderStyle::Create();
    style.SetColor(boo::Color {Red});

    // `currentcolor` resolves to `color`.
    CHECK(style.ColorResolvingCurrentColor(css::PropertyId::BorderTopColor, false) == Red);
    CHECK(style.ColorResolvingCurrentColor(style::Color::GetCurrentColor()) == Red);
    CHECK(style.ColorResolvingCurrentColor(css::PropertyId::Color, false) == Red);

    // Without an explicit stroke colour the stroke uses `color`.
    CHECK(style.UsedStrokeColor() == Red);
    CHECK(style.UsedStrokeWidth(IntSize {100, 100}) == 0);

    style.SetBorderTopColor(style::Color {boo::Color {Blue}});
    CHECK(style.ColorResolvingCurrentColor(css::PropertyId::BorderTopColor, false) == Blue);
    // Logical properties resolve through the writing mode.
    CHECK(style.ColorResolvingCurrentColor(css::PropertyId::BorderBlockStartColor, false) == Blue);

    // Outside visited links the visited colours aren't used.
    style.SetVisitedLinkBorderTopColor(style::Color {boo::Color {boo::Color::white}});
    CHECK(style.VisitedDependentColor(css::PropertyId::BorderTopColor) == Blue);
    style.SetInsideLink(InsideLink::InsideVisited);
    CHECK(style.VisitedDependentColor(css::PropertyId::BorderTopColor) == boo::Color {boo::Color::white});
    CHECK(style.VisitedDependentColor(css::PropertyId::BorderTopColor, PaintBehavior::DontShowVisitedLinks)
          == Blue);
  }

  TEST_CASE("RenderStyle: zoom adjustment", "[Rendering][Style][RenderStyle]")
  {
    auto style = RenderStyle::Create();
    CHECK(AdjustFloatForAbsoluteZoom(10, style) == 10);
    CHECK(ApplyZoom(10, style) == 10);
    std::ignore = style.SetUsedZoom(2);
    CHECK(AdjustFloatForAbsoluteZoom(10, style) == 5);
    CHECK(AdjustForAbsoluteZoom(10, style) == 5);
    CHECK(ApplyZoom(10, style) == 20);
  }
}
