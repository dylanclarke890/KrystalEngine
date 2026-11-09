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

#pragma once

#include "Krystal.Booey/Graphics/Filters/FilterRenderingMode.hpp"
#include "Krystal.Booey/Graphics/Geometry/BoxExtents.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatRect.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutRect.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextSwitcher.hpp"
#include "Krystal.Booey/Rendering/Painting/CSSFilterRenderer.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo
{
  class GraphicsContext;
  class RenderElement;
  class RenderLayer;

  namespace style
  {
    struct Filter;
  }

  /// @brief A layer's filter as it paints: built from the renderer's `filter`, applied around what the layer
  /// draws.
  class RenderLayerFilters final : public RefCounted<RenderLayerFilters>
  {
  private:
    WeakPtr<RenderLayer> _layer;

    LayoutRect _dirtySourceRect;
    LayoutRect _repaintRect;

    FloatSize _filterScale {1, 1};

    EnumSet<FilterRenderingMode> _preferredFilterRenderingModes {FilterRenderingMode::Software};

    RefPtr<CSSFilterRenderer> _filter;
    UniquePtr<GraphicsContextSwitcher> _targetSwitcher;

    List<Ref<dom::Element>> _internalSVGReferences;

  public:
    KRYS_NODISCARD static Ref<RenderLayerFilters> Create(RenderLayer &layer, FloatSize scale) noexcept;
    ~RenderLayerFilters() noexcept;

    void DetachFromLayer() noexcept
    {
      _layer = nullptr;
    }

    KRYS_NODISCARD const LayoutRect &DirtySourceRect() const noexcept
    {
      return _dirtySourceRect;
    }
    void ExpandDirtySourceRect(const LayoutRect &rect) noexcept
    {
      _dirtySourceRect.Unite(rect);
    }

    KRYS_NODISCARD CSSFilterRenderer *Filter() const noexcept
    {
      return _filter.get();
    }
    void ClearFilter() noexcept;

    KRYS_NODISCARD bool HasFilterThatMovesPixels() const noexcept;
    KRYS_NODISCARD bool HasFilterThatShouldBeRestrictedBySecurityOrigin() const noexcept;
    KRYS_NODISCARD bool HasSourceImage() const noexcept;

    void UpdateReferenceFilterClients(const style::Filter &filter) noexcept;
    void RemoveReferenceFilterClients() noexcept;

    KRYS_NODISCARD static bool IsIdentity(RenderElement &renderer) noexcept;
    KRYS_NODISCARD static IntOutsets CalculateOutsets(RenderElement &renderer,
                                                      const FloatRect &targetBoundingBox) noexcept;

    // Per render
    KRYS_NODISCARD LayoutRect RepaintRect() const noexcept
    {
      return _repaintRect;
    }

    /// @brief Starts drawing the layer's contents through the filter.
    /// @return The context to draw them into, or null when the filter can't apply.
    GraphicsContext *BeginFilterEffect(RenderElement &renderer, GraphicsContext &context,
                                       const LayoutRect &filterBoxRect, const LayoutRect &dirtyRect,
                                       const LayoutRect &layerRepaintRect,
                                       const LayoutRect &clipRect) noexcept;
    /// @brief Finishes drawing through the filter, into @p destinationContext.
    void ApplyFilterEffect(GraphicsContext &destinationContext) noexcept;

  private:
    RenderLayerFilters(RenderLayer &layer, FloatSize scale) noexcept;

    void ResetDirtySourceRect() noexcept
    {
      _dirtySourceRect = LayoutRect();
    }
  };
}
