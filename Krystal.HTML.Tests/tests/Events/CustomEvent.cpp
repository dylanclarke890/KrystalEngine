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
    CustomEventInit<uint32> init {};
    init.Detail = 4u;
    auto event = CreateRef<CustomEvent<uint32>>(init, EventNames::Click);
    REQUIRE(event->Detail() == 4u);

    event->Detail(5u);
    REQUIRE(event->Detail() == 5u);
  }

  TEST_CASE("CustomEvent::InitCustomEvent", "[HTML][Event]")
  {
    auto event = CreateRef<CustomEvent<uint32>>(CustomEventInit<uint32> {}, EventNames::Click);
    event->InitCustomEvent(EventNames::MouseWheel, true, true, 6u);

    REQUIRE(event->Type() == EventNames::MouseWheel);
    REQUIRE(event->Bubbles() == true);
    REQUIRE(event->Cancellable() == true);
    REQUIRE(event->Detail() == 6u);
  }
}