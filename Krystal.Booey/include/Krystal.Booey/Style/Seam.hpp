#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/CSS/Queries/ContainerQueryAxis.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Range.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Style/AnchorSizeDimension.hpp"
#include "Krystal.Booey/Style/ScopedName.hpp"
#include "Krystal.Booey/Style/ScopeOrdinal.hpp"
#include "Krystal.Booey/Style/Values/Color/ColorOptions.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeCheckedPtr.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"

// FIXME: Placeholders for the rendering, font and style-resolver types that length resolution, calc
// evaluation and colour resolution need. Each declares only the members those callers use, under its upstream
// name. Only `RenderStyle` can be constructed, and only its colour and zoom members are implemented (zoom is
// always the initial value); everything else is unreachable. Each type moves to its own header, with its real
// implementation, when its slice is ported:
//   - `FontMetrics`, `FontCascadeDescription`, `FontCascade`: platform/graphics fonts.
//   - `RenderStyle`, `RenderView`, `RenderElement`, `RenderBox`: rendering.
//   - `style::BuilderState`, `style::AnchorPositionEvaluator`, `style::ContainerQueryEvaluator`: the style
//     resolver (`Style/BuilderState.hpp`, `Style/AnchorPositionEvaluator.hpp`, unreviewed).
//   - `style::Scope`, `style::Resolver`: style sheet invalidation and the style resolver. Nothing creates a
//     scope yet (`ScopeForNode` returns null), so style sheets have no scope to notify.
//   - the `Document*` colour functions: `dom::Document` (upstream members) once it has a page and settings.
//   - the `Document*` URL, parser context, security and keyframes functions: `dom::Document` (upstream
//     members) once it has a URL, settings and a security origin.
// A few members collapse an upstream call chain whose types aren't ported yet; they're marked below.

namespace krys::boo::dom
{
  class Document;
  class Element;
  class Node;
}

namespace krys::boo::css
{
  class CSSStyleSheet;
  class KeyframesRule;
  class ToLengthConversionData;
  struct ParserContext;
}

namespace krys::boo::css::calc
{
  struct RandomCachingKey;
}

namespace krys::boo::style
{
  struct Color;
  struct ZoomFactor;
}

namespace krys::boo
{
  class FontMetrics
  {
    KRYS_NON_COPY_MOVABLE(FontMetrics);

  public:
    FontMetrics() = delete;

    KRYS_NODISCARD int IntAscent() const noexcept;
    KRYS_NODISCARD int IntLineSpacing() const noexcept;
    KRYS_NODISCARD Maybe<float> XHeight() const noexcept;
    KRYS_NODISCARD Maybe<float> CapHeight() const noexcept;
    KRYS_NODISCARD Maybe<float> ZeroWidth() const noexcept;
    KRYS_NODISCARD Maybe<float> IdeogramWidth() const noexcept;
  };

  class FontCascadeDescription
  {
    KRYS_NON_COPY_MOVABLE(FontCascadeDescription);

  public:
    FontCascadeDescription() = delete;

    KRYS_NODISCARD float ComputedSize() const noexcept;
    KRYS_NODISCARD float ComputedSizeForRangeZoomOption(css::RangeZoomOptions) const noexcept;
    KRYS_NODISCARD float SpecifiedSize() const noexcept;
  };

  class FontCascade
  {
    KRYS_NON_COPY_MOVABLE(FontCascade);

  public:
    FontCascade() = delete;

    KRYS_NODISCARD const FontCascadeDescription &GetFontDescription() const noexcept;
    KRYS_NODISCARD const FontMetrics &MetricsOfPrimaryFont() const noexcept;
    KRYS_NODISCARD float ZeroWidth() const noexcept;
  };

  class RenderStyle
  {
    KRYS_NON_COPY_MOVABLE(RenderStyle);

  private:
    boo::Color _color {boo::Color::black};
    boo::Color _visitedLinkColor {boo::Color::black};
    bool _hasDarkOnlyColorScheme {false};

  public:
    RenderStyle() noexcept = default;

    KRYS_NODISCARD const boo::Color &GetColor() const noexcept;
    void SetColor(const boo::Color &color) noexcept;
    KRYS_NODISCARD const boo::Color &VisitedLinkColor() const noexcept;
    void SetVisitedLinkColor(const boo::Color &color) noexcept;

    KRYS_NODISCARD boo::Color ColorResolvingCurrentColor(const style::Color &color,
                                                         bool visitedLink = false) const noexcept;

    /// @brief Collapses `colorScheme()` resolving to `dark` without `light`.
    KRYS_NODISCARD bool HasDarkOnlyColorScheme() const noexcept;
    void SetHasDarkOnlyColorScheme(bool value) noexcept;

    KRYS_NODISCARD WritingMode GetWritingMode() const noexcept;

    /// @brief The initial `zoom` (1); nothing sets it yet.
    KRYS_NODISCARD float Zoom() const noexcept;

    /// @brief The initial used zoom (1); nothing sets it yet.
    KRYS_NODISCARD float UsedZoom() const noexcept;

