#pragma once

#include "Krystal.Booey/Graphics/GraphicsLayer.hpp"
#include "Krystal.Booey/Graphics/LayoutRoundedRect.hpp"
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
  // FIXME: The scrolling coordinator's overflow scroll proxy nodes, once there's async scrolling.
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
