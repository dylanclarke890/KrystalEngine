#include "Krystal.Booey/Rendering/RenderListItem.hpp"

namespace krys::boo
{
  RenderListItem::RenderListItem(dom::Element &element, RenderStyle &&style) noexcept
      : RenderBlockFlow(Type::ListItem, element, krys::move(style))
  {
    krys_debug_assert(IsRenderListItem());
    SetInline(false);
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderListItem::~RenderListItem() noexcept = default;
}
