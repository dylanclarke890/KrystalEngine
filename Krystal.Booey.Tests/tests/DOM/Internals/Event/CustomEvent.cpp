#include "Krystal.Booey/DOM/CustomEvent.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/EventTarget.hpp"
#include <catch_all.hpp>

namespace krys::tests
{
  using namespace krys::boo;

  TEST_CASE("CustomEvent::Detail", "[HTML][Event]")
  {
    CustomEventInit<uint32> init {.Detail = 4u};

    auto event = CreateRef<CustomEvent<uint32>>(EventNames::Click, init);
    REQUIRE_FALSE(event->IsTrusted());
    REQUIRE(event->Detail() == 4u);
  }

  TEST_CASE("CustomEvent::InitCustomEvent", "[HTML][Event]")
  {
    auto event = CreateRef<CustomEvent<uint32>>(EventNames::Click, CustomEventInit<uint32> {});
    event->InitCustomEvent(EventNames::MouseWheel, true, true, 6u);
    REQUIRE_FALSE(event->IsTrusted());

    REQUIRE(event->Type() == EventNames::MouseWheel);
    REQUIRE(event->Bubbles() == true);
    REQUIRE(event->Cancelable() == true);
    REQUIRE(event->Detail() == 6u);
  }
}