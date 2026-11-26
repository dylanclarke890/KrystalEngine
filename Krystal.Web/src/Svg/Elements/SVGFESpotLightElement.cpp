#include "Krystal.Web/Svg/Elements/SVGFESpotLightElement.hpp"
#include "Krystal.Web/Graphics/Filters/SpotLightSource.hpp"

namespace krys::web
{
  SVGFESpotLightElement::SVGFESpotLightElement(Document &document) noexcept
      : SVGFELightElement(document, SVGElementInterface::FESpotLight)
  {
  }

  Ref<LightSource> SVGFESpotLightElement::GetLightSource() const noexcept
  {
    return SpotLightSource::Create({X(), Y(), Z()}, {PointsAtX(), PointsAtY(), PointsAtZ()},
                                   SpecularExponent(), LimitingConeAngle());
  }
}
