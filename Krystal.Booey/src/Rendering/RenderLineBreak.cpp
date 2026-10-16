#include "Krystal.Booey/Rendering/RenderLineBreak.hpp"
#include "Krystal.Booey/HTML/HTMLWBRElement.hpp"

namespace krys::boo
{
  RenderLineBreak::RenderLineBreak(html::HTMLElement &element, RenderStyle &&style) noexcept
      : RenderBoxModelObject(Type::LineBreak, element, krys::move(style), {},
                             Is<html::HTMLWBRElement>(element) ? EnumSet<LineBreakFlag> {LineBreakFlag::IsWBR}
                                                               : EnumSet<LineBreakFlag> {})
  {
    krys_debug_assert(IsRenderLineBreak());
  }

  RenderLineBreak::~RenderLineBreak() noexcept = default;

  void RenderLineBreak::UpdateFromStyle() noexcept
  {
    krys_assert(IsInline());
  }
}
