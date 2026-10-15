#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/CSS/Queries/ContainerQueryAxis.hpp"
#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Range.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/AnchorSizeDimension.hpp"
#include "Krystal.Booey/Style/PropertyCascade.hpp"
#include "Krystal.Booey/Style/PseudoElementIdentifier.hpp"
#include "Krystal.Booey/Style/ScopedName.hpp"
#include "Krystal.Booey/Style/ScopeOrdinal.hpp"
#include "Krystal.Booey/Style/Values/AnchorPosition/PositionTryFallbackTactic.hpp"
#include "Krystal.Booey/Style/Values/Color/ColorOptions.hpp"
#include "Krystal.Booey/Style/Values/FilterEffects/AppleColorFilter.hpp"
#include "Krystal.Booey/Style/Values/Math/MathDepth.hpp"
#include "Krystal.Booey/Style/Values/Sizing/AspectRatio.hpp"
#include "Krystal.Booey/Style/Values/Text/TextAlign.hpp"
#include "Krystal.Booey/Style/Values/Text/TextAlignLast.hpp"
#include "Krystal.Booey/Style/Values/Viewport/ZoomPrimitives.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeCheckedPtr.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include <bitset>

// FIXME: Placeholders for the rendering, font and style-resolver types that length resolution, calc
// evaluation, colour resolution and the computed value conversions need. Each declares only the members those
// callers use, under its upstream name. Only `style::BuilderState` can be constructed: it has upstream's
// state for converting values (no font updates, custom properties or style images). Everything else is
// unreachable. Each type moves to its own header, with its real implementation, when its slice is ported:
//   - `RenderView`, `RenderElement`, `RenderBox`, `RenderTheme` (focus ring metrics): rendering.
//   - `style::BuilderState`, `style::AnchorPositionEvaluator`, `style::ContainerQueryEvaluator`: the style
//     resolver (`Style/BuilderState.hpp`, `Style/AnchorPositionEvaluator.hpp`, unreviewed).
//   - the `Document*` colour functions: `dom::Document` (upstream members) once it has a page and settings.
//   - the `Document*` security and keyframes functions: `dom::Document` (upstream members) once it has a
//     security origin and runs animations.
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
  class StyleProperties;
}

namespace krys::boo::css::calc
{
  struct RandomCachingKey;
}

namespace krys::boo::style
{
  struct Color;
}

namespace krys::boo
{
  class StyleImage;

  class RenderStyle;

  class RenderTheme
  {
  public:
    KRYS_NODISCARD static float PlatformFocusRingWidth() noexcept
    {
      return 3;
    }

    KRYS_NODISCARD static float PlatformFocusRingOffset(float outlineWidth) noexcept
    {
      return std::max<float>(outlineWidth - PlatformFocusRingWidth(), 0);
    }
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

  /// @brief Collapses `document.protectedSecurityOrigin()->canRequest(url, ...)`. There's no security origin
  /// or loader yet, so every URL can be requested.
  KRYS_NODISCARD bool DocumentCanRequest(const dom::Document &, const url::URL &) noexcept;

  /// @brief Collapses `document.keyframesRuleDidChange(name)`. Nothing runs animations yet, so it does
  /// nothing.
  void DocumentKeyframesRuleDidChange(dom::Document &, const css::CSSOMString &name) noexcept;
}

namespace krys::boo::style
{
  /// @brief The position-try fallback a style is being built for: the `@position-try` rule's declarations (or
  /// the inline `position-area`) and the tactics to apply.
  struct BuilderPositionTryFallback
  {
    RefPtr<const css::StyleProperties> Properties;
    List<PositionTryFallbackTactic> Tactics;
  };

  /// @brief What a builder state is built from, besides the style it writes into.
  struct BuilderContext
  {
    RefPtr<const dom::Document> Document {};
    const RenderStyle *ParentStyle {};
    const RenderStyle *RootElementStyle {};
    RefPtr<const dom::Element> Element {};
    // FIXME: `treeResolutionState` comes with the tree resolver.
    Maybe<BuilderPositionTryFallback> PositionTryFallback {};
  };

