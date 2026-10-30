#include "Krystal.Booey/Graphics/Filters/FEConvolveMatrix.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  Ref<FEConvolveMatrix> FEConvolveMatrix::Create(const IntSize &kernelSize, float divisor, float bias,
                                                 const IntPoint &targetOffset, EdgeModeType edgeMode,
                                                 const FloatPoint &kernelUnitLength, bool preserveAlpha,
                                                 const List<float> &kernelMatrix,
                                                 const DestinationColorSpace &colorSpace) noexcept
  {
    return AdoptRef(*new FEConvolveMatrix(kernelSize, divisor, bias, targetOffset, edgeMode, kernelUnitLength,
                                          preserveAlpha, kernelMatrix, colorSpace));
  }

  FEConvolveMatrix::FEConvolveMatrix(const IntSize &kernelSize, float divisor, float bias,
                                     const IntPoint &targetOffset, EdgeModeType edgeMode,
                                     const FloatPoint &kernelUnitLength, bool preserveAlpha,
                                     const List<float> &kernelMatrix,
                                     const DestinationColorSpace &colorSpace) noexcept
      : FilterEffect(FilterEffect::Type::FEConvolveMatrix, colorSpace), _kernelSize(kernelSize),
        _divisor(divisor), _bias(bias), _targetOffset(targetOffset), _edgeMode(edgeMode),
        _kernelUnitLength(kernelUnitLength), _preserveAlpha(preserveAlpha), _kernelMatrix(kernelMatrix)
  {
    krys_debug_assert(_kernelSize.Width() > 0);
    krys_debug_assert(_kernelSize.Height() > 0);
    krys_debug_assert(IntRect(IntPoint(), kernelSize).Contains(targetOffset));
  }

  bool FEConvolveMatrix::operator==(const FEConvolveMatrix &other) const noexcept
  {
    return FilterEffect::operator==(other) && _kernelSize == other._kernelSize && _divisor == other._divisor
           && _bias == other._bias && _targetOffset == other._targetOffset && _edgeMode == other._edgeMode
           && _kernelUnitLength == other._kernelUnitLength && _preserveAlpha == other._preserveAlpha
           && _kernelMatrix == other._kernelMatrix;
  }

  void FEConvolveMatrix::SetKernelSize(const IntSize &kernelSize) noexcept
  {
    krys_debug_assert(kernelSize.Width() > 0);
    krys_debug_assert(kernelSize.Height() > 0);
    _kernelSize = kernelSize;
  }

  void FEConvolveMatrix::SetKernel(const List<float> &kernel) noexcept
  {
    _kernelMatrix = kernel;
  }

  bool FEConvolveMatrix::SetDivisor(float divisor) noexcept
  {
    krys_debug_assert(divisor);
    if (_divisor == divisor)
    {
      return false;
    }
    _divisor = divisor;
    return true;
  }

  bool FEConvolveMatrix::SetBias(float bias) noexcept
  {
    if (_bias == bias)
    {
      return false;
    }
    _bias = bias;
    return true;
  }

  bool FEConvolveMatrix::SetTargetOffset(const IntPoint &targetOffset) noexcept
  {
    if (_targetOffset == targetOffset)
    {
      return false;
    }
    _targetOffset = targetOffset;
    return true;
  }

  bool FEConvolveMatrix::SetEdgeMode(EdgeModeType edgeMode) noexcept
  {
    if (_edgeMode == edgeMode)
    {
      return false;
    }
    _edgeMode = edgeMode;
    return true;
  }

  bool FEConvolveMatrix::SetKernelUnitLength(const FloatPoint &kernelUnitLength) noexcept
  {
    krys_debug_assert(kernelUnitLength.X() > 0);
    krys_debug_assert(kernelUnitLength.Y() > 0);
    if (_kernelUnitLength == kernelUnitLength)
    {
      return false;
    }
    _kernelUnitLength = kernelUnitLength;
    return true;
  }

  bool FEConvolveMatrix::SetPreserveAlpha(bool preserveAlpha) noexcept
  {
    if (_preserveAlpha == preserveAlpha)
    {
      return false;
    }
    _preserveAlpha = preserveAlpha;
    return true;
  }

  FloatRect FEConvolveMatrix::CalculateImageRect(const Filter &filter, Span<const FloatRect>,
                                                 const FloatRect &primitiveSubregion) const noexcept
  {
    return filter.MaxEffectRect(primitiveSubregion);
  }

  TextStream &operator<<(TextStream &ts, const EdgeModeType &type) noexcept
  {
    switch (type)
    {
      case EdgeModeType::Unknown:   ts << u8"UNKNOWN"; break;
      case EdgeModeType::Duplicate: ts << u8"DUPLICATE"; break;
      case EdgeModeType::Wrap:      ts << u8"WRAP"; break;
      case EdgeModeType::None:      ts << u8"NONE"; break;
    }
    return ts;
  }

  TextStream &FEConvolveMatrix::ExternalRepresentation(TextStream &ts,
                                                       FilterRepresentation representation) const noexcept
  {
    ts << Indent << u8"[feConvolveMatrix";
    FilterEffect::ExternalRepresentation(ts, representation);

    ts << u8" order=\"" << _kernelSize << u8'"';
    ts << u8" kernelMatrix=\"" << _kernelMatrix << u8'"';
    ts << u8" divisor=\"" << _divisor << u8'"';
    ts << u8" bias=\"" << _bias << u8'"';
    ts << u8" target=\"" << _targetOffset << u8'"';
    ts << u8" edgeMode=\"" << _edgeMode << u8'"';
    ts << u8" kernelUnitLength=\"" << _kernelUnitLength << u8'"';
    ts << u8" preserveAlpha=\"" << _preserveAlpha << u8'"';

    ts << u8"]\n";
    return ts;
  }
}
