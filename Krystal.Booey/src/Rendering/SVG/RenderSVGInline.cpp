/*
 * Copyright (C) 2006 Oliver Hunt <ojh16@student.canterbury.ac.nz>
 * Copyright (C) 2006-2024 Apple Inc. All rights reserved.
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 * Copyright (C) 2013 Google Inc. All rights reserved.
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
 */

#include "Krystal.Booey/Rendering/SVG/RenderSVGInline.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorInlineBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResourcesCache.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInlineText.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGText.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderSupport.hpp"
#include "Krystal.Booey/SVG/Elements/SVGGraphicsElement.hpp"
#include "Krystal.Core/Geometry/Quad.hpp"

namespace krys::boo
{
  RenderSVGInline::RenderSVGInline(Type type, SVGGraphicsElement &element, RenderStyle &&style) noexcept
      : RenderInline(type, element, krys::move(style))
  {
    krys_debug_assert(IsRenderSVGInline());
  }

  RenderSVGInline::~RenderSVGInline() noexcept = default;

  SVGGraphicsElement &RenderSVGInline::GraphicsElement() const noexcept
  {
    return Downcast<SVGGraphicsElement>(Downcast<SVGElement>(*RenderInline::GetElement()));
  }

  bool RenderSVGInline::IsChildAllowed(const RenderObject &child, const RenderStyle &style) const noexcept
  {
    auto isEmptySVGInlineText = [](const RenderObject *object)
    {
      const auto *svgInlineText = DynamicDowncast<RenderSVGInlineText>(object);
      return svgInlineText && svgInlineText->HasEmptyText();
    };

    if (isEmptySVGInlineText(&child))
    {
      return false;
    }

    return RenderElement::IsChildAllowed(child, style);
  }

  RectF RenderSVGInline::ObjectBoundingBox() const noexcept
  {
    if (auto *textAncestor = RenderSVGText::LocateRenderSVGTextAncestor(*this))
    {
      return textAncestor->ObjectBoundingBox();
    }

    return RectF();
  }

  RectF RenderSVGInline::StrokeBoundingBox() const noexcept
  {
    if (auto *textAncestor = RenderSVGText::LocateRenderSVGTextAncestor(*this))
    {
      return textAncestor->StrokeBoundingBox();
    }

    return RectF();
  }

  RectF RenderSVGInline::RepaintRectInLocalCoordinates(
    RepaintRectCalculation repaintRectCalculation) const noexcept
  {
    if (auto *textAncestor = RenderSVGText::LocateRenderSVGTextAncestor(*this))
    {
      return textAncestor->RepaintRectInLocalCoordinates(repaintRectCalculation);
    }

    return RectF();
  }

  RectF RenderSVGInline::DecoratedBoundingBox() const noexcept
  {
    if (auto *textAncestor = RenderSVGText::LocateRenderSVGTextAncestor(*this))
    {
      return textAncestor->DecoratedBoundingBox();
    }

    return {};
  }

  LayoutRect RenderSVGInline::ClippedOverflowRect(const RenderLayerModelObject *repaintContainer,
                                                  VisibleRectContext context) const noexcept
  {
    return SVGRenderSupport::ClippedOverflowRectForRepaint(*this, repaintContainer, context);
  }

  auto
    RenderSVGInline::RectsForRepaintingAfterLayout(const RenderLayerModelObject *repaintContainer,
                                                   RepaintOutlineBounds repaintOutlineBounds) const noexcept
    -> RepaintRects
  {
    auto rects = RepaintRects {SVGRenderSupport::ClippedOverflowRectForRepaint(
      *this, repaintContainer, VisibleRectContextForRepaint())};
    if (repaintOutlineBounds == RepaintOutlineBounds::Yes)
    {
      rects.OutlineBoundsRect = OutlineBoundsForRepaint(repaintContainer);
    }

    return rects;
  }

  Maybe<RectF> RenderSVGInline::ComputeFloatVisibleRectInContainer(const RectF &rect,
                                                                   const RenderLayerModelObject *container,
                                                                   VisibleRectContext context) const noexcept
  {
    return SVGRenderSupport::ComputeFloatVisibleRectInContainer(*this, rect, container, context);
  }

  void RenderSVGInline::MapLocalToContainer(const RenderLayerModelObject *ancestorContainer,
                                            TransformState &transformState, EnumSet<MapCoordinatesMode>,
                                            bool *wasFixed) const noexcept
  {
    SVGRenderSupport::MapLocalToContainer(*this, ancestorContainer, transformState, wasFixed);
  }

  const RenderElement *RenderSVGInline::PushMappingToContainer(const RenderLayerModelObject *ancestorToStopAt,
                                                               RenderGeometryMap &geometryMap) const noexcept
  {
    return SVGRenderSupport::PushMappingToContainer(*this, ancestorToStopAt, geometryMap);
  }

  void RenderSVGInline::AbsoluteQuads(List<QuadF> &quads, bool *wasFixed) const noexcept
  {
    auto *textAncestor = RenderSVGText::LocateRenderSVGTextAncestor(*this);
    if (!textAncestor)
    {
      return;
    }

    RectF textBoundingBox = textAncestor->StrokeBoundingBox();
    for (auto box = inline_iterator::LineLeftmostInlineBoxFor(*this); box;
         box.TraverseInlineBoxLineRightward())
    {
      auto rect = box->VisualRectIgnoringBlockDirection();
      rect.MoveBy(textBoundingBox.Location());
      quads.push_back(LocalToAbsoluteQuad(rect, MapCoordinatesMode::UseTransforms, wasFixed));
    }
  }

  void RenderSVGInline::WillBeDestroyed() noexcept
  {
    SVGResourcesCache::ClientDestroyed(*this);
    RenderInline::WillBeDestroyed();
  }

  void RenderSVGInline::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    if (diff == style::DifferenceResult::Layout)
    {
      InvalidateCachedBoundaries();
    }
    RenderInline::StyleDidChange(diff, oldStyle);
    SVGResourcesCache::ClientStyleChanged(*this, diff, oldStyle, Style());
  }

  void RenderSVGInline::UpdateFromStyle() noexcept
  {
    RenderInline::UpdateFromStyle();

    // SVG text layout code expects us to be an inline-level element.
    SetInline(true);
  }
}
