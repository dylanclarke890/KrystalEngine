/**
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2006, 2013 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "Krystal.Booey/Rendering/RenderLineBreak.hpp"
#include "Krystal.Booey/DOM/Ranges/Position.hpp"
#include "Krystal.Booey/Editing/Selection/TextAffinity.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatQuad.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLWBRElement.hpp"
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

  bool RenderLineBreak::CanBeSelectionLeaf() const noexcept
  {
    return true;
  }

  void RenderLineBreak::UpdateFromStyle() noexcept
  {
    krys_assert(IsInline());
  }

  dom::PositionWithAffinity RenderLineBreak::PositionForPoint(const LayoutPoint &, HitTestSource,
                                                              const RenderFragmentContainer *) noexcept
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
