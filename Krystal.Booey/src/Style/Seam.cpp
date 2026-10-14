#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/CSS/Calc/RandomCachingKey.hpp"
#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/CSS/Values/Color/KeywordColor.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Style/Values/Color/Color.hpp"
#include "Krystal.Booey/Style/Values/Viewport/ZoomPrimitives.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGSvgElement.hpp"
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
    return _writingMode;
  }

  bool RenderStyle::SetDirection(TextDirection bidiDirection) noexcept
  {
    if (_writingMode.ComputedTextDirection() == bidiDirection)
    {
      return false;
    }
    _writingMode.SetTextDirection(bidiDirection);
    return true;
  }

  bool RenderStyle::SetTextOrientation(TextOrientation textOrientation) noexcept
  {
    if (_writingMode.ComputedTextOrientation() == textOrientation)
    {
      return false;
    }
    _writingMode.SetTextOrientation(textOrientation);
    return true;
  }

  bool RenderStyle::SetWritingMode(StyleWritingMode mode) noexcept
  {
    if (_writingMode.ComputedWritingMode() == mode)
    {
      return false;
    }
    _writingMode.SetWritingMode(mode);
    return true;
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

  Maybe<PseudoElementType> RenderStyle::GetPseudoElementType() const noexcept
  {
    if (!_pseudoElementIdentifier)
    {
      return null;
    }
    return _pseudoElementIdentifier->Type;
  }

  Maybe<style::PseudoElementIdentifier> RenderStyle::GetPseudoElementIdentifier() const noexcept
  {
    return _pseudoElementIdentifier;
  }

  void RenderStyle::SetPseudoElementIdentifier(Maybe<style::PseudoElementIdentifier> &&identifier) noexcept
  {
    _pseudoElementIdentifier = krys::move(identifier);
  }

  bool RenderStyle::UsesViewportUnits() const noexcept
  {
    return _usesViewportUnits;
  }

  void RenderStyle::SetUsesViewportUnits() noexcept
  {
    _usesViewportUnits = true;
  }

  bool RenderStyle::UsesContainerUnits() const noexcept
  {
    return _usesContainerUnits;
  }

  void RenderStyle::SetUsesContainerUnits() noexcept
  {
    _usesContainerUnits = true;
  }

  bool RenderStyle::UsesTreeCountingFunctions() const noexcept
  {
    return _usesTreeCountingFunctions;
  }

  void RenderStyle::SetUsesTreeCountingFunctions() noexcept
  {
    _usesTreeCountingFunctions = true;
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

  float AdjustFloatForAbsoluteZoom(float value, const RenderStyle &style) noexcept
  {
    return value / style.UsedZoom();
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
  BuilderState::BuilderState(RenderStyle &style, BuilderContext &&context) noexcept
      : _style(style), _context(krys::move(context)), _cssToLengthConversionData(style, *this)
  {
  }

  BuilderState::~BuilderState() noexcept = default;

  RenderStyle &BuilderState::Style() noexcept
  {
    return _style;
  }

  const RenderStyle &BuilderState::Style() const noexcept
  {
    return _style;
  }

  const RenderStyle &BuilderState::ParentStyle() const noexcept
  {
    return *_context.ParentStyle;
  }

  const RenderStyle *BuilderState::RootElementStyle() const noexcept
  {
    return _context.RootElementStyle;
  }

  const dom::Document &BuilderState::GetDocument() const noexcept
  {
    return *_context.Document;
  }

  const dom::Element *BuilderState::GetElement() const noexcept
  {
    return _context.Element.get();
  }

  float BuilderState::ZoomWithTextZoomFactor() noexcept
  {
    // FIXME: Documents have no frame, so there's no text zoom factor (`-webkit-text-zoom` isn't supported).
    return CssToLengthConversionData().Zoom();
  }

  // SVG handles zooming in a different way compared to CSS. The whole document is scaled instead
  // of each individual length value in the render style / tree. CSSPrimitiveValue::resolveAsLength*()
  // multiplies each resolved length with the zoom multiplier - so for SVG we need to disable that.
  // Though all CSS values that can be applied to outermost <svg> elements (width/height/border/padding...)
  // need to respect the scaling. RenderBox (the parent class of LegacyRenderSVGRoot) grabs values like
  // width/height/border/padding/... from the RenderStyle -> for SVG these values would never scale,
  // if we'd pass a 1.0 zoom factor everywhere. So we only pass a zoom factor of 1.0 for specific
  // properties that are NOT allowed to scale within a zoomed SVG document (letter/word-spacing/font-size).
  bool BuilderState::UseSVGZoomRules() const noexcept
  {
    return Is<svg::SVGElement>(GetElement());
  }

  bool BuilderState::UseSVGZoomRulesForLength() const noexcept
  {
    auto *svgElement = DynamicDowncast<svg::SVGElement>(GetElement());
    return svgElement && !(Is<svg::SVGSvgElement>(*svgElement) && svgElement->ParentNode());
  }

  style::ScopeOrdinal BuilderState::StyleScopeOrdinal() const noexcept
  {
    return _currentProperty->StyleScopeOrdinal;
  }

  const List<css::CSSOMStringAtom> &BuilderState::RegisteredContentAttributes() const noexcept
  {
    return _registeredContentAttributes;
  }

  void BuilderState::RegisterContentAttribute(const css::CSSOMStringAtom &attributeLocalName) noexcept
  {
    if (Style().GetPseudoElementType() == PseudoElementType::Before
        || Style().GetPseudoElementType() == PseudoElementType::After)
    {
      _registeredContentAttributes.push_back(attributeLocalName);
    }
  }

  const css::ToLengthConversionData &BuilderState::CssToLengthConversionData() const noexcept
  {
    return _cssToLengthConversionData;
  }

  bool BuilderState::IsAuthorOrigin() const noexcept
  {
    return _currentProperty && _currentProperty->Origin == PropertyCascade::Origin::Author;
  }

  css::PropertyId BuilderState::CssPropertyId() const noexcept
  {
    return _currentProperty ? _currentProperty->Id : css::PropertyId::Invalid;
  }

  bool BuilderState::IsCurrentPropertyInvalidAtComputedValueTime() const noexcept
  {
    return _invalidAtComputedValueTimeProperties.test(static_cast<size_t>(CssPropertyId()));
  }

  void BuilderState::SetCurrentPropertyInvalidAtComputedValueTime() noexcept
  {
    _invalidAtComputedValueTimeProperties.set(static_cast<size_t>(CssPropertyId()));
  }

  bool BuilderState::EvaluationTimeZoomEnabled() const noexcept
  {
    return false;
  }

  void BuilderState::SetUsesViewportUnits() noexcept
  {
    _style.SetUsesViewportUnits();
  }

  void BuilderState::SetUsesContainerUnits() noexcept
  {
    _style.SetUsesContainerUnits();
  }

  double BuilderState::LookupCSSRandomBaseValue(const css::calc::RandomCachingKey &,
                                                Maybe<css::keywords::ElementShared>) const noexcept
  {
    // FIXME: The element and document `random()` caches come with the resolver.
    krys_unreachable();
  }

  // MARK: - Tree Counting Functions

  uint32 BuilderState::SiblingCount() noexcept
  {
    // https://drafts.csswg.org/css-values-5/#funcdef-sibling-count

    krys_debug_assert(GetElement());

    RefPtr parent = ShareRefPtr(GetElement()->ParentElement());
    if (!parent)
    {
      return 1;
    }

    _style.SetUsesTreeCountingFunctions();
    // FIXME: Mark the parent's children as affected by forward and backward positional rules once elements
    // keep their style relations (invalidation).

    uint32 count = 1;
    for (auto sibling = GetElement()->PreviousElementSibling(); sibling;
         sibling = sibling->PreviousElementSibling())
    {
      ++count;
    }
    for (auto sibling = GetElement()->NextElementSibling(); sibling; sibling = sibling->NextElementSibling())
    {
      ++count;
    }
    return count;
  }

  uint32 BuilderState::SiblingIndex() noexcept
  {
    // https://drafts.csswg.org/css-values-5/#funcdef-sibling-index

    krys_debug_assert(GetElement());

    RefPtr parent = ShareRefPtr(GetElement()->ParentElement());
    if (!parent)
    {
      return 1;
    }

    _style.SetUsesTreeCountingFunctions();
    // FIXME: Mark the parent's children as affected by forward and backward positional rules once elements
    // keep their style relations (invalidation).

    uint32 count = 1;
    for (auto sibling = GetElement()->PreviousElementSibling(); sibling;
         sibling = sibling->PreviousElementSibling())
    {
      ++count;
    }
    return count;
  }

  void BuilderState::SetCurrentProperty(const PropertyCascade::Property *property) noexcept
  {
    _currentProperty = property;
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
