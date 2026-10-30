#include "Krystal.Booey/SVG/SVGFEDistantLightElement.hpp"
#include "Krystal.Booey/Graphics/Filters/DistantLightSource.hpp"

namespace krys::boo::svg
{
  SVGFEDistantLightElement::SVGFEDistantLightElement(dom::Document &document) noexcept
      : SVGFELightElement(document, SVGElementInterface::FEDistantLight)
  {
  }

  Ref<LightSource> SVGFEDistantLightElement::GetLightSource() const noexcept
  {
    return DistantLightSource::Create(Azimuth(), Elevation());
  }
}
