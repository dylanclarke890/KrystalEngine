#include "Krystal.Booey/Graphics/Filters/FEGaussianBlur.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Text/TextStream.hpp"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD float GaussianKernelFactor() noexcept
    {
      return 3 / 4.f * std::sqrt(2 * std::numbers::pi_v<float>);
    }

    KRYS_NODISCARD int ClampedToKernelSize(float value) noexcept
    {
      constexpr unsigned MaxKernelSize = 500;

      // Limit the kernel size to 500. A bigger radius won't make a big difference for the result image but
      // inflates the absolute paint rect too much. This is compatible with Firefox' behavior.
      unsigned size =
        std::max<unsigned>(2, static_cast<unsigned>(std::floor(value * GaussianKernelFactor() + 0.5f)));
      return ClampTo<int>(std::min(size, MaxKernelSize));
    }
  }

  Ref<FEGaussianBlur> FEGaussianBlur::Create(float x, float y, EdgeModeType edgeMode) noexcept
  {
    return AdoptRef(*new FEGaussianBlur(x, y, edgeMode));
  }

  FEGaussianBlur::FEGaussianBlur(float x, float y, EdgeModeType edgeMode) noexcept
      : FilterEffect(FilterEffect::Type::FEGaussianBlur), _stdX(x), _stdY(y), _edgeMode(edgeMode)
  {
  }

  bool FEGaussianBlur::operator==(const FEGaussianBlur &other) const noexcept
  {
    return FilterEffect::operator==(other) && _stdX == other._stdX && _stdY == other._stdY
           && _edgeMode == other._edgeMode;
  }

  bool FEGaussianBlur::SetStdDeviationX(float stdX) noexcept
  {
    if (_stdX == stdX)
    {
      return false;
    }
    _stdX = stdX;
    return true;
  }

  bool FEGaussianBlur::SetStdDeviationY(float stdY) noexcept
  {
    if (_stdY == stdY)
    {
      return false;
    }
    _stdY = stdY;
    return true;
  }

  bool FEGaussianBlur::SetEdgeMode(EdgeModeType edgeMode) noexcept
  {
    if (_edgeMode == edgeMode)
    {
      return false;
    }
    _edgeMode = edgeMode;
    return true;
  }

  IntSize FEGaussianBlur::CalculateUnscaledKernelSize(FloatSize stdDeviation) noexcept
  {
    krys_debug_assert(stdDeviation.Width() >= 0 && stdDeviation.Height() >= 0);
    IntSize kernelSize;

    if (stdDeviation.Width())
    {
      kernelSize.SetWidth(ClampedToKernelSize(stdDeviation.Width()));
    }

    if (stdDeviation.Height())
    {
      kernelSize.SetHeight(ClampedToKernelSize(stdDeviation.Height()));
    }

    return kernelSize;
  }

  IntSize FEGaussianBlur::CalculateKernelSize(const Filter &filter, FloatSize stdDeviation) noexcept
  {
    stdDeviation = filter.ResolvedSize(stdDeviation);
    return CalculateUnscaledKernelSize(filter.ScaledByFilterScale(stdDeviation));
  }

  IntSize FEGaussianBlur::CalculateOutsetSize(FloatSize stdDeviation) noexcept
  {
    auto kernelSize = CalculateUnscaledKernelSize(stdDeviation);

    // We take the half kernel size and multiply it with three, because we run box blur three times.
    return {3 * kernelSize.Width() / 2, 3 * kernelSize.Height() / 2};
  }

  FloatRect FEGaussianBlur::CalculateImageRect(const Filter &filter, Span<const FloatRect> inputImageRects,
                                               const FloatRect &primitiveSubregion) const noexcept
  {
    auto imageRect = inputImageRects[0];

    // Edge modes other than 'none' do not inflate the affected paint rect.
    if (_edgeMode != EdgeModeType::None)
    {
      return FloatRect(EnclosingIntRect(imageRect));
    }

    auto kernelSize = CalculateUnscaledKernelSize(filter.ResolvedSize({_stdX, _stdY}));

    // We take the half kernel size and multiply it with three, because we run box blur three times.
    imageRect.InflateX(3 * static_cast<float>(kernelSize.Width()) * 0.5f);
    imageRect.InflateY(3 * static_cast<float>(kernelSize.Height()) * 0.5f);

    return filter.ClipToMaxEffectRect(imageRect, primitiveSubregion);
  }

  IntOutsets FEGaussianBlur::CalculateOutsets(const FloatSize &stdDeviation) noexcept
  {
    IntSize outsetSize = CalculateOutsetSize(stdDeviation);
    return {outsetSize.Height(), outsetSize.Width(), outsetSize.Height(), outsetSize.Width()};
  }

  bool FEGaussianBlur::ResultIsAlphaImage(Span<const Ref<FilterImage>> inputs) const noexcept
  {
    return inputs[0]->IsAlphaImage();
  }

  EnumSet<FilterRenderingMode> FEGaussianBlur::SupportedFilterRenderingModes(
    EnumSet<FilterRenderingMode> preferredFilterRenderingModes) const noexcept
  {
    // The accelerated backend blurs with any edge mode. The graphics context blurs by one standard deviation,
    // the same along both axes.
    EnumSet<FilterRenderingMode> modes = FilterRenderingMode::Accelerated;
    if (_stdX == _stdY && _edgeMode == EdgeModeType::None)
    {
      modes.Add(FilterRenderingMode::GraphicsContext);
    }
    return modes & preferredFilterRenderingModes;
  }

  Maybe<GraphicsStyle> FEGaussianBlur::CreateGraphicsStyle(GraphicsContext &,
                                                           const Filter &filter) const noexcept
  {
    // The blur radius, twice the standard deviation, as shadows have it.
    auto stdDeviation = filter.ResolvedSize({_stdX, _stdY});
    return GraphicsGaussianBlur {stdDeviation * 2.f};
  }

  TextStream &FEGaussianBlur::ExternalRepresentation(TextStream &ts,
                                                     FilterRepresentation representation) const noexcept
  {
    ts << Indent << u8"[feGaussianBlur";
    FilterEffect::ExternalRepresentation(ts, representation);

    ts << u8" stdDeviation=\"" << _stdX << u8", " << _stdY << u8'"';

    ts << u8"]\n";
    return ts;
  }
}
