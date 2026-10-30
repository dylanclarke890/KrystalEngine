#include "Krystal.Booey/Graphics/Filters/FEDropShadow.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"
#include "Krystal.Booey/Graphics/Filters/FEGaussianBlur.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Core/Text/TextStream.hpp"
#include <algorithm>

namespace krys::boo
{
  Ref<FEDropShadow> FEDropShadow::Create(float stdX, float stdY, float dx, float dy, const Color &shadowColor,
                                         float shadowOpacity) noexcept
  {
    return AdoptRef(*new FEDropShadow(stdX, stdY, dx, dy, shadowColor, shadowOpacity));
  }

  FEDropShadow::FEDropShadow(float stdX, float stdY, float dx, float dy, const Color &shadowColor,
                             float shadowOpacity) noexcept
      : FilterEffect(FilterEffect::Type::FEDropShadow), _stdX(stdX), _stdY(stdY), _dx(dx), _dy(dy),
        _shadowColor(shadowColor), _shadowOpacity(shadowOpacity)
  {
  }

  bool FEDropShadow::operator==(const FEDropShadow &other) const noexcept
  {
    return FilterEffect::operator==(other) && _stdX == other._stdX && _stdY == other._stdY && _dx == other._dx
           && _dy == other._dy && _shadowColor == other._shadowColor
           && _shadowOpacity == other._shadowOpacity;
  }

  bool FEDropShadow::SetStdDeviationX(float stdX) noexcept
  {
    if (_stdX == stdX)
    {
      return false;
    }
    _stdX = stdX;
    return true;
  }

  bool FEDropShadow::SetStdDeviationY(float stdY) noexcept
  {
    if (_stdY == stdY)
    {
      return false;
    }
    _stdY = stdY;
    return true;
  }

  bool FEDropShadow::SetDx(float dx) noexcept
  {
    if (_dx == dx)
    {
      return false;
    }
    _dx = dx;
    return true;
  }

  bool FEDropShadow::SetDy(float dy) noexcept
  {
    if (_dy == dy)
    {
      return false;
    }
    _dy = dy;
    return true;
  }

  bool FEDropShadow::SetShadowColor(const Color &shadowColor) noexcept
  {
    if (_shadowColor == shadowColor)
    {
      return false;
    }
    _shadowColor = shadowColor;
    return true;
  }

  bool FEDropShadow::SetShadowOpacity(float shadowOpacity) noexcept
  {
    if (_shadowOpacity == shadowOpacity)
    {
      return false;
    }
    _shadowOpacity = shadowOpacity;
    return true;
  }

  FloatRect FEDropShadow::CalculateImageRect(const Filter &filter, Span<const FloatRect> inputImageRects,
                                             const FloatRect &primitiveSubregion) const noexcept
  {
    auto imageRect = inputImageRects[0];
    auto imageRectWithOffset(imageRect);
    imageRectWithOffset.Move(filter.ResolvedSize({_dx, _dy}));
    imageRect.Unite(imageRectWithOffset);

    auto kernelSize = FEGaussianBlur::CalculateUnscaledKernelSize(filter.ResolvedSize({_stdX, _stdY}));

    // We take the half kernel size and multiply it with three, because we run box blur three times.
    imageRect.InflateX(3 * static_cast<float>(kernelSize.Width()) * 0.5f);
    imageRect.InflateY(3 * static_cast<float>(kernelSize.Height()) * 0.5f);

    return filter.ClipToMaxEffectRect(imageRect, primitiveSubregion);
  }

  IntOutsets FEDropShadow::CalculateOutsets(const FloatSize &offset, const FloatSize &stdDeviation) noexcept
  {
    IntSize outsetSize = FEGaussianBlur::CalculateOutsetSize(stdDeviation);

    int top = std::max<int>(0, static_cast<int>(outsetSize.Height() - offset.Height()));
    int right = std::max<int>(0, static_cast<int>(outsetSize.Width() + offset.Width()));
    int bottom = std::max<int>(0, static_cast<int>(outsetSize.Height() + offset.Height()));
    int left = std::max<int>(0, static_cast<int>(outsetSize.Width() - offset.Width()));

    return {top, right, bottom, left};
  }

  EnumSet<FilterRenderingMode> FEDropShadow::SupportedFilterRenderingModes(
    EnumSet<FilterRenderingMode> preferredFilterRenderingModes) const noexcept
  {
    // The accelerated backend draws any shadow. The graphics context blurs a shadow by one standard
    // deviation, the same along both axes.
    EnumSet<FilterRenderingMode> modes = FilterRenderingMode::Accelerated;
    if (_stdX == _stdY)
    {
      modes.Add(FilterRenderingMode::GraphicsContext);
    }
    return modes & preferredFilterRenderingModes;
  }

  Maybe<GraphicsStyle> FEDropShadow::CreateGraphicsStyle(GraphicsContext &,
                                                         const Filter &filter) const noexcept
  {
    krys_debug_assert(_stdX == _stdY);

    // The blur radius, twice the standard deviation, as shadows have it.
    auto offset = filter.ResolvedSize({_dx, _dy});
    auto stdDeviation = filter.ResolvedSize({_stdX, _stdY});

    return GraphicsDropShadow {offset, stdDeviation.Width() * 2.f, _shadowColor, ShadowRadiusMode::Default,
                               _shadowOpacity};
  }

  TextStream &FEDropShadow::ExternalRepresentation(TextStream &ts,
                                                   FilterRepresentation representation) const noexcept
  {
    ts << Indent << u8"[feDropShadow";
    FilterEffect::ExternalRepresentation(ts, representation);

    ts << u8" stdDeviation=\"" << _stdX << u8", " << _stdY << u8'"';
    ts << u8" dx=\"" << _dx << u8"\" dy=\"" << _dy << u8'"';
    ts << u8" flood-color=\"" << SerialisationForRenderTreeAsText(_shadowColor) << u8'"';
    ts << u8" flood-opacity=\"" << _shadowOpacity << u8'"';

    ts << u8"]\n";
    return ts;
  }
}
