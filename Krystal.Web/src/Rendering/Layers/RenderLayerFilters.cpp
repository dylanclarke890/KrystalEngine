/*
 * Copyright (C) 2012 Adobe Systems Incorporated. All rights reserved.
 * Copyright (C) 2013-2023 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "Krystal.Web/Rendering/Layers/RenderLayerFilters.hpp"
#include "Krystal.Web/Dom/Mixins/NonElementParentNode.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Graphics/GraphicsContext.hpp"
#include "Krystal.Web/Graphics/GraphicsContextSwitcher.hpp"
#include "Krystal.Web/Page/Frame.hpp"
#include "Krystal.Web/Rendering/Layers/RenderLayer.hpp"
#include "Krystal.Web/Rendering/Painting/CSSFilterRenderer.hpp"
#include "Krystal.Web/Rendering/RenderElement.hpp"
#include "Krystal.Web/Rendering/RenderLayerModelObject.hpp"
#include "Krystal.Web/Rendering/Style/ReferenceFilterOperation.hpp"
#include "Krystal.Web/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Web/Rendering/SVG/Legacy/LegacyRenderSVGResourceFilter.hpp"
#include "Krystal.Web/Style/Values/FilterEffects/Filter.hpp"

namespace krys::web
{
  Ref<RenderLayerFilters> RenderLayerFilters::Create(RenderLayer &layer, SizeF scale) noexcept
  {
    return AdoptRef(*new RenderLayerFilters(layer, scale));
  }

  RenderLayerFilters::RenderLayerFilters(RenderLayer &layer, SizeF scale) noexcept
      : _layer(CreateWeakPtr(&layer)), _filterScale(scale)
  {
  }

  RenderLayerFilters::~RenderLayerFilters() noexcept
  {
    RemoveReferenceFilterClients();
  }

  void RenderLayerFilters::ClearFilter() noexcept
  {
    _filter = nullptr;
  }

  bool RenderLayerFilters::HasFilterThatMovesPixels() const noexcept
  {
    return _filter && _filter->HasFilterThatMovesPixels();
  }

  bool RenderLayerFilters::HasFilterThatShouldBeRestrictedBySecurityOrigin() const noexcept
  {
    return _filter && _filter->HasFilterThatShouldBeRestrictedBySecurityOrigin();
  }

  bool RenderLayerFilters::HasSourceImage() const noexcept
  {
    return _targetSwitcher && _targetSwitcher->HasSourceImage();
  }

  void RenderLayerFilters::UpdateReferenceFilterClients(const style::Filter &filter) noexcept
  {
    RemoveReferenceFilterClients();

    for (const auto &value : filter)
    {
      auto *referenceOperation = DynamicDowncast<style::ReferenceFilterOperation>(*value);
      if (!referenceOperation)
      {
        continue;
      }

      // FIXME: An external reference (an SVG document a reference filter loads), waited on with
      // notifyFinished, once reference filters load their documents.

      // Reference is internal; add layer as a client so we can trigger filter repaint on SVG attribute
      // change.
      auto *layer = _layer.get();
      if (!layer)
      {
        continue;
      }
      auto filterElement = dom::mixins::NonElementParentNode::GetElementById(
        layer->Renderer().GetDocument(), referenceOperation->Fragment().View());
      if (!filterElement)
      {
        continue;
      }
      auto *renderer = DynamicDowncast<LegacyRenderSVGResourceFilter>(filterElement->Renderer());
      if (!renderer)
      {
        continue;
      }
      renderer->AddClientRenderLayer(*layer);
      _internalSVGReferences.emplace_back(filterElement);
    }
  }

  void RenderLayerFilters::RemoveReferenceFilterClients() noexcept
  {
    if (auto *layer = _layer.get())
    {
      for (const auto &filterElement : _internalSVGReferences)
      {
        if (auto *renderer = filterElement->Renderer())
        {
          Downcast<LegacyRenderSVGResourceContainer>(*renderer).RemoveClientRenderLayer(*layer);
        }
      }
    }
    _internalSVGReferences.clear();
  }

  bool RenderLayerFilters::IsIdentity(RenderElement &renderer) noexcept
  {
    const auto &filter = renderer.Style().Filter();
    return CSSFilterRenderer::IsIdentity(renderer, filter);
  }

  IntOutsets RenderLayerFilters::CalculateOutsets(RenderElement &renderer,
                                                  const RectF &targetBoundingBox) noexcept
  {
    const auto &filter = renderer.Style().Filter();

    if (!filter.HasFilterThatMovesPixels())
    {
      return {};
    }

    return CSSFilterRenderer::CalculateOutsets(renderer, filter, targetBoundingBox);
  }

  GraphicsContext *RenderLayerFilters::BeginFilterEffect(RenderElement &renderer, GraphicsContext &context,
                                                         const LayoutRect &filterBoxRect,
                                                         const LayoutRect &dirtyRect,
                                                         const LayoutRect &layerRepaintRect,
                                                         const LayoutRect &clipRect) noexcept
  {
    auto *frame = renderer.GetDocument().Frame();
    if (!frame)
    {
      return nullptr;
    }
    auto preferredFilterRenderingModes = frame->PreferredFilterRenderingModes(context);
    auto outsets = CalculateOutsets(renderer, RectF(filterBoxRect));

    auto dirtyFilterRegion = dirtyRect;
    auto filterRegion = dirtyRect;

    if (!outsets.IsZero())
    {
      // FIXME: This flipping was added for drop-shadow, but it's not obvious that it's correct.
      LayoutBoxExtent flippedOutsets {LayoutUnit {outsets.Bottom()}, LayoutUnit {outsets.Left()},
                                      LayoutUnit {outsets.Top()}, LayoutUnit {outsets.Right()}};
      Expand(dirtyFilterRegion, flippedOutsets);
    }

    dirtyFilterRegion = Intersection(filterBoxRect, dirtyFilterRegion);
    filterRegion = dirtyFilterRegion;

    if (!outsets.IsZero())
    {
      Expand(filterRegion, ToLayoutBoxExtent(outsets));
    }

    if (filterRegion.IsEmpty())
    {
      return nullptr;
    }

    auto geometryReferenceGeometryChanged =
      [](const FilterGeometry &existingGeometry, const FilterGeometry &newGeometry)
    {
      return existingGeometry.ReferenceBox != newGeometry.ReferenceBox
             || existingGeometry.Scale != newGeometry.Scale;
    };

    auto geometry = FilterGeometry {
      .ReferenceBox = RectF(filterBoxRect),
      .FilterRegion = RectF(filterRegion),
      .Scale = _filterScale,
    };

    bool hasUpdatedBackingStore = false;
    if (!_filter || geometryReferenceGeometryChanged(_filter->Geometry(), geometry)
        || _preferredFilterRenderingModes != preferredFilterRenderingModes)
    {
      // FIXME: This rebuilds the entire effects chain even if the filter style didn't change.
      _filter = CSSFilterRenderer::Create(renderer, renderer.Style().Filter(), geometry,
                                          preferredFilterRenderingModes, context);
      hasUpdatedBackingStore = true;
    }
    else if (RectF(filterRegion) != _filter->FilterRegion())
    {
      _filter->SetFilterRegion(RectF(filterRegion));
      hasUpdatedBackingStore = true;
    }

    _preferredFilterRenderingModes = preferredFilterRenderingModes;

    if (!_filter)
    {
      return nullptr;
    }

    Ref<CSSFilterRenderer> filter = _filter;
    if (!filter->HasFilterThatMovesPixels())
    {
      _repaintRect = dirtyRect;
    }
    else if (hasUpdatedBackingStore || !HasSourceImage())
    {
      _repaintRect = filterRegion;
    }
    else
    {
      _repaintRect = dirtyRect;
      _repaintRect.Unite(layerRepaintRect);
      _repaintRect.Intersect(filterRegion);
    }

    ResetDirtySourceRect();

    if (!_targetSwitcher || hasUpdatedBackingStore)
    {
      auto sourceImageRect = RectF(dirtyFilterRegion);
      _targetSwitcher = GraphicsContextSwitcher::Create(context, sourceImageRect, filter);
    }

    if (!_targetSwitcher)
    {
      return nullptr;
    }

    _targetSwitcher->BeginClipAndDrawSourceImage(context, RectF(_repaintRect), RectF(clipRect));

    return _targetSwitcher->DrawingContext(context);
  }

  void RenderLayerFilters::ApplyFilterEffect(GraphicsContext &destinationContext) noexcept
  {
    krys_debug_assert(_targetSwitcher);
    _targetSwitcher->EndClipAndDrawSourceImage(destinationContext);
  }
}
