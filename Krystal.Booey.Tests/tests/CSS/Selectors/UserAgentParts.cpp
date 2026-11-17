#include "Krystal.Booey/CSS/Selectors/UserAgentParts.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("UserAgentParts", "[CSS][Selectors][UserAgentParts]")
  {
    CHECK(user_agent_parts::Cue().View() == u8"cue");
    CHECK(user_agent_parts::DetailsContent().View() == u8"details-content");
    CHECK(user_agent_parts::FileSelectorButton().View() == u8"file-selector-button");
    CHECK(user_agent_parts::Placeholder().View() == u8"placeholder");
    CHECK(user_agent_parts::Thumb().View() == u8"thumb");
    CHECK(user_agent_parts::Track().View() == u8"track");
  }

  TEST_CASE("UserAgentParts(atoms)", "[CSS][Selectors][UserAgentParts]")
  {
    CHECK(&user_agent_parts::Placeholder() == &user_agent_parts::Placeholder());
    CHECK(user_agent_parts::Placeholder() == u8"placeholder");
  }
}
