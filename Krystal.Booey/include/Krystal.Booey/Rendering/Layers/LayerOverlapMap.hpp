/*
 * Copyright (C) 2019 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Geometry/LayoutRect.hpp"
#include "Krystal.Booey/Rendering/RenderGeometryMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  class OverlapMapContainer;
  class RenderLayer;

  /// @brief The bounds of the layers composited so far, which later layers that overlap them must be
  /// composited above.
  class LayerOverlapMap
  {
    KRYS_NON_COPY_MOVABLE(LayerOverlapMap);

  public:
    struct LayerAndBounds
    {
      const RenderLayer *Layer;
      LayoutRect Bounds;
    };

    using LayerAndBoundsVector = List<LayerAndBounds>;

  private:
    List<UniquePtr<OverlapMapContainer>> _overlapStack;
    List<UniquePtr<OverlapMapContainer>> _speculativeOverlapStack;
    RenderGeometryMap _geometryMap;
    const RenderLayer &_rootLayer;
    bool _isEmpty {true};

  public:
    explicit LayerOverlapMap(const RenderLayer &rootLayer) noexcept;
    ~LayerOverlapMap() noexcept;

    void Add(const RenderLayer &layer, const LayoutRect &bounds,
             const LayerAndBoundsVector &enclosingClippingLayers) noexcept;
    KRYS_NODISCARD bool OverlapsLayers(const RenderLayer &layer, const LayoutRect &bounds,
                                       const LayerAndBoundsVector &enclosingClippingLayers) const noexcept;
    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _isEmpty;
    }

    void PushCompositingContainer(const RenderLayer &layer) noexcept;
    void PopCompositingContainer(const RenderLayer &layer) noexcept;

    void PushSpeculativeCompositingContainer(const RenderLayer &layer) noexcept;
    void ConfirmSpeculativeCompositingContainer() noexcept;
    bool MaybePopSpeculativeCompositingContainer() noexcept;

    KRYS_NODISCARD const RenderGeometryMap &GeometryMap() const noexcept
    {
      return _geometryMap;
    }
    KRYS_NODISCARD RenderGeometryMap &GeometryMap() noexcept
    {
      return _geometryMap;
    }

    KRYS_NODISCARD const List<UniquePtr<OverlapMapContainer>> &OverlapStack() const noexcept
    {
      return _overlapStack;
    }
  };

  TextStream &operator<<(TextStream &ts, const LayerOverlapMap &overlapMap) noexcept;
}
