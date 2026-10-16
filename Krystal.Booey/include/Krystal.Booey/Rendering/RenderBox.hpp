#pragma once

#include "Krystal.Booey/Core/BoxSides.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Rendering/RenderBoxModelObject.hpp"
#include "Krystal.Booey/Style/Values/Box/MarginTrim.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  enum class AvailableLogicalHeightType : bool
  {
    ExcludeMarginBorderPadding,
    IncludeMarginBorderPadding
  };

  /// @brief A renderer that generates a box.
  // FIXME: Layout, the box geometry, overflow, scrolling, floats and positioned boxes, intrinsic sizes and
  // painting come with layout and painting.
  class RenderBox : public RenderBoxModelObject
  {
    KRYS_NON_COPY_MOVABLE(RenderBox);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderBox);

  public:
    ~RenderBox() noexcept override;

    KRYS_NODISCARD bool IsGridItem() const noexcept
    {
      return Parent() && Parent()->IsRenderGrid() && !IsExcludedFromNormalLayout();
    }
    KRYS_NODISCARD bool IsFlexItem() const noexcept
    {
      return Parent() && Parent()->IsRenderFlexibleBox() && !IsExcludedFromNormalLayout();
    }

    // FIXME: Layout queries, reachable only through `RendererOf` and `RenderBoxOf`, which return null until
    // there's layout.
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

  protected:
    RenderBox(Type type, dom::Element &element, RenderStyle &&style, EnumSet<TypeFlag> baseTypeFlags = {},
              TypeSpecificFlags typeSpecificFlags = {}) noexcept;
    RenderBox(Type type, dom::Document &document, RenderStyle &&style, EnumSet<TypeFlag> baseTypeFlags = {},
              TypeSpecificFlags typeSpecificFlags = {}) noexcept;

    void UpdateFromStyle() noexcept override;

  private:
    KRYS_NODISCARD bool ComputeHasTransformRelatedProperty(const RenderStyle &styleToUse) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderBox)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderBox();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
