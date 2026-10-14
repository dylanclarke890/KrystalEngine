#include "Krystal.Booey/Graphics/FontCustomPlatformData.hpp"
#include "Krystal.Booey/CSS/Values/FontFaceSrcValue.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo
{
  bool FontCustomPlatformData::SupportsFormat(css::CSSOMStringView format) noexcept
  {
    for (auto supported : {u8"truetype", u8"opentype", u8"woff2", u8"woff2-variations", u8"woff-variations",
                           u8"truetype-variations", u8"opentype-variations", u8"woff", u8"svg"})
    {
      if (text::IsASCIICaselessEqual(format, utf8_stringview(supported)))
      {
        return true;
      }
    }

    return false;
  }

  bool FontCustomPlatformData::SupportsTechnology(css::FontTechnology technology) noexcept
  {
    using enum css::FontTechnology;

    switch (technology)
    {
      case ColorCbdt:
      case ColorColrv0:
      case ColorColrv1:
      case ColorSbix:
      case ColorSvg:
      case FeaturesAat:
      case FeaturesOpentype:
      case Palettes:
      case Variations:       return true;
      case FeaturesGraphite:
      case Incremental:
      case Invalid:          return false;
    }

    return false;
  }
}
