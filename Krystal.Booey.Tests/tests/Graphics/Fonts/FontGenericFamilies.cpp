#include "Krystal.Booey/Graphics/Fonts/FontGenericFamilies.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include "Krystal.I18n/Language.hpp"
#include "Krystal.Text/Script.hpp"

namespace krys::boo::tests
{
  namespace
  {
    // Sets the user's preferred languages for a test, and clears them after it.
    struct GenericFamiliesLanguagesScope
    {
      explicit GenericFamiliesLanguagesScope(List<utf8_string> languages) noexcept
      {
        i18n::OverrideUserPreferredLanguages(krys::move(languages));
      }

      ~GenericFamiliesLanguagesScope() noexcept
      {
        i18n::OverrideUserPreferredLanguages({});
      }
    };
  }

  TEST_CASE("FontGenericFamilies: Han text follows the preferred Chinese",
            "[Graphics][Fonts][FontGenericFamilies]")
  {
    FontGenericFamilies families;
    (void)families.SetStandardFontFamily(u8"Simplified", text::Script::SimplifiedHan);
    (void)families.SetStandardFontFamily(u8"Traditional", text::Script::TraditionalHan);
    {
      GenericFamiliesLanguagesScope scope({u8"zh-CN"});
      CHECK(families.StandardFontFamily(text::Script::Han) == u8"Simplified");
    }
    GenericFamiliesLanguagesScope scope({u8"zh-TW"});
    CHECK(families.StandardFontFamily(text::Script::Han) == u8"Traditional");
  }
}
