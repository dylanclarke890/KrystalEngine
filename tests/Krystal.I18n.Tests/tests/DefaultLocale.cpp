#include "Krystal.I18n/DefaultLocale.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::i18n::tests
{
  TEST_CASE("DefaultLocale", "[I18n][DefaultLocale]")
  {
    auto previous = DefaultLocale();
    SetDefaultLocale(u8"fr_FR");
    CHECK(DefaultLocale() == u8"fr_FR");
    SetDefaultLocale(previous);
    CHECK(DefaultLocale() == previous);
  }
}
