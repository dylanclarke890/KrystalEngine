#include "Krystal.I18n/Language.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Move.hpp"
#include "Krystal.I18n/Locale.hpp"

namespace krys::i18n::tests
{
  namespace
  {
    // Sets the user's preferred languages for a test, and clears them after it.
    struct PreferredLanguagesScope
    {
      explicit PreferredLanguagesScope(List<utf8_string> languages) noexcept
      {
        OverrideUserPreferredLanguages(krys::move(languages));
      }

      ~PreferredLanguagesScope() noexcept
      {
        OverrideUserPreferredLanguages({});
      }
    };
  }

  TEST_CASE("Language: the user's preferred languages", "[I18n][Language]")
  {
    // Without the host's, the system's language is the only one.
    CHECK(UserPreferredLanguages() == List<utf8_string> {DefaultLanguage()});

    auto generation = UserPreferredLanguagesGeneration();
    PreferredLanguagesScope scope({u8"en-GB", u8"fr"});
    CHECK(UserPreferredLanguages() == List<utf8_string> {u8"en-GB", u8"fr"});
    CHECK(UserPreferredLanguagesGeneration() != generation);
  }

  TEST_CASE("Language: Simplified or Traditional Chinese", "[I18n][Language]")
  {
    SECTION("Simplified without a signal")
    {
      PreferredLanguagesScope scope({u8"en-GB"});
      CHECK(UserPrefersSimplifiedChinese());
    }

    SECTION("Traditional when zh-TW comes first")
    {
      PreferredLanguagesScope scope({u8"en-GB", u8"ZH-tw", u8"zh-CN"});
      CHECK_FALSE(UserPrefersSimplifiedChinese());
    }

    SECTION("Simplified when zh-CN comes first")
    {
      PreferredLanguagesScope scope({u8"zh-CN", u8"zh-TW"});
      CHECK(UserPrefersSimplifiedChinese());
    }
  }
}
