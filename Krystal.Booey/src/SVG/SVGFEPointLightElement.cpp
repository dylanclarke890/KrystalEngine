#include "Krystal.Booey/SVG/SVGFEPointLightElement.hpp"
#include "Krystal.Booey/Graphics/Filters/PointLightSource.hpp"

namespace krys::boo::svg
{
  SVGFEPointLightElement::SVGFEPointLightElement(dom::Document &document) noexcept
      : SVGFELightElement(document, SVGElementInterface::FEPointLight)
  {
  }

  Ref<LightSource> SVGFEPointLightElement::GetLightSource() const noexcept
  {
    return PointLightSource::Create({X(), Y(), Z()});
  }
}
