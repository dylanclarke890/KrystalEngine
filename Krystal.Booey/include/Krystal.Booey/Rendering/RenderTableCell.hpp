#pragma once

#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  /// @brief A table cell.
  // FIXME: Spans, the column index, collapsed borders and cell layout come with the table builder and layout.
  class RenderTableCell final : public RenderBlockFlow
  {
    KRYS_NON_COPY_MOVABLE(RenderTableCell);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderTableCell);

  public:
    RenderTableCell(dom::Element &element, RenderStyle &&style) noexcept;
    RenderTableCell(dom::Document &document, RenderStyle &&style) noexcept;
    ~RenderTableCell() noexcept override;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return (IsAnonymous() || IsPseudoElement()) ? u8"RenderTableCell (anonymous)" : u8"RenderTableCell";
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderTableCell)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderTableCell();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
