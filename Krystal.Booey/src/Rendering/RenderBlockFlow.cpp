#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"

namespace krys::boo
{
  RenderBlockFlow::RenderBlockFlow(Type type, dom::Element &element, RenderStyle &&style,
                                   EnumSet<BlockFlowFlag> flags) noexcept
      : RenderBlock(type, element, krys::move(style), {}, flags)
  {
    krys_debug_assert(IsRenderBlockFlow());
    SetChildrenInline(true);
  }

  RenderBlockFlow::RenderBlockFlow(Type type, dom::Document &document, RenderStyle &&style,
                                   EnumSet<BlockFlowFlag> flags) noexcept
      : RenderBlock(type, document, krys::move(style), {}, flags)
  {
    krys_debug_assert(IsRenderBlockFlow());
    SetChildrenInline(true);
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderBlockFlow::~RenderBlockFlow() noexcept = default;
}
