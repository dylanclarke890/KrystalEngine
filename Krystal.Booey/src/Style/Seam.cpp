#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/CSS/Calc/RandomCachingKey.hpp"
#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
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
  }

  FontCascade RenderStyle::InitialFontCascade() noexcept
  {
    FontCascadeDescription description;
    description.SetSpecifiedSize(InitialFontSize);
    description.SetComputedSize(InitialFontSize);
    description.SetKeywordSizeFromIdentifier(css::ValueId::Medium);
    return FontCascade {krys::move(description)};
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

  style::ZoomFactor RenderStyle::UsedZoomForLength() const noexcept
  {
    if (EvaluationTimeZoomEnabled())
    {
      return style::ZoomFactor {UsedZoom(), DeviceScaleFactor()};
    }
    return style::ZoomFactor {1.0f, DeviceScaleFactor()};
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
    return _fontCascade;
  }

  FontCascade &RenderStyle::MutableFontCascade() noexcept
  {
    return _fontCascade;
  }

  const FontCascadeDescription &RenderStyle::GetFontDescription() const noexcept
  {
    return _fontCascade.GetFontDescription();
  }

  bool RenderStyle::SetFontDescription(FontCascadeDescription &&description) noexcept
  {
    if (_fontCascade.GetFontDescription() == description)
    {
      return false;
    }
    _fontCascade = FontCascade {krys::move(description), _fontCascade};
    return true;
  }

  float RenderStyle::ComputedFontSize() const noexcept
  {
    return GetFontDescription().ComputedSize();
  }

  const FontMetrics &RenderStyle::MetricsOfPrimaryFont() const noexcept
  {
    return _fontCascade.MetricsOfPrimaryFont();
  }

  float RenderStyle::ComputedLineHeight() const noexcept
  {
    return static_cast<float>(MetricsOfPrimaryFont().IntLineSpacing());
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

  // Properties (see the declarations).

  Maybe<float> RenderStyle::EvaluateFixedLineHeight(const style::ZoomFactor &) const noexcept
  {
    // The initial `line-height` is `normal`, which isn't fixed.
    return null;
  }

  float RenderStyle::ComputeSpecifiedLineHeight() const noexcept
  {
    return static_cast<float>(MetricsOfPrimaryFont().IntLineSpacing());
  }

  const style::AspectRatio &RenderStyle::AspectRatio() const noexcept
  {
    return _aspectRatio;
  }

  void RenderStyle::SetAspectRatio(style::AspectRatio value) noexcept
  {
    _aspectRatio = krys::move(value);
  }

  double RenderStyle::LogicalAspectRatio() const noexcept
  {
    auto ratio = AspectRatio().TryRatio();
    krys_debug_assert(ratio);

    if (GetWritingMode().IsHorizontal())
    {
      return ratio->Numerator.Value / ratio->Denominator.Value;
    }
    return ratio->Denominator.Value / ratio->Numerator.Value;
  }

  const style::MathDepth &RenderStyle::MathDepth() const noexcept
  {
    return _mathDepth;
  }

  void RenderStyle::SetMathDepth(style::MathDepth value) noexcept
  {
    _mathDepth = krys::move(value);
  }

  MathStyle RenderStyle::GetMathStyle() const noexcept
  {
    return _mathStyle;
  }

  void RenderStyle::SetMathStyle(MathStyle value) noexcept
  {
    _mathStyle = krys::move(value);
  }

  style::TextAlign RenderStyle::TextAlign() const noexcept
  {
    return _textAlign;
  }

  void RenderStyle::SetTextAlign(style::TextAlign value) noexcept
  {
    _textAlign = krys::move(value);
  }

  style::TextAlignLast RenderStyle::TextAlignLast() const noexcept
  {
    return _textAlignLast;
  }

  void RenderStyle::SetTextAlignLast(style::TextAlignLast value) noexcept
  {
    _textAlignLast = krys::move(value);
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

  const Maybe<BuilderPositionTryFallback> &BuilderState::GetPositionTryFallback() const noexcept
  {
    return _context.PositionTryFallback;
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

  css::ValueId AnchorPositionEvaluator::ResolvePositionTryFallbackValueForSelfPosition(
    css::PropertyId propertyID, css::ValueId position, WritingMode writingMode,
    const BuilderPositionTryFallback &fallback) noexcept
  {
    // Implements the bullet starting "For the self-alignment properties" from step 4 of
    // https://drafts.csswg.org/css-anchor-position-1/#swap-due-to-a-try-tactic.

    krys_debug_assert(propertyID == css::PropertyId::AlignSelf || propertyID == css::PropertyId::JustifySelf);

    auto flipSidedPosition = [](css::ValueId position) -> css::ValueId
    {
      // Swap to the "opposite" position if the current position is "sided".
      // If there is no opposite value, nothing is changed.
      switch (position)
      {
        case css::ValueId::Start:     return css::ValueId::End;
        case css::ValueId::End:       return css::ValueId::Start;
        case css::ValueId::SelfStart: return css::ValueId::SelfEnd;
        case css::ValueId::SelfEnd:   return css::ValueId::SelfStart;
        case css::ValueId::FlexStart: return css::ValueId::FlexEnd;
        case css::ValueId::FlexEnd:   return css::ValueId::FlexStart;
        case css::ValueId::Left:      return css::ValueId::Right;
        case css::ValueId::Right:     return css::ValueId::Left;
        default:                      return position;
      }
    };

    auto flipStart = [](WritingMode writingMode, css::ValueId position) -> css::ValueId
    {
      // `justify-self` additionally takes `left`/`right`, `align-self` doesn't. When applying `flip-start`,
      // `justify-self` gets swapped with `align-self`. So if we're resolving `justify-self` (which later gets
      // swapped with `align-self`), and the position is `left`/`right`, resolve it to
      // `self-start`/`self-end`.
      switch (position)
      {
        case css::ValueId::Left:
          return writingMode.IsBidiLTR() ? css::ValueId::SelfStart : css::ValueId::SelfEnd;
        case css::ValueId::Right:
          return writingMode.IsBidiLTR() ? css::ValueId::SelfEnd : css::ValueId::SelfStart;
        default: return position;
      }
    };

    for (auto tactic : fallback.Tactics)
    {
      switch (tactic)
      {
        case PositionTryFallbackTactic::FlipBlock:
          if (propertyID == css::PropertyId::AlignSelf)
          {
            position = flipSidedPosition(position);
          }
          break;
        case PositionTryFallbackTactic::FlipInline:
          if (propertyID == css::PropertyId::JustifySelf)
          {
            position = flipSidedPosition(position);
          }
          break;
        case PositionTryFallbackTactic::FlipX:
          if (propertyID
              == (writingMode.IsHorizontal() ? css::PropertyId::JustifySelf : css::PropertyId::AlignSelf))
          {
            position = flipSidedPosition(position);
          }
          break;
        case PositionTryFallbackTactic::FlipY:
          if (propertyID
              == (writingMode.IsHorizontal() ? css::PropertyId::AlignSelf : css::PropertyId::JustifySelf))
          {
            position = flipSidedPosition(position);
          }
          break;
        case PositionTryFallbackTactic::FlipStart:
          if (propertyID == css::PropertyId::JustifySelf)
          {
            position = flipStart(writingMode, position);
          }
          break;
      }
    }

    return position;
  }
}