    /// @brief Collapses the page's device scale factor for a document with no page: 1.
    KRYS_NODISCARD float DeviceScaleFactor() const noexcept;

    /// @brief Collapses `document().settings().evaluationTimeZoomEnabled()` for default settings: off.
    KRYS_NODISCARD bool EvaluationTimeZoomEnabled() const noexcept;

    KRYS_NODISCARD const FontCascade &GetFontCascade() const noexcept;
    KRYS_NODISCARD const FontCascadeDescription &GetFontDescription() const noexcept;
    KRYS_NODISCARD const FontMetrics &MetricsOfPrimaryFont() const noexcept;

    KRYS_NODISCARD float ComputedLineHeight() const noexcept;

    /// @brief Collapses `!pseudoElementType()`.
    KRYS_NODISCARD bool HasPseudoElementType() const noexcept;

    /// @brief Collapses `lineHeight().tryFixed()` evaluated to a `LayoutUnit` with `zoom`, as a float.
    KRYS_NODISCARD Maybe<float> EvaluateFixedLineHeight(const style::ZoomFactor &zoom) const noexcept;

    /// @brief Collapses `computeLineHeight(specifiedLineHeight())`.
    KRYS_NODISCARD float ComputeSpecifiedLineHeight() const noexcept;
  };

  class RenderView
  {
    KRYS_NON_COPY_MOVABLE(RenderView);

  public:
    RenderView() = delete;

    KRYS_NODISCARD const dom::Document &GetDocument() const noexcept;
    KRYS_NODISCARD float ZoomFactor() const noexcept;

    KRYS_NODISCARD FloatSize SizeForCSSSmallViewportUnits() const noexcept;
    KRYS_NODISCARD FloatSize SizeForCSSLargeViewportUnits() const noexcept;
    KRYS_NODISCARD FloatSize SizeForCSSDynamicViewportUnits() const noexcept;
    KRYS_NODISCARD FloatSize SizeForCSSDefaultViewportUnits() const noexcept;
  };

  class RenderElement : public CanMakeCheckedPtr<RenderElement>
  {
    KRYS_NON_COPY_MOVABLE(RenderElement);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderElement);

  public:
    RenderElement() = delete;
  };

  class RenderBox : public RenderElement
  {
    KRYS_NON_COPY_MOVABLE(RenderBox);

  public:
    RenderBox() = delete;

    KRYS_NODISCARD bool HasEligibleContainmentForSizeQuery() const noexcept;

    KRYS_NODISCARD LayoutUnit ContentBoxWidth() const noexcept;
    KRYS_NODISCARD LayoutUnit ContentBoxHeight() const noexcept;
  };

  KRYS_NODISCARD float AdjustFloatForAbsoluteZoom(float, const RenderStyle &) noexcept;

  /// @brief Collapses `adjustForAbsoluteZoom(value, renderView)` (`RenderView` is a `RenderElement`).
  KRYS_NODISCARD int32 AdjustForAbsoluteZoom(int32, const RenderView &) noexcept;

  /// @brief Collapses `document.renderView()`. Nothing creates a render tree yet, so it's always null.
  KRYS_NODISCARD const RenderView *RenderViewOf(const dom::Document &) noexcept;

  /// @brief Collapses `element.renderStyle()`.
  KRYS_NODISCARD const RenderStyle *RenderStyleOf(const dom::Element &) noexcept;

  /// @brief Collapses `dynamicDowncast<RenderBox>(element.renderer())`.
  KRYS_NODISCARD const RenderBox *RenderBoxOf(const dom::Element &) noexcept;

  /// @brief Collapses `document.textColor()`. Nothing sets it (`<body text>`), so it's the initial black.
  KRYS_NODISCARD boo::Color DocumentTextColor(const dom::Document &) noexcept;

  /// @brief Collapses `document.linkColor(style)`. Nothing sets it (`<body link>`), so it's the
  /// `-webkit-link` keyword colour.
  KRYS_NODISCARD boo::Color DocumentLinkColor(const dom::Document &, const RenderStyle &) noexcept;

  /// @brief Collapses `document.visitedLinkColor(style)`. Nothing sets it (`<body vlink>`), so it's the
  /// visited `-webkit-link` keyword colour.
  KRYS_NODISCARD boo::Color DocumentVisitedLinkColor(const dom::Document &, const RenderStyle &) noexcept;

  /// @brief Collapses `document.activeLinkColor(style)`. Nothing sets it (`<body alink>`), so it's the
  /// `-webkit-activelink` keyword colour.
  KRYS_NODISCARD boo::Color DocumentActiveLinkColor(const dom::Document &, const RenderStyle &) noexcept;

  /// @brief Collapses `document.useDarkAppearance(style)` for a document with no page and default settings:
  /// dark only when the style's colour scheme is dark only.
  KRYS_NODISCARD bool DocumentUseDarkAppearance(const dom::Document &, const RenderStyle *) noexcept;

  /// @brief Collapses `document.styleColorOptions(style)` for a document with no page and default settings.
  KRYS_NODISCARD style::ColorOptions DocumentStyleColorOptions(const dom::Document &,
                                                               const RenderStyle *) noexcept;

  /// @brief Collapses `document.baseURL()`. `Document` has no URL yet, so it's `about:blank`.
  KRYS_NODISCARD url::URL DocumentBaseURL(const dom::Document &) noexcept;

  /// @brief Collapses `document.completeURL(url)`: @p url resolved against `DocumentBaseURL`.
  KRYS_NODISCARD url::URL DocumentCompleteURL(const dom::Document &, utf8_stringview url) noexcept;

  /// @brief Collapses `CSSParserContext(document, baseURL)` for a no-quirks document with default settings.
  KRYS_NODISCARD css::ParserContext DocumentCssParserContext(const dom::Document &,
                                                             const url::URL &baseURL) noexcept;

  /// @brief Collapses `document.protectedSecurityOrigin()->canRequest(url, ...)`. There's no security origin
  /// or loader yet, so every URL can be requested.
  KRYS_NODISCARD bool DocumentCanRequest(const dom::Document &, const url::URL &) noexcept;

  /// @brief Collapses `document.keyframesRuleDidChange(name)`. Nothing runs animations yet, so it does
  /// nothing.
  void DocumentKeyframesRuleDidChange(dom::Document &, const css::CSSOMString &name) noexcept;
}

