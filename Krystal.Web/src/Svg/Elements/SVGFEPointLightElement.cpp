#include "Krystal.Web/Svg/Elements/SVGFEPointLightElement.hpp"
#include "Krystal.Web/Graphics/Filters/PointLightSource.hpp"

namespace krys::web
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
