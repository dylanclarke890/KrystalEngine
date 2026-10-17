#pragma once

#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  class RenderTableCell;
  class RenderTableSection;

  /// @brief A table row.
  // FIXME: The row index and row layout come with the table builder and layout.
  class RenderTableRow final : public RenderBox
  {
    KRYS_NON_COPY_MOVABLE(RenderTableRow);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderTableRow);

  public:
    RenderTableRow(dom::Element &element, RenderStyle &&style) noexcept;
    RenderTableRow(dom::Document &document, RenderStyle &&style) noexcept;
    ~RenderTableRow() noexcept override;

    KRYS_NODISCARD RenderTableRow *NextRow() const noexcept;
    KRYS_NODISCARD RenderTableRow *PreviousRow() const noexcept;

    KRYS_NODISCARD RenderTableCell *FirstCell() const noexcept;
    KRYS_NODISCARD RenderTableCell *LastCell() const noexcept;

    KRYS_NODISCARD RenderTableSection *Section() const noexcept;

    KRYS_NODISCARD bool CanHaveChildren() const noexcept override
    {
      return true;
    }

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return (IsAnonymous() || IsPseudoElement()) ? u8"RenderTableRow (anonymous)" : u8"RenderTableRow";
    }

  private:
    void ComputeIntrinsicLogicalWidths(LayoutUnit &, LayoutUnit &) const noexcept override
    {
    }

    KRYS_NODISCARD bool RequiresLayer() const noexcept final
    {
      return HasNonVisibleOverflow() || HasTransformRelatedProperty() || HasHiddenBackface() || HasClipPath()
             || CreatesGroup() || IsStickilyPositioned() || RequiresRenderingConsolidationForViewTransition();
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderTableRow)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderTableRow();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
