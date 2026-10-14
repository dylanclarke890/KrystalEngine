#include "Krystal.Booey/Graphics/FontPaletteValues.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::tests
{
  TEST_CASE("FontPaletteValues", "[Graphics][FontPaletteValues]")
  {
    SECTION("FontPaletteIndex")
    {
      CHECK_FALSE(static_cast<bool>(FontPaletteIndex {}));
      CHECK_FALSE(static_cast<bool>(FontPaletteIndex(0u)));
      CHECK(static_cast<bool>(FontPaletteIndex(2u)));
      CHECK(static_cast<bool>(FontPaletteIndex(FontPaletteIndex::Type::Light)));
      CHECK(FontPaletteIndex(FontPaletteIndex::Type::Dark) == FontPaletteIndex(FontPaletteIndex::Type::Dark));
      CHECK_FALSE(FontPaletteIndex(FontPaletteIndex::Type::Dark)
                  == FontPaletteIndex(FontPaletteIndex::Type::Light));
      CHECK(FontPaletteIndex(3u) == FontPaletteIndex(3u));
      CHECK_FALSE(FontPaletteIndex(3u) == FontPaletteIndex(4u));
      CHECK(DefaultHash<FontPaletteIndex>::Hash(FontPaletteIndex(3u))
            == DefaultHash<FontPaletteIndex>::Hash(FontPaletteIndex(3u)));
      CHECK(DefaultHash<FontPaletteIndex>::Hash(FontPaletteIndex(3u))
            != DefaultHash<FontPaletteIndex>::Hash(FontPaletteIndex(4u)));
    }

    SECTION("FontPaletteValues")
    {
      CHECK_FALSE(static_cast<bool>(FontPaletteValues {}));
      FontPaletteValues base(FontPaletteIndex(1u), {});
      CHECK(static_cast<bool>(base));
      CHECK(base.BasePalette() == FontPaletteIndex(1u));

      List<FontPaletteValues::OverriddenColor> colors;
      colors.push_back({0u, Color::black});
      FontPaletteValues overridden({}, krys::move(colors));
      CHECK(static_cast<bool>(overridden));
      CHECK_FALSE(overridden.BasePalette());
      CHECK(overridden.OverrideColors().size() == 1);

      CHECK_FALSE(base == overridden);
      CHECK(base == FontPaletteValues(FontPaletteIndex(1u), {}));
      CHECK(DefaultHash<FontPaletteValues>::Hash(base)
            == DefaultHash<FontPaletteValues>::Hash(FontPaletteValues(FontPaletteIndex(1u), {})));
      CHECK(DefaultHash<FontPaletteValues>::Hash(base) != DefaultHash<FontPaletteValues>::Hash(overridden));
    }
  }
}
