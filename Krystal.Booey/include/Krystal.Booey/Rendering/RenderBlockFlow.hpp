#pragma once

#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  /// @brief A block container that lays out block or inline content.
  // FIXME: Block and inline layout (line layout, floats, margins, pagination, multi-column flows), text
  // autosizing and painting come with layout and painting.
  class RenderBlockFlow : public RenderBlock
  {
    KRYS_NON_COPY_MOVABLE(RenderBlockFlow);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderBlockFlow);

  public:
    RenderBlockFlow(Type type, dom::Element &element, RenderStyle &&style,
                    EnumSet<BlockFlowFlag> flags = {}) noexcept;
    RenderBlockFlow(Type type, dom::Document &document, RenderStyle &&style,
                    EnumSet<BlockFlowFlag> flags = {}) noexcept;
    ~RenderBlockFlow() noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderBlockFlow)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderBlockFlow();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
