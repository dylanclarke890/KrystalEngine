#include "Krystal.Booey/DOM/WheelEvent.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"

namespace krys::boo::dom::tests
{
  TEST_CASE("WheelEvent: from a dictionary", "[DOM][WheelEvent]")
  {
    Ref<WheelEvent> defaults = CreateRef<WheelEvent>(DOMStringAtom(u8"wheel"));
    CHECK(defaults->DeltaX() == 0);
    CHECK(defaults->DeltaY() == 0);
    CHECK(defaults->DeltaZ() == 0);
    CHECK(defaults->DeltaMode() == WheelEvent::DOM_DELTA_PIXEL);
    CHECK_FALSE(defaults->UnderlyingPlatformEvent());
    CHECK_FALSE(defaults->IsTrusted());
    CHECK(defaults->IsWheelEvent());
    CHECK(defaults->IsMouseEvent());

    WheelEventInit init;
    init.DeltaX = 1.5;
    init.DeltaY = -2;
    init.DeltaZ = 3;
    init.DeltaMode = WheelEvent::DOM_DELTA_LINE;
    init.ClientX = 10;
    Ref<WheelEvent> event = CreateRef<WheelEvent>(DOMStringAtom(u8"wheel"), init);
    CHECK(event->DeltaX() == 1.5);
    CHECK(event->DeltaY() == -2);
    CHECK(event->DeltaZ() == 3);
    CHECK(event->DeltaMode() == WheelEvent::DOM_DELTA_LINE);
    CHECK(event->ClientX() == 10);
  }

  TEST_CASE("WheelEvent: from the host's event", "[DOM][WheelEvent]")
  {
    // A notch towards the user: the content moves up, so the platform delta is negative and the DOM's
    // positive.
    PlatformWheelEvent platformEvent({10, 20}, {110, 120}, 0, -100, 0, -1,
                                     PlatformWheelEventGranularity::ScrollByPixelWheelEvent,
                                     {PlatformEvent::Modifier::ShiftKey}, MonotonicTime::Now());
    Ref<WheelEvent> event = WheelEvent::Create(platformEvent, nullptr);
    CHECK(event->Type() == EventNames::Wheel);
    CHECK(event->IsTrusted());
    CHECK(event->Bubbles());
    CHECK(event->Cancelable());
    CHECK(event->Composed());
    CHECK(event->DeltaX() == 0);
    CHECK(event->DeltaY() == 100);
    CHECK(event->DeltaMode() == WheelEvent::DOM_DELTA_PIXEL);
    CHECK(event->ScreenX() == 110);
    CHECK(event->ShiftKey());
    REQUIRE(event->UnderlyingPlatformEvent());
    CHECK(event->UnderlyingPlatformEvent()->WheelTicksY() == -1);

    SECTION("by page")
    {
      PlatformWheelEvent pageEvent({}, {}, 0, 1, 0, 1, PlatformWheelEventGranularity::ScrollByPageWheelEvent,
                                   {}, MonotonicTime::Now());
      Ref<WheelEvent> page = WheelEvent::Create(pageEvent, nullptr, WheelEvent::IsCancelable::No);
      CHECK(page->DeltaY() == -1);
      CHECK(page->DeltaMode() == WheelEvent::DOM_DELTA_PAGE);
      CHECK_FALSE(page->Cancelable());
    }
  }
}
