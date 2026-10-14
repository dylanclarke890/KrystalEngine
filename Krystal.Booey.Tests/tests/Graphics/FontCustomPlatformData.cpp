#include "Krystal.Booey/Graphics/FontCustomPlatformData.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Values/FontFaceSrcValue.hpp"

namespace krys::boo::tests
{
  TEST_CASE("FontCustomPlatformData::SupportsFormat", "[Graphics][FontCustomPlatformData]")
  {
    for (auto format : {u8"truetype", u8"OpenType", u8"woff", u8"woff2", u8"woff-variations",
                        u8"woff2-variations", u8"truetype-variations", u8"opentype-variations", u8"svg"})
    {
      CHECK(FontCustomPlatformData::SupportsFormat(format));
    }

    CHECK_FALSE(FontCustomPlatformData::SupportsFormat(u8"collection"));
    CHECK_FALSE(FontCustomPlatformData::SupportsFormat(u8"embedded-opentype"));
    CHECK_FALSE(FontCustomPlatformData::SupportsFormat(u8""));
  }

  TEST_CASE("FontCustomPlatformData::SupportsTechnology", "[Graphics][FontCustomPlatformData]")
  {
    using enum css::FontTechnology;

    for (auto technology : {ColorCbdt, ColorColrv0, ColorColrv1, ColorSbix, ColorSvg, FeaturesAat,
                            FeaturesOpentype, Palettes, Variations})
    {
      CHECK(FontCustomPlatformData::SupportsTechnology(technology));
    }

    CHECK_FALSE(FontCustomPlatformData::SupportsTechnology(FeaturesGraphite));
    CHECK_FALSE(FontCustomPlatformData::SupportsTechnology(Incremental));
    CHECK_FALSE(FontCustomPlatformData::SupportsTechnology(Invalid));
  }
}
