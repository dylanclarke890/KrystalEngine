#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/CSS/Calc/RandomCachingKey.hpp"
#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/CSS/Values/Color/KeywordColor.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Style/Values/Color/Color.hpp"
#include "Krystal.Booey/Style/Values/Viewport/ZoomPrimitives.hpp"
#include "Krystal.Core/Assert.hpp"

// FIXME: Placeholder definitions; see `Style/Seam.hpp`. Only `RenderStyle` can be constructed, so only its
// colour, zoom and font members and the document functions can be reached.

namespace krys::boo
{
  namespace
  {
    // The initial `font-size` (`medium`).
    constexpr float InitialFontSize = 16.f;

    // FIXME: There's no font data, so the metrics use the fallbacks of css-values-4 where it gives one (an
    // `ex` or `ch` is 0.5em, an `ic` is 1em), the em box for the ascent and 1.2em for a `normal` line height.
    constexpr int32 InitialFontAscent = 16;
    constexpr int32 InitialFontLineSpacing = 19;

    const FontMetrics InitialFontMetrics {};
    const FontCascadeDescription InitialFontDescription {};
    const FontCascade InitialFontCascade {};
  }

  int FontMetrics::IntAscent() const noexcept
  {
    return InitialFontAscent;
  }

  int FontMetrics::IntLineSpacing() const noexcept
  {
    return InitialFontLineSpacing;
  }

  Maybe<float> FontMetrics::XHeight() const noexcept
  {
    return null;
  }

  Maybe<float> FontMetrics::CapHeight() const noexcept
  {
    return null;
  }

  Maybe<float> FontMetrics::ZeroWidth() const noexcept
  {
    return null;
  }

  Maybe<float> FontMetrics::IdeogramWidth() const noexcept
  {
    return InitialFontSize;
  }

  float FontCascadeDescription::ComputedSize() const noexcept
  {
    return InitialFontSize;
  }

  float FontCascadeDescription::ComputedSizeForRangeZoomOption(css::RangeZoomOptions) const noexcept
  {
    return InitialFontSize;
  }

  float FontCascadeDescription::SpecifiedSize() const noexcept
  {
    return InitialFontSize;
  }

  const FontCascadeDescription &FontCascade::GetFontDescription() const noexcept
  {
    return InitialFontDescription;
  }

  const FontMetrics &FontCascade::MetricsOfPrimaryFont() const noexcept
  {
    return InitialFontMetrics;
  }

  float FontCascade::ZeroWidth() const noexcept
  {
    return InitialFontSize / 2;
  }

  const boo::Color &RenderStyle::GetColor() const noexcept
  {
    return _color;
  }

  void RenderStyle::SetColor(const boo::Color &color) noexcept
  {
    _color = color;
  }

  const boo::Color &RenderStyle::VisitedLinkColor() const noexcept
  {
    return _visitedLinkColor;
  }

  void RenderStyle::SetVisitedLinkColor(const boo::Color &color) noexcept
  {
    _visitedLinkColor = color;
  }

  boo::Color RenderStyle::ColorResolvingCurrentColor(const style::Color &color,
                                                     bool visitedLink) const noexcept
  {
    return color.ResolveColor(visitedLink ? VisitedLinkColor() : GetColor());
  }

  bool RenderStyle::HasDarkOnlyColorScheme() const noexcept
  {
    return _hasDarkOnlyColorScheme;
  }

  void RenderStyle::SetHasDarkOnlyColorScheme(bool value) noexcept
  {
    _hasDarkOnlyColorScheme = value;
  }

  WritingMode RenderStyle::GetWritingMode() const noexcept
  {
    krys_unreachable();
  }

  float RenderStyle::Zoom() const noexcept
  {
    return 1.f;
  }

  float RenderStyle::UsedZoom() const noexcept
  {
    return 1.f;
  }

  float RenderStyle::DeviceScaleFactor() const noexcept
  {
    return 1.f;
  }

  bool RenderStyle::EvaluationTimeZoomEnabled() const noexcept
  {
    return false;
  }

  const FontCascade &RenderStyle::GetFontCascade() const noexcept
  {
    return InitialFontCascade;
  }

  const FontCascadeDescription &RenderStyle::GetFontDescription() const noexcept
  {
    return InitialFontDescription;
  }

  const FontMetrics &RenderStyle::MetricsOfPrimaryFont() const noexcept
  {
    return InitialFontMetrics;
  }

  float RenderStyle::ComputedLineHeight() const noexcept
  {
    return static_cast<float>(InitialFontLineSpacing);
  }

  bool RenderStyle::HasPseudoElementType() const noexcept
  {
    krys_unreachable();
  }

  Maybe<float> RenderStyle::EvaluateFixedLineHeight(const style::ZoomFactor &) const noexcept
  {
    // The initial `line-height` is `normal`, which isn't fixed.
    return null;
  }

  float RenderStyle::ComputeSpecifiedLineHeight() const noexcept
  {
    return static_cast<float>(InitialFontLineSpacing);
  }

  const dom::Document &RenderView::GetDocument() const noexcept
  {
    krys_unreachable();
  }

  float RenderView::ZoomFactor() const noexcept
  {
    krys_unreachable();
  }

  FloatSize RenderView::SizeForCSSSmallViewportUnits() const noexcept
  {
    krys_unreachable();
  }

  FloatSize RenderView::SizeForCSSLargeViewportUnits() const noexcept
  {
    krys_unreachable();
  }

  FloatSize RenderView::SizeForCSSDynamicViewportUnits() const noexcept
  {
    krys_unreachable();
  }

