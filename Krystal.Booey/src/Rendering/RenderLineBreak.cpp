#include "Krystal.Booey/Rendering/RenderLineBreak.hpp"
#include "Krystal.Booey/HTML/HTMLWBRElement.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationLineLayout.hpp"

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

  IntRect RenderLineBreak::LinesBoundingBox() const noexcept
  {
    auto run = inline_iterator::BoxFor(*this);
    if (!run)
    {
      return {};
    }

    return EnclosingIntRect(run->VisualRectIgnoringBlockDirection());
  }
}
