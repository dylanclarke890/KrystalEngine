#include "Krystal.Booey/Rendering/RenderTableCaption.hpp"

namespace krys::boo
{
  RenderTableCaption::RenderTableCaption(dom::Element &element, RenderStyle &&style) noexcept
      : RenderBlockFlow(Type::TableCaption, element, krys::move(style))
  {
    krys_debug_assert(IsRenderTableCaption());
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderTableCaption::~RenderTableCaption() noexcept = default;
}
