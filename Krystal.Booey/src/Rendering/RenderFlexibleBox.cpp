#include "Krystal.Booey/Rendering/RenderFlexibleBox.hpp"

namespace krys::boo
{
  RenderFlexibleBox::RenderFlexibleBox(Type type, dom::Element &element, RenderStyle &&style) noexcept
      : RenderBlock(type, element, krys::move(style), TypeFlag::IsFlexibleBox)
  {
    krys_debug_assert(IsRenderFlexibleBox());
    SetChildrenInline(false); // All of our children must be block-level.
  }

  RenderFlexibleBox::RenderFlexibleBox(Type type, dom::Document &document, RenderStyle &&style) noexcept
      : RenderBlock(type, document, krys::move(style), TypeFlag::IsFlexibleBox)
  {
    krys_debug_assert(IsRenderFlexibleBox());
    SetChildrenInline(false); // All of our children must be block-level.
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderFlexibleBox::~RenderFlexibleBox() noexcept = default;
}
