#pragma once

#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  /// @brief A box that contains other boxes: a block container, flex, grid or table.
  // FIXME: Layout (block layout, positioned and percent-height descendants, floats), line boxes,
  // continuations, selection and painting come with layout and painting.
  class RenderBlock : public RenderBox
  {
    KRYS_NON_COPY_MOVABLE(RenderBlock);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderBlock);

  public:
    ~RenderBlock() noexcept override;

    KRYS_NODISCARD bool CanHaveChildren() const noexcept override
    {
      return true;
    }

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override;

    // FIXME: Layout queries, reachable only through `RendererOf` and `RenderBoxOf`, which return null until
    // there's layout.
    KRYS_NODISCARD LayoutUnit PaddingBoxWidth() const noexcept;
    KRYS_NODISCARD LayoutUnit PaddingBoxHeight() const noexcept;

  protected:
    RenderBlock(Type type, dom::Element &element, RenderStyle &&style, EnumSet<TypeFlag> baseTypeFlags = {},
                TypeSpecificFlags typeSpecificFlags = {}) noexcept;
    RenderBlock(Type type, dom::Document &document, RenderStyle &&style, EnumSet<TypeFlag> baseTypeFlags = {},
                TypeSpecificFlags typeSpecificFlags = {}) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderBlock)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderBlock();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
