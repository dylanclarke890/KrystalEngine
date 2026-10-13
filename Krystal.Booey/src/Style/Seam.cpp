#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/CSS/Calc/RandomCachingKey.hpp"
#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Style/Values/Viewport/ZoomPrimitives.hpp"
#include "Krystal.Core/Assert.hpp"

// FIXME: Placeholder definitions; see `Style/Seam.hpp`. None of these types can be constructed, so none of
// these can be reached.

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

  WritingMode RenderStyle::GetWritingMode() const noexcept
  {
    krys_unreachable();
  }

  float RenderStyle::Zoom() const noexcept
  {
    krys_unreachable();
  }

  float RenderStyle::UsedZoom() const noexcept
  {
    krys_unreachable();
  }

  float RenderStyle::DeviceScaleFactor() const noexcept
  {
    krys_unreachable();
  }

  bool RenderStyle::EvaluationTimeZoomEnabled() const noexcept
  {
    krys_unreachable();
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

  const RenderView *RenderViewOf(const dom::Document &) noexcept
  {
    krys_unreachable();
  }

  const RenderStyle *RenderStyleOf(const dom::Element &) noexcept
  {
    krys_unreachable();
  }

  const RenderBox *RenderBoxOf(const dom::Element &) noexcept
  {
    krys_unreachable();
  }
}

namespace krys::boo::style
{
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
}
