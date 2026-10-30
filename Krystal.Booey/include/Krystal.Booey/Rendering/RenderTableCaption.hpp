#pragma once

#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  class RenderTable;

  /// @brief A table caption.
  class RenderTableCaption final : public RenderBlockFlow
  {
    KRYS_NON_COPY_MOVABLE(RenderTableCaption);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderTableCaption);

  public:
    RenderTableCaption(dom::Element &element, RenderStyle &&style) noexcept;
    ~RenderTableCaption() noexcept override;

  private:
    void InsertedIntoTree() noexcept override;
    void WillBeRemovedFromTree() noexcept override;

    KRYS_NODISCARD LayoutUnit ContainingBlockLogicalWidthForContent() const noexcept final;

    KRYS_NODISCARD RenderTable *Table() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderTableCaption)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderTableCaption();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
