#pragma once

#include "Krystal.Booey/Graphics/Filters/FilterEffect.hpp"
#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief How a filter primitive reads past its input's edges.
  enum class EdgeModeType : uint8
  {
    Unknown,
    Duplicate,
    Wrap,
    None
  };

  /// @brief The convolve matrix filter effect: a kernel convolved with its input.
  class FEConvolveMatrix final : public FilterEffect
  {
  private:
    IntSize _kernelSize;
    float _divisor;
    float _bias;
    IntPoint _targetOffset;
    EdgeModeType _edgeMode;
    FloatPoint _kernelUnitLength;
    bool _preserveAlpha;
    List<float> _kernelMatrix;

  public:
    KRYS_NODISCARD static Ref<FEConvolveMatrix>
      Create(const IntSize &kernelSize, float divisor, float bias, const IntPoint &targetOffset,
             EdgeModeType edgeMode, const FloatPoint &kernelUnitLength, bool preserveAlpha,
             const List<float> &kernelMatrix,
             const DestinationColorSpace &colorSpace = DestinationColorSpace::SRGB()) noexcept;

    KRYS_NODISCARD bool operator==(const FEConvolveMatrix &other) const noexcept;

    KRYS_NODISCARD IntSize KernelSize() const noexcept
    {
      return _kernelSize;
    }
    void SetKernelSize(const IntSize &kernelSize) noexcept;

    KRYS_NODISCARD const List<float> &Kernel() const noexcept
    {
      return _kernelMatrix;
    }
    void SetKernel(const List<float> &kernel) noexcept;

    KRYS_NODISCARD float Divisor() const noexcept
    {
      return _divisor;
    }
    bool SetDivisor(float divisor) noexcept;

    KRYS_NODISCARD float Bias() const noexcept
    {
      return _bias;
    }
    bool SetBias(float bias) noexcept;

    KRYS_NODISCARD IntPoint TargetOffset() const noexcept
    {
      return _targetOffset;
    }
    bool SetTargetOffset(const IntPoint &targetOffset) noexcept;

    KRYS_NODISCARD EdgeModeType EdgeMode() const noexcept
    {
      return _edgeMode;
    }
    bool SetEdgeMode(EdgeModeType edgeMode) noexcept;

    KRYS_NODISCARD FloatPoint KernelUnitLength() const noexcept
    {
      return _kernelUnitLength;
    }
    bool SetKernelUnitLength(const FloatPoint &kernelUnitLength) noexcept;

    KRYS_NODISCARD bool PreserveAlpha() const noexcept
    {
      return _preserveAlpha;
    }
    bool SetPreserveAlpha(bool preserveAlpha) noexcept;

  private:
    FEConvolveMatrix(const IntSize &kernelSize, float divisor, float bias, const IntPoint &targetOffset,
                     EdgeModeType edgeMode, const FloatPoint &kernelUnitLength, bool preserveAlpha,
                     const List<float> &kernelMatrix, const DestinationColorSpace &colorSpace) noexcept;

    KRYS_NODISCARD bool operator==(const FilterEffect &other) const noexcept override
    {
      return AreEqual<FEConvolveMatrix>(*this, other);
    }

    KRYS_NODISCARD FloatRect CalculateImageRect(const Filter &filter, Span<const FloatRect> inputImageRects,
                                                const FloatRect &primitiveSubregion) const noexcept override;

    TextStream &ExternalRepresentation(TextStream &ts,
                                       FilterRepresentation representation) const noexcept override;
  };

  TextStream &operator<<(TextStream &ts, const EdgeModeType &type) noexcept;
}

KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_FUNCTION(FEConvolveMatrix)
