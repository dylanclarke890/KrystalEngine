#pragma once

#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  /// @brief A box whose content is outside CSS: images, media, frames, form controls.
  // FIXME: The intrinsic size, aspect ratio, replaced content geometry, selection and painting come with
  // layout and painting; images, media and the other replaced elements come with their renderers.
  class RenderReplaced : public RenderBox
  {
    KRYS_NON_COPY_MOVABLE(RenderReplaced);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderReplaced);

  public:
    ~RenderReplaced() noexcept override;

    KRYS_NODISCARD bool CanHaveChildren() const noexcept override
    {
      return false;
    }

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderReplaced";
    }

  protected:
    RenderReplaced(Type type, dom::Element &element, RenderStyle &&style,
                   EnumSet<ReplacedFlag> flags = {}) noexcept;
    RenderReplaced(Type type, dom::Document &document, RenderStyle &&style,
                   EnumSet<ReplacedFlag> flags = {}) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderReplaced)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderReplaced();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
