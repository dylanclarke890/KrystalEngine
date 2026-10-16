#pragma once

#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  /// @brief A list item.
  // FIXME: The marker, the value and the ordinal come with generated content; layout and painting with layout
  // and painting.
  class RenderListItem final : public RenderBlockFlow
  {
    KRYS_NON_COPY_MOVABLE(RenderListItem);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderListItem);

  public:
    RenderListItem(dom::Element &element, RenderStyle &&style) noexcept;
    ~RenderListItem() noexcept override;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept final
    {
      return u8"RenderListItem";
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderListItem)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderListItem();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