namespace krys::boo::style
{
  class BuilderState : public CanMakeCheckedPtr<BuilderState>
  {
    KRYS_NON_COPY_MOVABLE(BuilderState);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(BuilderState);

  public:
    BuilderState() = delete;

    KRYS_NODISCARD const RenderStyle &Style() const noexcept;
    KRYS_NODISCARD const RenderStyle &ParentStyle() const noexcept;
    KRYS_NODISCARD const RenderStyle *RootElementStyle() const noexcept;

    KRYS_NODISCARD const dom::Document &GetDocument() const noexcept;
    KRYS_NODISCARD const dom::Element *GetElement() const noexcept;

    KRYS_NODISCARD float ZoomWithTextZoomFactor() noexcept;

    KRYS_NODISCARD style::ScopeOrdinal StyleScopeOrdinal() const noexcept;

    KRYS_NODISCARD const css::ToLengthConversionData &CssToLengthConversionData() const noexcept;
    KRYS_NODISCARD bool UseSVGZoomRulesForLength() const noexcept;

    /// @brief Collapses `document().settings().evaluationTimeZoomEnabled()`.
    KRYS_NODISCARD bool EvaluationTimeZoomEnabled() const noexcept;

    void SetCurrentPropertyInvalidAtComputedValueTime() noexcept;

    void SetUsesViewportUnits() noexcept;
    void SetUsesContainerUnits() noexcept;

    KRYS_NODISCARD double LookupCSSRandomBaseValue(const css::calc::RandomCachingKey &,
                                                   Maybe<css::keywords::ElementShared>) const noexcept;

    // Accessors for sibling information used by the sibling-count() and sibling-index() CSS functions.
    KRYS_NODISCARD uint32 SiblingCount() noexcept;
    KRYS_NODISCARD uint32 SiblingIndex() noexcept;
  };

  class AnchorPositionEvaluator
  {
  public:
    using Side = Variant<css::ValueId, double>;

    AnchorPositionEvaluator() = delete;

    KRYS_NODISCARD static Maybe<double> Evaluate(BuilderState &, Maybe<ScopedName> elementName,
                                                 Side) noexcept;
    KRYS_NODISCARD static Maybe<double> EvaluateSize(BuilderState &, Maybe<ScopedName> elementName,
                                                     Maybe<AnchorSizeDimension>) noexcept;
  };

  class ContainerQueryEvaluator
  {
  public:
    enum class SelectionMode : uint8
    {
      Element,
      PseudoElement,
      PartPseudoElement
    };

    ContainerQueryEvaluator() = delete;

    KRYS_NODISCARD static RefPtr<const dom::Element>
      SelectContainer(css::cq::Axis, const css::CSSOMString &name, const dom::Element &,
                      SelectionMode = SelectionMode::Element) noexcept;
  };

  class Resolver
  {
    KRYS_NON_COPY_MOVABLE(Resolver);

  public:
    Resolver() = delete;

    void AddKeyframeStyle(css::KeyframesRule &) noexcept;
  };

  class Scope : public CanMakeCheckedPtr<Scope>, public CanMakeWeakPtr<Scope>
  {
    KRYS_NON_COPY_MOVABLE(Scope);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(Scope);

  public:
    Scope() = delete;

    /// @brief Collapses `Style::Scope::forNode(node)`. No document or shadow root has a scope yet, so it's
    /// always null.
    KRYS_NODISCARD static Scope *ForNode(dom::Node &) noexcept;

    void DidChangeActiveStyleSheetCandidates() noexcept;
    void DidChangeStyleSheetContents() noexcept;
    KRYS_NODISCARD bool ActiveStyleSheetsContains(const css::CSSStyleSheet &) const noexcept;
    KRYS_NODISCARD Resolver *ResolverIfExists() noexcept;

    /// @brief Collapses `setLastSuccessfulPositionOptionIndexMap({ })`.
    void ClearLastSuccessfulPositionOptionIndexMap() noexcept;
  };
}