  FloatSize RenderView::SizeForCSSDefaultViewportUnits() const noexcept
  {
    krys_unreachable();
  }

  bool RenderBox::HasEligibleContainmentForSizeQuery() const noexcept
  {
    krys_unreachable();
  }

  LayoutUnit RenderBox::ContentBoxWidth() const noexcept
  {
    krys_unreachable();
  }

  LayoutUnit RenderBox::ContentBoxHeight() const noexcept
  {
    krys_unreachable();
  }

  float AdjustFloatForAbsoluteZoom(float, const RenderStyle &) noexcept
  {
    krys_unreachable();
  }

  int32 AdjustForAbsoluteZoom(int32, const RenderView &) noexcept
  {
    krys_unreachable();
  }

  const RenderView *RenderViewOf(const dom::Document &) noexcept
  {
    return nullptr;
  }

  const RenderStyle *RenderStyleOf(const dom::Element &) noexcept
  {
    krys_unreachable();
  }

  const RenderBox *RenderBoxOf(const dom::Element &) noexcept
  {
    krys_unreachable();
  }

  boo::Color DocumentTextColor(const dom::Document &) noexcept
  {
    return boo::Color::black;
  }

  boo::Color DocumentLinkColor(const dom::Document &document, const RenderStyle &style) noexcept
  {
    return css::ColorFromKeyword(css::ValueId::WebkitLink, DocumentStyleColorOptions(document, &style));
  }

  boo::Color DocumentVisitedLinkColor(const dom::Document &document, const RenderStyle &style) noexcept
  {
    return css::ColorFromKeyword(css::ValueId::WebkitLink, DocumentStyleColorOptions(document, &style)
                                                             | style::ColorOptions::ForVisitedLink);
  }

  boo::Color DocumentActiveLinkColor(const dom::Document &document, const RenderStyle &style) noexcept
  {
    return css::ColorFromKeyword(css::ValueId::WebkitActivelink, DocumentStyleColorOptions(document, &style));
  }

  bool DocumentUseDarkAppearance(const dom::Document &, const RenderStyle *style) noexcept
  {
    return style && style->HasDarkOnlyColorScheme();
  }

  style::ColorOptions DocumentStyleColorOptions(const dom::Document &document,
                                                const RenderStyle *style) noexcept
  {
    auto options = style::ColorOptions::None;
    if (DocumentUseDarkAppearance(document, style))
    {
      options |= style::ColorOptions::UseDarkAppearance;
    }
    return options;
  }

  bool DocumentCanRequest(const dom::Document &, const url::URL &) noexcept
  {
    return true;
  }

  void DocumentKeyframesRuleDidChange(dom::Document &, const css::CSSOMString &) noexcept
  {
  }
}

namespace krys::boo::style
{
  const RenderStyle &BuilderState::Style() const noexcept
  {
    krys_unreachable();
  }

  const RenderStyle &BuilderState::ParentStyle() const noexcept
  {
    krys_unreachable();
  }

  const RenderStyle *BuilderState::RootElementStyle() const noexcept
  {
    krys_unreachable();
  }

  const dom::Document &BuilderState::GetDocument() const noexcept
  {
    krys_unreachable();
  }

  const dom::Element *BuilderState::GetElement() const noexcept
  {
    krys_unreachable();
  }

  float BuilderState::ZoomWithTextZoomFactor() noexcept
  {
    krys_unreachable();
  }

  style::ScopeOrdinal BuilderState::StyleScopeOrdinal() const noexcept
  {
    krys_unreachable();
  }

  const css::ToLengthConversionData &BuilderState::CssToLengthConversionData() const noexcept
  {
    krys_unreachable();
  }

  bool BuilderState::UseSVGZoomRulesForLength() const noexcept
  {
    krys_unreachable();
  }

  bool BuilderState::EvaluationTimeZoomEnabled() const noexcept
  {
    krys_unreachable();
  }

  void BuilderState::SetCurrentPropertyInvalidAtComputedValueTime() noexcept
  {
    krys_unreachable();
  }

  void BuilderState::SetUsesViewportUnits() noexcept
  {
    krys_unreachable();
  }

  void BuilderState::SetUsesContainerUnits() noexcept
  {
    krys_unreachable();
  }

  double BuilderState::LookupCSSRandomBaseValue(const css::calc::RandomCachingKey &,
                                                Maybe<css::keywords::ElementShared>) const noexcept
  {
    krys_unreachable();
  }

  uint32 BuilderState::SiblingCount() noexcept
  {
    krys_unreachable();
  }

  uint32 BuilderState::SiblingIndex() noexcept
  {
    krys_unreachable();
  }

  Maybe<double> AnchorPositionEvaluator::Evaluate(BuilderState &, Maybe<ScopedName>,
                                                  AnchorPositionEvaluator::Side) noexcept
  {
    krys_unreachable();
  }

  Maybe<double> AnchorPositionEvaluator::EvaluateSize(BuilderState &, Maybe<ScopedName>,
                                                      Maybe<AnchorSizeDimension>) noexcept
  {
    krys_unreachable();
  }

  RefPtr<const dom::Element>
    ContainerQueryEvaluator::SelectContainer(css::cq::Axis, const css::CSSOMString &, const dom::Element &,
                                             ContainerQueryEvaluator::SelectionMode) noexcept
  {
    krys_unreachable();
  }

  void Resolver::AddKeyframeStyle(css::KeyframesRule &) noexcept
  {
    krys_unreachable();
  }
}
