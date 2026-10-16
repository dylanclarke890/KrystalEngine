#pragma once

#include "Krystal.Booey/Core/BoxSides.hpp"
#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Range.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Graphics/LayoutSize.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/AnchorSizeDimension.hpp"
#include "Krystal.Booey/Style/PropertyCascade.hpp"
#include "Krystal.Booey/Style/PseudoElementIdentifier.hpp"
#include "Krystal.Booey/Style/ScopedName.hpp"
#include "Krystal.Booey/Style/ScopeOrdinal.hpp"
#include "Krystal.Booey/Style/Values/AnchorPosition/PositionTryFallbackTactic.hpp"
#include "Krystal.Booey/Style/Values/Box/MarginTrim.hpp"
#include "Krystal.Booey/Style/Values/Color/ColorOptions.hpp"
#include "Krystal.Booey/Style/Values/FilterEffects/AppleColorFilter.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridTrackSizingDirection.hpp"
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
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include <bitset>

// FIXME: Placeholders for the rendering, font and style-resolver types that length resolution, calc
// evaluation, colour resolution, the computed value conversions and the extractor need. Each declares only
// the members those callers use, under its upstream name. None can be constructed but `RenderView`, which a
// document shown in a frame with a view owns. Each type moves to its own header, with its real
// implementation, when its slice is ported:
//   - `RenderView`, `RenderElement` (with the `RenderObject` members), `RenderBoxModelObject`, `RenderBox`,
//     `RenderBlock`, `RenderGrid`, `RenderInline`, `RenderTheme` (focus ring metrics): rendering.
//   - `style::AnchorPositionEvaluator`: anchor positioning (`Style/AnchorPositionEvaluator.hpp`, unreviewed).
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

    // NOTE: This is just a legacy hard-coded list of elements that have some appearance value in html.css
    // FIXME: Remove when devolvable widgets are universally enabled.
    // FIXME: Also the `::-webkit-list-button` part of user agent shadow trees, once there are any.
    KRYS_NODISCARD static bool HasAppearanceForElementTypeFromUAStyle(const dom::Element &) noexcept;
  };

  /// @brief Stands in for the root of a document's render tree: its document, whether it needs layout, the
  /// page zoom and the sizes viewport units resolve against, from the document's view.
  class RenderView
  {
    KRYS_NON_COPY_MOVABLE(RenderView);

  private:
    dom::Document &_document;
    // A new renderer needs layout. Style changes don't mark it: there are no other renderers.
    bool _needsLayout {true};

  public:
    explicit RenderView(dom::Document &document) noexcept : _document(document)
    {
    }

    KRYS_NODISCARD const dom::Document &GetDocument() const noexcept
    {
      return _document;
    }

    KRYS_NODISCARD bool NeedsLayout() const noexcept
    {
      return _needsLayout;
    }

    void SetNeedsLayout() noexcept
    {
      _needsLayout = true;
    }

    void ClearNeedsLayout() noexcept
    {
      _needsLayout = false;
    }

    KRYS_NODISCARD float ZoomFactor() const noexcept;

    KRYS_NODISCARD FloatSize SizeForCSSSmallViewportUnits() const noexcept;
    KRYS_NODISCARD FloatSize SizeForCSSLargeViewportUnits() const noexcept;
    KRYS_NODISCARD FloatSize SizeForCSSDynamicViewportUnits() const noexcept;
    KRYS_NODISCARD FloatSize SizeForCSSDefaultViewportUnits() const noexcept;
  };

  class RenderBlock;
  class RenderBox;

  /// @brief Collapses `AvailableLogicalHeightType` (`RenderBox.h`).
  enum class AvailableLogicalHeightType : bool
  {
    ExcludeMarginBorderPadding,
    IncludeMarginBorderPadding
  };

  // The `RenderObject` members are declared on `RenderElement`.
  class RenderElement : public CanMakeCheckedPtr<RenderElement>
  {
    KRYS_NON_COPY_MOVABLE(RenderElement);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderElement);

  public:
    RenderElement() = delete;

    KRYS_NODISCARD const RenderStyle &Style() const noexcept;
    KRYS_NODISCARD const RenderElement *Parent() const noexcept;
    KRYS_NODISCARD const RenderElement *Container() const noexcept;
    KRYS_NODISCARD const RenderBlock *ContainingBlock() const noexcept;

    KRYS_NODISCARD bool IsRenderBoxModelObject() const noexcept;
    KRYS_NODISCARD bool IsRenderBox() const noexcept;
    KRYS_NODISCARD bool IsRenderBlock() const noexcept;
    KRYS_NODISCARD bool IsRenderGrid() const noexcept;
    KRYS_NODISCARD bool IsRenderInline() const noexcept;
    KRYS_NODISCARD bool IsRenderOrLegacyRenderSVGModelObject() const noexcept;

    KRYS_NODISCARD bool IsInline() const noexcept;
    KRYS_NODISCARD bool IsBlockLevelReplacedOrAtomicInline() const noexcept;
    KRYS_NODISCARD bool IsBlockLevelBox() const noexcept;
    KRYS_NODISCARD bool IsFlexItem() const noexcept;
    KRYS_NODISCARD bool IsGridItem() const noexcept;
    KRYS_NODISCARD bool IsHorizontalWritingMode() const noexcept;

    KRYS_NODISCARD FloatRect TransformReferenceBoxRect(const RenderStyle &) const noexcept;
  };

  class RenderBoxModelObject : public RenderElement
  {
    KRYS_NON_COPY_MOVABLE(RenderBoxModelObject);

  public:
    RenderBoxModelObject() = delete;

    KRYS_NODISCARD LayoutUnit OffsetLeft() const noexcept;
    KRYS_NODISCARD LayoutUnit OffsetTop() const noexcept;
    KRYS_NODISCARD LayoutUnit OffsetWidth() const noexcept;
    KRYS_NODISCARD LayoutUnit OffsetHeight() const noexcept;

    KRYS_NODISCARD LayoutUnit ComputedCSSPaddingTop() const noexcept;
    KRYS_NODISCARD LayoutUnit ComputedCSSPaddingRight() const noexcept;
    KRYS_NODISCARD LayoutUnit ComputedCSSPaddingBottom() const noexcept;
    KRYS_NODISCARD LayoutUnit ComputedCSSPaddingLeft() const noexcept;

    KRYS_NODISCARD bool IsPositioned() const noexcept;
    KRYS_NODISCARD bool IsOutOfFlowPositioned() const noexcept;
    KRYS_NODISCARD bool IsRelativelyPositioned() const noexcept;
    KRYS_NODISCARD bool IsStickilyPositioned() const noexcept;

    KRYS_NODISCARD LayoutSize RelativePositionOffset() const noexcept;
    KRYS_NODISCARD LayoutUnit ContainingBlockLogicalWidthForContent() const noexcept;
  };

  class RenderBox : public RenderBoxModelObject
  {
    KRYS_NON_COPY_MOVABLE(RenderBox);

  public:
    RenderBox() = delete;

    KRYS_NODISCARD bool HasEligibleContainmentForSizeQuery() const noexcept;

    KRYS_NODISCARD LayoutUnit ContentBoxWidth() const noexcept;
    KRYS_NODISCARD LayoutUnit ContentBoxHeight() const noexcept;
    KRYS_NODISCARD LayoutUnit ContentBoxLogicalWidth() const noexcept;
    KRYS_NODISCARD LayoutUnit ContentBoxLogicalHeight() const noexcept;
    KRYS_NODISCARD LayoutRect BorderBoxRect() const noexcept;
    KRYS_NODISCARD LayoutRect ComputedCSSContentBoxRect() const noexcept;

    KRYS_NODISCARD LayoutUnit MarginTop() const noexcept;
    KRYS_NODISCARD LayoutUnit MarginRight() const noexcept;
    KRYS_NODISCARD LayoutUnit MarginBottom() const noexcept;
    KRYS_NODISCARD LayoutUnit MarginLeft() const noexcept;
    KRYS_NODISCARD bool HasTrimmedMargin(Maybe<style::MarginTrimSide>) const noexcept;

    /// @brief Collapses `enclosingClippingBoxForStickyPosition().first`.
    KRYS_NODISCARD const RenderBox &EnclosingClippingBoxForStickyPosition() const noexcept;
    /// @brief Collapses `containingBlockRangeForPositioned(containingBlock, axis).size()`.
    KRYS_NODISCARD LayoutUnit ContainingBlockRangeSizeForPositioned(const RenderBoxModelObject &,
                                                                    BoxAxis) const noexcept;
    KRYS_NODISCARD
    LayoutUnit ContainingBlockLogicalHeightForContent(AvailableLogicalHeightType) const noexcept;
  };

  class RenderBlock : public RenderBox
  {
    KRYS_NON_COPY_MOVABLE(RenderBlock);

  public:
    RenderBlock() = delete;

    KRYS_NODISCARD LayoutUnit PaddingBoxWidth() const noexcept;
    KRYS_NODISCARD LayoutUnit PaddingBoxHeight() const noexcept;
  };

  class RenderGrid : public RenderBlock
  {
    KRYS_NON_COPY_MOVABLE(RenderGrid);

  public:
    RenderGrid() = delete;

    KRYS_NODISCARD const List<LayoutUnit> &Positions(style::GridTrackSizingDirection) const noexcept;
    KRYS_NODISCARD bool IsSubgrid(style::GridTrackSizingDirection) const noexcept;
    KRYS_NODISCARD uint32 NumTracks(style::GridTrackSizingDirection) const noexcept;
    KRYS_NODISCARD uint32 AutoRepeatCountForDirection(style::GridTrackSizingDirection) const noexcept;
    KRYS_NODISCARD List<LayoutUnit>
      TrackSizesForComputedStyle(style::GridTrackSizingDirection) const noexcept;
    KRYS_NODISCARD int32 ExplicitGridStartForDirection(style::GridTrackSizingDirection) const noexcept;
  };

  class RenderInline : public RenderBoxModelObject
  {
    KRYS_NON_COPY_MOVABLE(RenderInline);

  public:
    RenderInline() = delete;

    KRYS_NODISCARD LayoutUnit InnerPaddingBoxWidth() const noexcept;
    KRYS_NODISCARD LayoutUnit InnerPaddingBoxHeight() const noexcept;
  };

  /// @brief Collapses `adjustForAbsoluteZoom(value, renderView)` (`RenderView` is a `RenderElement`): the
  /// render view's style is the document's `RenderViewStyle()`.
  KRYS_NODISCARD int32 AdjustForAbsoluteZoom(int32, const RenderView &) noexcept;

  /// @brief Collapses `document.renderView()`: the document's render view, if it's shown in a frame with a
  /// view.
  KRYS_NODISCARD const RenderView *RenderViewOf(const dom::Document &) noexcept;

  /// @brief Collapses `dynamicDowncast<RenderBox>(element.renderer())`. Nothing creates a render tree yet, so
  /// it's always null.
  KRYS_NODISCARD const RenderBox *RenderBoxOf(const dom::Element &) noexcept;

  /// @brief Collapses `element.renderer()`. Nothing creates a render tree yet, so it's always null.
  KRYS_NODISCARD const RenderElement *RendererOf(const dom::Element &) noexcept;

  /// @brief Collapses `Styleable(element, pseudoElementIdentifier).renderer()`. Always null, as above.
  KRYS_NODISCARD const RenderElement *RendererOf(const dom::Element &,
                                                 const Maybe<style::PseudoElementIdentifier> &) noexcept;

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
  class BuilderState;
  struct BuilderPositionTryFallback;

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
}

#define KRYS_SEAM_RENDERER_TYPE_CAST(Type)                                                                   \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::Type)                                                    \
    KRYS_NODISCARD static bool IsType(const krys::boo::RenderElement &renderer) noexcept                     \
    {                                                                                                        \
      return renderer.Is##Type();                                                                            \
    }                                                                                                        \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SEAM_RENDERER_TYPE_CAST(RenderBoxModelObject);
KRYS_SEAM_RENDERER_TYPE_CAST(RenderBox);
KRYS_SEAM_RENDERER_TYPE_CAST(RenderBlock);
KRYS_SEAM_RENDERER_TYPE_CAST(RenderGrid);
KRYS_SEAM_RENDERER_TYPE_CAST(RenderInline);

#undef KRYS_SEAM_RENDERER_TYPE_CAST
