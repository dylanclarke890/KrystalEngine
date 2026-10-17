#pragma once

#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/Graphics/LayoutSize.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  /// @brief A point, quad or both mapped through a chain of offsets and transforms, from a renderer to one of
  /// its ancestors or the other way round.
  class TransformState
  {
  public:
    enum class TransformDirection : bool
    {
      ApplyTransform,
      UnapplyInverseTransform
    };

    enum class TransformAccumulation : bool
    {
      FlattenTransform,
      AccumulateTransform
    };

    enum class TransformMatrixTracking : uint8
    {
      DoNotTrackTransformMatrix,
      TrackSVGCTMMatrix,
      TrackSVGScreenCTMMatrix
    };

  private:
    FloatPoint _inputPoint;
    FloatQuad _inputQuad;
    Maybe<FloatQuad> _inputSecondaryQuad;

    // We only allocate the transform if we need to
    UniquePtr<TransformationMatrix> _accumulatedTransform;
    LayoutSize _accumulatedOffset;
    TransformMatrixTracking _tracking {TransformMatrixTracking::DoNotTrackTransformMatrix};
    TransformDirection _direction;

  public:
    TransformState(TransformDirection mappingDirection, const FloatPoint &p, const FloatQuad &quad) noexcept
        : _inputPoint(p), _inputQuad(quad), _direction(mappingDirection)
    {
    }

    TransformState(TransformDirection mappingDirection, const FloatPoint &p) noexcept
        : _inputPoint(p), _direction(mappingDirection)
    {
    }

    TransformState(TransformDirection mappingDirection, const FloatQuad &quad,
                   Maybe<FloatQuad> secondaryQuad = null) noexcept
        : _inputQuad(quad), _inputSecondaryQuad(secondaryQuad), _direction(mappingDirection)
    {
    }

    TransformState(const TransformState &other) noexcept
    {
      *this = other;
    }
    TransformState &operator=(const TransformState &other) noexcept;

    void Reset(const FloatQuad &quad, const Maybe<FloatQuad> &secondaryQuad = null) noexcept;

    void SetSecondaryQuadInMappedSpace(const Maybe<FloatQuad> &quad) noexcept;

    void SetTransformMatrixTracking(TransformMatrixTracking tracking) noexcept
    {
      _tracking = tracking;
    }
    KRYS_NODISCARD TransformMatrixTracking GetTransformMatrixTracking() const noexcept
    {
      return _tracking;
    }

    void Move(LayoutUnit x, LayoutUnit y,
              TransformAccumulation accumulate = TransformAccumulation::FlattenTransform) noexcept
    {
      Move(LayoutSize(x, y), accumulate);
    }
    void Move(const LayoutSize &offset,
              TransformAccumulation accumulate = TransformAccumulation::FlattenTransform) noexcept;

    void ApplyTransform(const AffineTransform &transformFromContainer,
                        TransformAccumulation accumulate = TransformAccumulation::FlattenTransform,
                        bool *wasClamped = nullptr) noexcept;
    void ApplyTransform(const TransformationMatrix &transformFromContainer,
                        TransformAccumulation accumulate = TransformAccumulation::FlattenTransform,
                        bool *wasClamped = nullptr) noexcept;

    KRYS_NODISCARD bool IsMappingSecondaryQuad() const noexcept
    {
      return _inputSecondaryQuad.has_value();
    }

    /// @brief The point mapped through the current transform.
    KRYS_NODISCARD FloatPoint MappedPoint() const noexcept;
    /// @brief The quad mapped through the current transform.
    KRYS_NODISCARD FloatQuad MappedQuad(bool *wasClamped = nullptr) const noexcept;
    KRYS_NODISCARD Maybe<FloatQuad> MappedSecondaryQuad(bool *wasClamped = nullptr) const noexcept;

    KRYS_NODISCARD LayoutSize AccumulatedOffset() const noexcept
    {
      return _accumulatedOffset;
    }
    KRYS_NODISCARD TransformationMatrix *AccumulatedTransform() const noexcept
    {
      return _accumulatedTransform.get();
    }
    KRYS_NODISCARD UniquePtr<TransformationMatrix> ReleaseTrackedTransform() const noexcept;
    KRYS_NODISCARD TransformDirection Direction() const noexcept
    {
      return _direction;
    }

    void Flatten() noexcept;

  private:
    void TranslateTransform(const LayoutSize &offset, TransformDirection direction) noexcept;

    KRYS_NODISCARD bool ShouldFlattenBefore(TransformAccumulation accumulate) const noexcept
    {
      return accumulate == TransformAccumulation::FlattenTransform
             && _direction != TransformDirection::ApplyTransform;
    }
    KRYS_NODISCARD bool ShouldFlattenAfter(TransformAccumulation accumulate) const noexcept
    {
      return accumulate == TransformAccumulation::FlattenTransform
             && _direction == TransformDirection::ApplyTransform;
    }
    KRYS_NODISCARD TransformDirection InverseDirection() const noexcept
    {
      return _direction == TransformDirection::ApplyTransform ? TransformDirection::UnapplyInverseTransform
                                                              : TransformDirection::ApplyTransform;
    }

    void MapQuad(FloatQuad &quad, TransformDirection direction, bool *wasClamped = nullptr) const noexcept;
  };
}
