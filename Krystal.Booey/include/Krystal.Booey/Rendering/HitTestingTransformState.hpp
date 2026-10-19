#pragma once

#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief The point and area of a hit test in the plane of the nearest flattening layer, and the transform
  /// from that plane to the current layer, accumulated through transformed and preserve-3d layers.
  // FIXME: Now that TransformState lazily creates its TransformationMatrix it takes up less space.
  // So there's really no need for a ref counted version. So This class should be removed and replaced
  // with TransformState. There are some minor differences (like the way translate() works slightly
  // differently than move()) so care has to be taken when this is done.
  class HitTestingTransformState : public RefCounted<HitTestingTransformState>
  {
  public:
    FloatPoint LastPlanarPoint;
    FloatQuad LastPlanarQuad;
    FloatQuad LastPlanarArea;
    TransformationMatrix AccumulatedTransform;

    KRYS_NODISCARD static Ref<HitTestingTransformState> Create(const FloatPoint &p, const FloatQuad &quad,
                                                               const FloatQuad &area) noexcept
    {
      return AdoptRef(*new HitTestingTransformState(p, quad, area));
    }

    KRYS_NODISCARD static Ref<HitTestingTransformState> Create(const HitTestingTransformState &other) noexcept
    {
      return AdoptRef(*new HitTestingTransformState(other));
    }

    void Translate(int32 x, int32 y) noexcept;
    void ApplyTransform(const TransformationMatrix &transformFromContainer) noexcept;

    KRYS_NODISCARD FloatPoint MappedPoint() const noexcept;
    KRYS_NODISCARD FloatQuad MappedQuad() const noexcept;
    KRYS_NODISCARD FloatQuad MappedArea() const noexcept;
    KRYS_NODISCARD LayoutRect BoundsOfMappedArea() const noexcept;
    KRYS_NODISCARD LayoutRect BoundsOfMappedQuad() const noexcept;

    void Flatten() noexcept;

  private:
    HitTestingTransformState(const FloatPoint &p, const FloatQuad &quad, const FloatQuad &area) noexcept
        : LastPlanarPoint(p), LastPlanarQuad(quad), LastPlanarArea(area)
    {
    }

    HitTestingTransformState(const HitTestingTransformState &other) noexcept
        : RefCounted<HitTestingTransformState>(), LastPlanarPoint(other.LastPlanarPoint),
          LastPlanarQuad(other.LastPlanarQuad), LastPlanarArea(other.LastPlanarArea),
          AccumulatedTransform(other.AccumulatedTransform)
    {
    }
  };
}
