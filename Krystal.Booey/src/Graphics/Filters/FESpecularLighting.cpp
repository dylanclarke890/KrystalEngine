#include "Krystal.Booey/Graphics/Filters/FESpecularLighting.hpp"
#include "Krystal.Core/Text/TextStream.hpp"
#include <algorithm>

namespace krys::boo
{
  Ref<FESpecularLighting> FESpecularLighting::Create(const Color &lightingColor, float surfaceScale,
                                                     float specularConstant, float specularExponent,
                                                     float kernelUnitLengthX, float kernelUnitLengthY,
                                                     Ref<LightSource> &&lightSource,
                                                     const DestinationColorSpace &colorSpace) noexcept
  {
    return AdoptRef(*new FESpecularLighting(lightingColor, surfaceScale, specularConstant, specularExponent,
                                            kernelUnitLengthX, kernelUnitLengthY, krys::move(lightSource),
                                            colorSpace));
  }

  FESpecularLighting::FESpecularLighting(const Color &lightingColor, float surfaceScale,
                                         float specularConstant, float specularExponent,
                                         float kernelUnitLengthX, float kernelUnitLengthY,
                                         Ref<LightSource> &&lightSource,
                                         const DestinationColorSpace &colorSpace) noexcept
      : FELighting(FilterEffect::Type::FESpecularLighting, lightingColor, surfaceScale, 0, specularConstant,
                   specularExponent, kernelUnitLengthX, kernelUnitLengthY, krys::move(lightSource),
                   colorSpace)
  {
  }

  bool FESpecularLighting::SetSpecularConstant(float specularConstant) noexcept
  {
    specularConstant = std::max(specularConstant, 0.0f);
    if (_specularConstant == specularConstant)
    {
      return false;
    }
    _specularConstant = specularConstant;
    return true;
  }

  bool FESpecularLighting::SetSpecularExponent(float specularExponent) noexcept
  {
    specularExponent = std::clamp(specularExponent, 1.0f, 128.0f);
    if (_specularExponent == specularExponent)
    {
      return false;
    }
    _specularExponent = specularExponent;
    return true;
  }

  TextStream &FESpecularLighting::ExternalRepresentation(TextStream &ts,
                                                         FilterRepresentation representation) const noexcept
  {
    ts << Indent << u8"[feSpecularLighting";
    FilterEffect::ExternalRepresentation(ts, representation);

    ts << u8" surfaceScale=\"" << _surfaceScale << u8'"';
    ts << u8" specualConstant=\"" << _specularConstant << u8'"';
    ts << u8" specularExponent=\"" << _specularExponent << u8'"';

    ts << u8"]\n";
    return ts;
  }
}
