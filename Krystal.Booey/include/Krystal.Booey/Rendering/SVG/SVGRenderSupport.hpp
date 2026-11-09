/**
 * Copyright (C) 2007 Rob Buis <buis@kde.org>
 * Copyright (C) 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2007 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2009 Google, Inc. All rights reserved.
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
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

#pragma once

#include "Krystal.Booey/Graphics/Geometry/FloatRect.hpp"
#include "Krystal.Booey/Rendering/LayoutRepainter.hpp"
#include "Krystal.Booey/Rendering/RepaintRectCalculation.hpp"
#include "Krystal.Booey/Rendering/VisibleRectContext.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Markable.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  class AffineTransform;
  class FloatPoint;
  class GraphicsContext;
  class LayoutRect;
  class LegacyRenderSVGRoot;
  class RenderElement;
  class RenderGeometryMap;
  class RenderLayerModelObject;
  class RenderStyle;
  class TransformState;
  struct PaintInfo;

  // SVGRendererSupport is a helper class sharing code between all SVG renderers.
  class SVGRenderSupport
  {
    KRYS_NON_COPY_MOVABLE(SVGRenderSupport);

  public:
    SVGRenderSupport() noexcept = delete;

    static void LayoutDifferentRootIfNeeded(const RenderElement &) noexcept;

    // Shares child layouting code between LegacyRenderSVGRoot/RenderSVG(Hidden)Container
    static void LayoutChildren(RenderElement &, bool selfNeedsLayout) noexcept;

    // Helper function determining wheter overflow is hidden
    KRYS_NODISCARD static bool IsOverflowHidden(const RenderElement &) noexcept;

    // Calculates the repaintRect in combination with filter, clipper and masker in local coordinates.
    static void
      IntersectRepaintRectWithResources(const RenderElement &, FloatRect &,
                                        RepaintRectCalculation = RepaintRectCalculation::Fast) noexcept;

    // Determines whether a container needs to be laid out because it's filtered and a child is being laid
    // out.
    KRYS_NODISCARD static bool FiltersForceContainerLayout(const RenderElement &) noexcept;

    // Determines whether the passed point lies in a clipping area
    KRYS_NODISCARD static bool PointInClippingArea(const RenderElement &, const FloatPoint &) noexcept;

    struct ContainerBoundingBoxes
    {
      Markable<FloatRect> ObjectBoundingBox {};
      FloatRect RepaintBoundingBox {};
    };

    KRYS_NODISCARD static ContainerBoundingBoxes
      ComputeContainerBoundingBoxes(const RenderElement &,
                                    RepaintRectCalculation = RepaintRectCalculation::Fast) noexcept;

    KRYS_NODISCARD static FloatRect
      ComputeContainerStrokeBoundingBox(const RenderElement &container) noexcept;
    KRYS_NODISCARD static bool PaintInfoIntersectsRepaintRect(const FloatRect &localRepaintRect,
                                                              const AffineTransform &localTransform,
                                                              const PaintInfo &) noexcept;

    // Important functions used by nearly all SVG renderers centralizing coordinate transformations / repaint
    // rect calculations
    KRYS_NODISCARD static LayoutRect ClippedOverflowRectForRepaint(const RenderElement &,
                                                                   const RenderLayerModelObject *container,
                                                                   VisibleRectContext) noexcept;
    KRYS_NODISCARD static Maybe<FloatRect>
      ComputeFloatVisibleRectInContainer(const RenderElement &, const FloatRect &,
                                         const RenderLayerModelObject *container,
                                         VisibleRectContext) noexcept;
    static const RenderElement &LocalToParentTransform(const RenderElement &, AffineTransform &) noexcept;
    static void MapLocalToContainer(const RenderElement &, const RenderLayerModelObject *ancestorContainer,
                                    TransformState &, bool *wasFixed) noexcept;
    static const RenderElement *PushMappingToContainer(const RenderElement &,
                                                       const RenderLayerModelObject *ancestorToStopAt,
                                                       RenderGeometryMap &) noexcept;
    KRYS_NODISCARD static LayoutRepainter::CheckForRepaint
      CheckForSVGRepaintDuringLayout(const RenderElement &) noexcept;

    KRYS_NODISCARD static FloatRect CalculateApproximateStrokeBoundingBox(const RenderElement &) noexcept;

    // Shared between SVG renderers and resources.
    static void ApplyStrokeStyleToContext(GraphicsContext &, const RenderStyle &,
                                          const RenderElement &) noexcept;

    // Determines if any ancestor's transform has changed.
    KRYS_NODISCARD static bool TransformToRootChanged(RenderElement *) noexcept;

    static void ClipContextToCSSClippingArea(GraphicsContext &, const RenderElement &renderer) noexcept;

    static void StyleChanged(RenderElement &, const RenderStyle *) noexcept;

    KRYS_NODISCARD static bool IsolatesBlending(const RenderStyle &) noexcept;
    static void UpdateMaskedAncestorShouldIsolateBlending(const RenderElement &) noexcept;

    KRYS_NODISCARD static LegacyRenderSVGRoot *FindTreeRootObject(RenderElement &) noexcept;
    KRYS_NODISCARD static const LegacyRenderSVGRoot *FindTreeRootObject(const RenderElement &) noexcept;
  };
}
