#include "Krystal.Booey/CSS/Selectors/UserAgentParts.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("UserAgentParts", "[CSS][Selectors][UserAgentParts]")
  {
    CHECK(UserAgentParts::Cue().View() == u8"cue");
    CHECK(UserAgentParts::DetailsContent().View() == u8"details-content");
    CHECK(UserAgentParts::FileSelectorButton().View() == u8"file-selector-button");
    CHECK(UserAgentParts::Placeholder().View() == u8"placeholder");
    CHECK(UserAgentParts::Thumb().View() == u8"thumb");
    CHECK(UserAgentParts::Track().View() == u8"track");
  }

  TEST_CASE("UserAgentParts(atoms)", "[CSS][Selectors][UserAgentParts]")
  {
    CHECK(&UserAgentParts::Placeholder() == &UserAgentParts::Placeholder());
    CHECK(UserAgentParts::Placeholder() == CSSOMStringAtom(u8"placeholder"));
  }
}
