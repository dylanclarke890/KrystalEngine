#include "Krystal.Booey/Graphics/Filters/FELighting.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Core/Utils/PointerComparison.hpp"
#include <algorithm>

namespace krys::boo
{
  FELighting::FELighting(Type type, const Color &lightingColor, float surfaceScale, float diffuseConstant,
                         float specularConstant, float specularExponent, float kernelUnitLengthX,
                         float kernelUnitLengthY, Ref<LightSource> &&lightSource,
                         const DestinationColorSpace &colorSpace) noexcept
      : FilterEffect(type, colorSpace), _lightingColor(lightingColor), _surfaceScale(surfaceScale),
        _diffuseConstant(std::max(diffuseConstant, 0.0f)),
        _specularConstant(std::max(specularConstant, 0.0f)),
        _specularExponent(std::clamp(specularExponent, 1.0f, 128.0f)), _kernelUnitLengthX(kernelUnitLengthX),
        _kernelUnitLengthY(kernelUnitLengthY), _lightSource(krys::move(lightSource))
  {
  }

  bool FELighting::operator==(const FELighting &other) const noexcept
  {
    return FilterEffect::operator==(other) && _lightingColor == other._lightingColor
           && _surfaceScale == other._surfaceScale && _diffuseConstant == other._diffuseConstant
           && _specularConstant == other._specularConstant && _specularExponent == other._specularExponent
           && _kernelUnitLengthX == other._kernelUnitLengthX && _kernelUnitLengthY == other._kernelUnitLengthY
           && ArePointingToEqualData(_lightSource, other._lightSource);
  }

  bool FELighting::SetSurfaceScale(float surfaceScale) noexcept
  {
    if (_surfaceScale == surfaceScale)
    {
      return false;
    }
    _surfaceScale = surfaceScale;
    return true;
  }

  bool FELighting::SetLightingColor(const Color &lightingColor) noexcept
  {
    if (_lightingColor == lightingColor)
    {
      return false;
    }
    _lightingColor = lightingColor;
    return true;
  }

  bool FELighting::SetKernelUnitLengthX(float kernelUnitLengthX) noexcept
  {
    if (_kernelUnitLengthX == kernelUnitLengthX)
    {
      return false;
    }
    _kernelUnitLengthX = kernelUnitLengthX;
    return true;
  }

  bool FELighting::SetKernelUnitLengthY(float kernelUnitLengthY) noexcept
  {
    if (_kernelUnitLengthY == kernelUnitLengthY)
    {
      return false;
    }
    _kernelUnitLengthY = kernelUnitLengthY;
    return true;
  }

  FloatRect FELighting::CalculateImageRect(const Filter &filter, Span<const FloatRect>,
                                           const FloatRect &primitiveSubregion) const noexcept
  {
    return filter.MaxEffectRect(primitiveSubregion);
  }
}