  class BuilderState : public CanMakeCheckedPtr<BuilderState>
  {
    KRYS_NON_COPY_MOVABLE(BuilderState);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(BuilderState);

  private:
    RenderStyle &_style;
    BuilderContext _context;

    const css::ToLengthConversionData _cssToLengthConversionData;

    std::bitset<css::TotalPropertyIdValues> _invalidAtComputedValueTimeProperties;

    const PropertyCascade::Property *_currentProperty {nullptr};

    List<css::CSSOMStringAtom> _registeredContentAttributes;

  public:
    BuilderState(RenderStyle &, BuilderContext &&) noexcept;
    ~BuilderState() noexcept;

    KRYS_NODISCARD RenderStyle &Style() noexcept;
    KRYS_NODISCARD const RenderStyle &Style() const noexcept;
    KRYS_NODISCARD const RenderStyle &ParentStyle() const noexcept;
    KRYS_NODISCARD const RenderStyle *RootElementStyle() const noexcept;

    KRYS_NODISCARD const dom::Document &GetDocument() const noexcept;
    KRYS_NODISCARD const dom::Element *GetElement() const noexcept;

    KRYS_NODISCARD const Maybe<BuilderPositionTryFallback> &GetPositionTryFallback() const noexcept;

    KRYS_NODISCARD float ZoomWithTextZoomFactor() noexcept;

    KRYS_NODISCARD bool UseSVGZoomRules() const noexcept;
    KRYS_NODISCARD bool UseSVGZoomRulesForLength() const noexcept;

    KRYS_NODISCARD style::ScopeOrdinal StyleScopeOrdinal() const noexcept;

    KRYS_NODISCARD const List<css::CSSOMStringAtom> &RegisteredContentAttributes() const noexcept;
    void RegisterContentAttribute(const css::CSSOMStringAtom &attributeLocalName) noexcept;

    KRYS_NODISCARD RefPtr<StyleImage> CreateStyleImage(const css::Value &) const noexcept;

    KRYS_NODISCARD const css::ToLengthConversionData &CssToLengthConversionData() const noexcept;

    KRYS_NODISCARD bool IsAuthorOrigin() const noexcept;

    KRYS_NODISCARD css::PropertyId CssPropertyId() const noexcept;

    KRYS_NODISCARD bool IsCurrentPropertyInvalidAtComputedValueTime() const noexcept;
    void SetCurrentPropertyInvalidAtComputedValueTime() noexcept;

    /// @brief Collapses `document().settings().evaluationTimeZoomEnabled()`.
    KRYS_NODISCARD bool EvaluationTimeZoomEnabled() const noexcept;

    void SetUsesViewportUnits() noexcept;
    void SetUsesContainerUnits() noexcept;

    KRYS_NODISCARD double LookupCSSRandomBaseValue(const css::calc::RandomCachingKey &,
                                                   Maybe<css::keywords::ElementShared>) const noexcept;

    // Accessors for sibling information used by the sibling-count() and sibling-index() CSS functions.
    KRYS_NODISCARD uint32 SiblingCount() noexcept;
    KRYS_NODISCARD uint32 SiblingIndex() noexcept;

    /// @brief Collapses `Builder` setting the property it applies.
    void SetCurrentProperty(const PropertyCascade::Property *property) noexcept;
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

    /// @brief Swaps a self-alignment position (`align-self`/`justify-self`) according to a position-try
    /// fallback's tactics.
    KRYS_NODISCARD static css::ValueId
      ResolvePositionTryFallbackValueForSelfPosition(css::PropertyId, css::ValueId position, WritingMode,
                                                     const BuilderPositionTryFallback &) noexcept;
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
}
