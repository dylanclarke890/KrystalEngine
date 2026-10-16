#pragma once

#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  /// @brief A flex container.
  // FIXME: Flex layout comes with layout.
  class RenderFlexibleBox : public RenderBlock
  {
    KRYS_NON_COPY_MOVABLE(RenderFlexibleBox);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderFlexibleBox);

  public:
    RenderFlexibleBox(Type type, dom::Element &element, RenderStyle &&style) noexcept;
    RenderFlexibleBox(Type type, dom::Document &document, RenderStyle &&style) noexcept;
    ~RenderFlexibleBox() noexcept override;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderFlexibleBox";
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderFlexibleBox)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderFlexibleBox();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
