#include "Krystal.HTML/Events/CustomEvent.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Events/EventNames.hpp"
#include "Krystal.HTML/Events/EventTarget.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  TEST_CASE("CustomEvent::Detail", "[HTML][Event]")
  {
    CustomEventInit<uint32> init {.Detail = 4u};

    auto event = CreateRef<CustomEvent<uint32>>(EventNames::Click, init);
    REQUIRE_FALSE(event->IsTrusted());
    REQUIRE(event->Detail() == 4u);

    event->Detail(5u);
    REQUIRE(event->Detail() == 5u);
  }

  TEST_CASE("CustomEvent::InitCustomEvent", "[HTML][Event]")
  {
    auto event = CreateRef<CustomEvent<uint32>>(EventNames::Click, CustomEventInit<uint32> {});
    event->InitCustomEvent(EventNames::MouseWheel, true, true, 6u);
    REQUIRE_FALSE(event->IsTrusted());

    REQUIRE(event->Type() == EventNames::MouseWheel);
    REQUIRE(event->Bubbles() == true);
    REQUIRE(event->Cancellable() == true);
    REQUIRE(event->Detail() == 6u);
  }
}