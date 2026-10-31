#include "Krystal.Booey/DOM/CustomEvent.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/EventTarget.hpp"

namespace krys::boo::dom::tests
{
  TEST_CASE("CustomEvent.detail: the value it was initialized to, null by default",
            "[HTML][Event][dom:dom-customevent-detail]")
  {
    auto event = CreateRef<CustomEvent<uint32>>(EventNames::Click, CustomEventInit<uint32> {.Detail = 4u});
    REQUIRE_FALSE(event->IsTrusted());
    REQUIRE(event->Detail() == 4u);

    auto withoutDetail = CreateRef<CustomEvent<uint32>>(EventNames::Click);
    REQUIRE_FALSE(withoutDetail->Detail().has_value());
  }

  TEST_CASE(
    "CustomEvent.initCustomEvent(): initializes the event and sets detail, unless it's being dispatched",
    "[HTML][Event][dom:dom-customevent-initcustomevent]")
  {
    SECTION("Sets the type, bubbles, cancelable and detail")
    {
      auto event = CreateRef<CustomEvent<uint32>>(EventNames::Click, CustomEventInit<uint32> {});
      event->InitCustomEvent(EventNames::MouseWheel, true, true, 6u);
      REQUIRE_FALSE(event->IsTrusted());

      REQUIRE(event->Type() == EventNames::MouseWheel);
      REQUIRE(event->Bubbles() == true);
      REQUIRE(event->Cancelable() == true);
      REQUIRE(event->Detail() == 6u);
    }

    SECTION("Clears the canceled flag, and detail when it's omitted")
    {
      auto event =
        CreateRef<CustomEvent<uint32>>(EventNames::Click, CustomEventInit<uint32> {{.Cancelable = true}, 4u});
      event->PreventDefault();
      REQUIRE(event->DefaultPrevented());

      event->InitCustomEvent(EventNames::Click, false, true);
      REQUIRE_FALSE(event->DefaultPrevented());
      REQUIRE_FALSE(event->Detail().has_value());
    }

    SECTION("Does nothing while the event is being dispatched")
    {
      auto document = CreateRef<Document>();
      auto element = document->CreateElement(u8"div").ReleaseValue();
      auto event = CreateRef<CustomEvent<uint32>>(EventNames::Click, CustomEventInit<uint32> {.Detail = 1u});
      auto listener = element->AddEventListener(
        EventNames::Click, [&](Event &) { event->InitCustomEvent(EventNames::MouseWheel, true, true, 2u); });
      REQUIRE_FALSE(element->DispatchEvent(*event).HasException());

      REQUIRE(event->Type() == EventNames::Click);
      REQUIRE_FALSE(event->Bubbles());
      REQUIRE(event->Detail() == 1u);
    }
  }
}
