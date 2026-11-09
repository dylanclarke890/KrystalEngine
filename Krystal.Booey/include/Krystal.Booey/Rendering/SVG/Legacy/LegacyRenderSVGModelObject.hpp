/*
 * Copyright (c) 2009 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Rendering/RenderElement.hpp"

namespace krys::boo::svg
{
  class SVGElement;
}

namespace krys::boo
{
  // Most renderers in the SVG rendering tree will inherit from this class
  // but not all. (e.g. LegacyRenderSVGForeignObject, RenderSVGBlock) thus methods
  // required by SVG renders need to be declared on RenderObject, but shared
  // logic can go in this class or in SVGRenderSupport.
  class LegacyRenderSVGModelObject : public RenderElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LegacyRenderSVGModelObject);

  public:
    ~LegacyRenderSVGModelObject() noexcept override;

    KRYS_NODISCARD LayoutRect ClippedOverflowRect(const RenderLayerModelObject *repaintContainer,
                                                  VisibleRectContext) const noexcept override;
    KRYS_NODISCARD RepaintRects RectsForRepaintingAfterLayout(const RenderLayerModelObject *repaintContainer,
                                                              RepaintOutlineBounds) const noexcept override;

    KRYS_NODISCARD Maybe<FloatRect>
      ComputeFloatVisibleRectInContainer(const FloatRect &, const RenderLayerModelObject *container,
                                         VisibleRectContext) const noexcept final;
    KRYS_NODISCARD LayoutRect
      OutlineBoundsForRepaint(const RenderLayerModelObject *repaintContainer) const noexcept final;

    void BoundingRects(List<LayoutRect> &, const LayoutPoint &accumulatedOffset) const noexcept final;
    void AbsoluteQuads(List<FloatQuad> &, bool *wasFixed) const noexcept override;

    void MapLocalToContainer(const RenderLayerModelObject *ancestorContainer, TransformState &,
                             EnumSet<MapCoordinatesMode>, bool *wasFixed) const noexcept final;
    const RenderElement *PushMappingToContainer(const RenderLayerModelObject *ancestorToStopAt,
                                                RenderGeometryMap &) const noexcept final;
    void StyleDidChange(style::Difference, const RenderStyle *oldStyle) noexcept override;

    KRYS_NODISCARD static bool CheckIntersection(RenderElement *, const FloatRect &) noexcept;
    KRYS_NODISCARD static bool CheckEnclosure(RenderElement *, const FloatRect &) noexcept;

    KRYS_NODISCARD svg::SVGElement &GetElement() const noexcept;

    virtual void AddFocusRingRects(List<LayoutRect> &, const LayoutPoint &additionalOffset,
                                   const RenderLayerModelObject *paintContainer = nullptr) const noexcept;

  protected:
    LegacyRenderSVGModelObject(Type, svg::SVGElement &, RenderStyle &&,
                               EnumSet<SVGModelObjectFlag> = {}) noexcept;

    void WillBeDestroyed() noexcept override;

  private:
    // This method should never be called, SVG uses a different nodeAtPoint method
    bool NodeAtPoint(const HitTestRequest &, HitTestResult &, const HitTestLocation &locationInContainer,
                     const LayoutPoint &accumulatedOffset, HitTestAction) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::LegacyRenderSVGModelObject)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsLegacyRenderSVGModelObject();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
