#pragma once

#include "Krystal.Booey/Graphics/LayoutRect.hpp"
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
