#pragma once

#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  /// @brief A `display: -webkit-box` container.
  // FIXME: The legacy box layout comes with layout.
  class RenderDeprecatedFlexibleBox final : public RenderBlock
  {
    KRYS_NON_COPY_MOVABLE(RenderDeprecatedFlexibleBox);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderDeprecatedFlexibleBox);

  public:
    RenderDeprecatedFlexibleBox(dom::Element &element, RenderStyle &&style) noexcept;
    ~RenderDeprecatedFlexibleBox() noexcept override;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderDeprecatedFlexibleBox)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderDeprecatedFlexibleBox();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
