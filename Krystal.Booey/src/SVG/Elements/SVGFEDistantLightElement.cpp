#include "Krystal.Booey/SVG/Elements/SVGFEDistantLightElement.hpp"
#include "Krystal.Booey/Graphics/Filters/DistantLightSource.hpp"

namespace krys::boo
{
  SVGFEDistantLightElement::SVGFEDistantLightElement(Document &document) noexcept
      : SVGFELightElement(document, SVGElementInterface::FEDistantLight)
  {
  }

  Ref<LightSource> SVGFEDistantLightElement::GetLightSource() const noexcept
  {
    return DistantLightSource::Create(Azimuth(), Elevation());
  }
}
