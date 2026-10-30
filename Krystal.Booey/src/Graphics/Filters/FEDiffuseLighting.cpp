#include "Krystal.Booey/Graphics/Filters/FEDiffuseLighting.hpp"
#include "Krystal.Core/Text/TextStream.hpp"
#include <algorithm>

namespace krys::boo
{
  Ref<FEDiffuseLighting> FEDiffuseLighting::Create(const Color &lightingColor, float surfaceScale,
                                                   float diffuseConstant, float kernelUnitLengthX,
                                                   float kernelUnitLengthY, Ref<LightSource> &&lightSource,
                                                   const DestinationColorSpace &colorSpace) noexcept
  {
    return AdoptRef(*new FEDiffuseLighting(lightingColor, surfaceScale, diffuseConstant, kernelUnitLengthX,
                                           kernelUnitLengthY, krys::move(lightSource), colorSpace));
  }

  FEDiffuseLighting::FEDiffuseLighting(const Color &lightingColor, float surfaceScale, float diffuseConstant,
                                       float kernelUnitLengthX, float kernelUnitLengthY,
                                       Ref<LightSource> &&lightSource,
                                       const DestinationColorSpace &colorSpace) noexcept
      : FELighting(FilterEffect::Type::FEDiffuseLighting, lightingColor, surfaceScale, diffuseConstant, 0, 0,
                   kernelUnitLengthX, kernelUnitLengthY, krys::move(lightSource), colorSpace)
  {
  }

  bool FEDiffuseLighting::SetDiffuseConstant(float diffuseConstant) noexcept
  {
    diffuseConstant = std::max(diffuseConstant, 0.0f);
    if (_diffuseConstant == diffuseConstant)
    {
      return false;
    }
    _diffuseConstant = diffuseConstant;
    return true;
  }

  TextStream &FEDiffuseLighting::ExternalRepresentation(TextStream &ts,
                                                        FilterRepresentation representation) const noexcept
  {
    ts << Indent << u8"[feDiffuseLighting";
    FilterEffect::ExternalRepresentation(ts, representation);

    ts << u8" surfaceScale=\"" << _surfaceScale << u8'"';
    ts << u8" diffuseConstant=\"" << _diffuseConstant << u8'"';
    ts << u8" kernelUnitLength=\"" << _kernelUnitLengthX << u8", " << _kernelUnitLengthY << u8'"';

    ts << u8"]\n";
    return ts;
  }
}
