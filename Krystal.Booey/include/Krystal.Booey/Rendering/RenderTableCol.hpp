#pragma once

#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  /// @brief A table column or column group.
  // FIXME: The span (updateFromElement), the table of the column and column layout come with the table
  // builder and layout.
  class RenderTableCol final : public RenderBox
  {
    KRYS_NON_COPY_MOVABLE(RenderTableCol);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderTableCol);

  public:
    RenderTableCol(dom::Element &element, RenderStyle &&style) noexcept;
    RenderTableCol(dom::Document &document, RenderStyle &&style) noexcept;
    ~RenderTableCol() noexcept override;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderTableCol";
    }

    KRYS_NODISCARD bool IsTableColumnGroup() const noexcept;
    KRYS_NODISCARD bool IsTableColumn() const noexcept;

    KRYS_NODISCARD bool IsChildAllowed(const RenderObject &child,
                                       const RenderStyle &style) const noexcept override;
    KRYS_NODISCARD bool CanHaveChildren() const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderTableCol)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderTableCol();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
