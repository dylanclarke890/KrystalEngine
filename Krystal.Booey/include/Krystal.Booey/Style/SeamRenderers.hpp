#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Graphics/LayoutSize.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  // FIXME: Stand-ins for the box renderers' layout queries, until layout. They can't be constructed; they're
  // only reached through `RendererOf` and `RenderBoxOf`, which return null.

  /// @brief Collapses `AvailableLogicalHeightType` (`RenderBox.h`).
  enum class AvailableLogicalHeightType : bool
  {
    ExcludeMarginBorderPadding,
    IncludeMarginBorderPadding
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
}

#define KRYS_SEAM_RENDERER_TYPE_CAST(Type)                                                                   \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::Type)                                                    \
    KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept                      \
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
