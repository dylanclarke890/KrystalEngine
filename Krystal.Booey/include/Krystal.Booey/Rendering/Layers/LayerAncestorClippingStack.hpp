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

#include "Krystal.Booey/Graphics/Geometry/LayoutRoundedRect.hpp"
#include "Krystal.Booey/Graphics/Layers/GraphicsLayer.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  class RenderLayer;

  struct CompositedClipData
  {
    // For scroller entries, the scrolling layer. For other entries, the most-descendant layer that has a
    // clip.
    WeakPtr<RenderLayer> ClippingLayer;
    LayoutRoundedRect ClipRect; // In the coordinate system of the RenderLayer that owns the stack.
    bool IsOverflowScroll {false};

    CompositedClipData(RenderLayer *layer, const LayoutRoundedRect &roundedRect,
                       bool isOverflowScrollEntry) noexcept;

    KRYS_NODISCARD friend bool operator==(const CompositedClipData &a, const CompositedClipData &b) noexcept
    {
      return a.ClippingLayer.get() == b.ClippingLayer.get() && a.ClipRect == b.ClipRect
             && a.IsOverflowScroll == b.IsOverflowScroll;
    }
  };

  /// @brief Encapsulates the set of layers representing clipping in a layer's containing block ancestry, but
  /// not in its paint order ancestry.
  class LayerAncestorClippingStack
  {
    KRYS_NON_COPY_MOVABLE(LayerAncestorClippingStack);

  public:
    struct ClippingStackEntry
    {
      CompositedClipData ClipData;
      RefPtr<GraphicsLayer> ClippingLayer;
      RefPtr<GraphicsLayer> ScrollingLayer; // Only present for scrolling entries.

      KRYS_NODISCARD GraphicsLayer *ParentForSublayers() const noexcept
      {
        return ScrollingLayer ? ScrollingLayer.get() : ClippingLayer.get();
      }

      KRYS_NODISCARD GraphicsLayer *ChildForSuperlayers() const noexcept
      {
        return ClippingLayer.get();
      }
    };

  private:
    // Order is ancestors to descendants.
    List<ClippingStackEntry> _stack;

  public:
    explicit LayerAncestorClippingStack(List<CompositedClipData> &&clipDataStack) noexcept;
    ~LayerAncestorClippingStack() noexcept = default;

    KRYS_NODISCARD bool HasAnyScrollingLayers() const noexcept;

    KRYS_NODISCARD bool EqualToClipData(const List<CompositedClipData> &clipDataStack) const noexcept;
    bool UpdateWithClipData(List<CompositedClipData> &&clipDataStack) noexcept;

    KRYS_NODISCARD List<CompositedClipData> GetCompositedClipData() const noexcept;
    void Clear() noexcept;

    KRYS_NODISCARD GraphicsLayer *FirstLayer() const noexcept;
    KRYS_NODISCARD GraphicsLayer *LastLayer() const noexcept;

    KRYS_NODISCARD List<ClippingStackEntry> &Stack() noexcept
    {
      return _stack;
    }
    KRYS_NODISCARD const List<ClippingStackEntry> &Stack() const noexcept
    {
      return _stack;
    }
  };

  TextStream &operator<<(TextStream &ts, const LayerAncestorClippingStack &clipStack) noexcept;
}
