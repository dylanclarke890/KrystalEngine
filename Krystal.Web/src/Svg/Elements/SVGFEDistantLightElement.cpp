#include "Krystal.Web/Svg/Elements/SVGFEDistantLightElement.hpp"
#include "Krystal.Web/Graphics/Filters/DistantLightSource.hpp"

namespace krys::web
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
