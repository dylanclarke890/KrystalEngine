#include "Krystal.Booey/Svg/Elements/SVGFEPointLightElement.hpp"
#include "Krystal.Booey/Graphics/Filters/PointLightSource.hpp"

namespace krys::boo
{
  SVGFEPointLightElement::SVGFEPointLightElement(Document &document) noexcept
      : SVGFELightElement(document, SVGElementInterface::FEPointLight)
  {
  }

  Ref<LightSource> SVGFEPointLightElement::GetLightSource() const noexcept
  {
    return PointLightSource::Create({X(), Y(), Z()});
  }
}
