#include "Krystal.Booey/Rendering/RenderLineBreak.hpp"
#include "Krystal.Booey/DOM/Position.hpp"
#include "Krystal.Booey/Editing/TextAffinity.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/HTML/HTMLWBRElement.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorBox.hpp"
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

  dom::PositionWithAffinity RenderLineBreak::PositionForPoint(const LayoutPoint &, HitTestSource) noexcept
  {
    return CreatePositionWithAffinity(0, Affinity::Downstream);
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

  void RenderLineBreak::BoundingRects(List<LayoutRect> &rects,
                                      const LayoutPoint &accumulatedOffset) const noexcept
  {
    auto box = inline_iterator::BoxFor(*this);
    if (!box)
    {
      return;
    }

    auto rect = LayoutRect {box->VisualRectIgnoringBlockDirection()};
    rect.MoveBy(accumulatedOffset);
    rects.push_back(rect);
  }

  void RenderLineBreak::AbsoluteQuads(List<FloatQuad> &quads, bool *wasFixed) const noexcept
  {
    auto box = inline_iterator::BoxFor(*this);
    if (!box)
    {
      return;
    }

    auto rect = box->VisualRectIgnoringBlockDirection();
    quads.push_back(LocalToAbsoluteQuad(FloatRect(rect.Location(), rect.Size()),
                                        MapCoordinatesMode::UseTransforms, wasFixed));
  }
}
