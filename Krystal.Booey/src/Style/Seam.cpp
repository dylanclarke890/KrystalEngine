#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/CSS/Calc/RandomCachingKey.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/CSS/Values/Color/KeywordColor.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Style/Values/Color/Color.hpp"
#include "Krystal.Booey/Style/Values/Viewport/ZoomPrimitives.hpp"
#include "Krystal.Core/Assert.hpp"

// FIXME: Placeholder definitions; see `Style/Seam.hpp`. Only `RenderStyle` can be constructed, so only its
// colour members, the document functions and `Scope::ForNode` can be reached.

namespace krys::boo
{
  int FontMetrics::IntAscent() const noexcept
  {
    krys_unreachable();
  }

  int FontMetrics::IntLineSpacing() const noexcept
  {
    krys_unreachable();
  }

  Maybe<float> FontMetrics::XHeight() const noexcept
  {
    krys_unreachable();
  }

  Maybe<float> FontMetrics::CapHeight() const noexcept
  {
    krys_unreachable();
  }

  Maybe<float> FontMetrics::ZeroWidth() const noexcept
  {
    krys_unreachable();
  }

  Maybe<float> FontMetrics::IdeogramWidth() const noexcept
  {
    krys_unreachable();
  }

  float FontCascadeDescription::ComputedSize() const noexcept
  {
    krys_unreachable();
  }

  float FontCascadeDescription::ComputedSizeForRangeZoomOption(css::RangeZoomOptions) const noexcept
  {
    krys_unreachable();
  }

  float FontCascadeDescription::SpecifiedSize() const noexcept
  {
    krys_unreachable();
  }

  const FontCascadeDescription &FontCascade::GetFontDescription() const noexcept
  {
    krys_unreachable();
  }

  const FontMetrics &FontCascade::MetricsOfPrimaryFont() const noexcept
  {
    krys_unreachable();
  }

  float FontCascade::ZeroWidth() const noexcept
  {
    krys_unreachable();
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
    krys_unreachable();
  }

  const FontCascadeDescription &RenderStyle::GetFontDescription() const noexcept
  {
    krys_unreachable();
  }

  const FontMetrics &RenderStyle::MetricsOfPrimaryFont() const noexcept
  {
    krys_unreachable();
  }

  float RenderStyle::ComputedLineHeight() const noexcept
  {
    krys_unreachable();
  }

  bool RenderStyle::HasPseudoElementType() const noexcept
  {
    krys_unreachable();
  }

  Maybe<float> RenderStyle::EvaluateFixedLineHeight(const style::ZoomFactor &) const noexcept
  {
    krys_unreachable();
  }

  float RenderStyle::ComputeSpecifiedLineHeight() const noexcept
  {
    krys_unreachable();
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

  url::URL DocumentBaseURL(const dom::Document &) noexcept
  {
    return url::AboutBlankURL();
  }

  url::URL DocumentCompleteURL(const dom::Document &document, utf8_stringview url) noexcept
  {
    return url::URL(DocumentBaseURL(document), url);
  }

  css::ParserContext DocumentCssParserContext(const dom::Document &, const url::URL &baseURL) noexcept
  {
    return css::ParserContext(css::ParserMode::HTMLStandard, baseURL);
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

  Scope *Scope::ForNode(dom::Node &) noexcept
  {
    return nullptr;
  }

  void Scope::DidChangeActiveStyleSheetCandidates() noexcept
  {
    krys_unreachable();
  }

  void Scope::DidChangeStyleSheetContents() noexcept
  {
    krys_unreachable();
  }

  bool Scope::ActiveStyleSheetsContains(const css::CSSStyleSheet &) const noexcept
  {
    krys_unreachable();
  }

  Resolver *Scope::ResolverIfExists() noexcept
  {
    krys_unreachable();
  }

  void Scope::ClearLastSuccessfulPositionOptionIndexMap() noexcept
  {
    krys_unreachable();
  }
}
