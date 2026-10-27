#include "Krystal.Booey/SVG/SVGFESpotLightElement.hpp"
#include "Krystal.Booey/Graphics/Filters/SpotLightSource.hpp"

namespace krys::boo::svg
{
  SVGFESpotLightElement::SVGFESpotLightElement(dom::Document &document) noexcept
      : SVGFELightElement(document, SVGElementInterface::FESpotLight)
  {
  }

  Ref<LightSource> SVGFESpotLightElement::GetLightSource() const noexcept
  {
    return SpotLightSource::Create({X(), Y(), Z()}, {PointsAtX(), PointsAtY(), PointsAtZ()},
                                   SpecularExponent(), LimitingConeAngle());
  }
}
