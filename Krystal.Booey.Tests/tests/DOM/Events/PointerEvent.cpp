#include "Krystal.Booey/DOM/Events/PointerEvent.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Platform/Events/PlatformMouseEvent.hpp"
#include <numbers>

namespace krys::boo::dom::tests
{
  TEST_CASE("PointerEvent: from its dictionary", "[DOM][PointerEvent]")
  {
    SECTION("defaults")
    {
      auto eventRef = CreateRef<PointerEvent>(u8"pointerdown");
      auto &event = *eventRef;
      CHECK_FALSE(event.IsTrusted());
      CHECK(event.IsPointerEvent());
      CHECK(event.IsMouseEvent());
      CHECK(event.PointerId() == 0);
      CHECK(event.Width() == 1);
      CHECK(event.Height() == 1);
      CHECK(event.Pressure() == 0);
      CHECK(event.TiltX() == 0);
      CHECK(event.TiltY() == 0);
      CHECK(event.AltitudeAngle() == Catch::Approx(std::numbers::pi / 2));
      CHECK(event.AzimuthAngle() == 0);
      CHECK(event.PointerType().empty());
      CHECK_FALSE(event.GetIsPrimary());
      CHECK(event.GetCoalescedEvents().empty());
      CHECK(event.GetPredictedEvents().empty());
      CHECK(DynamicDowncast<PointerEvent>(static_cast<Event *>(&event)) == &event);
    }

    SECTION("the angles follow a tilt given alone")
    {
      PointerEventInit init;
      init.TiltX = 30;
      init.PointerType = u8"pen";
      init.PointerId = 7;
      init.IsPrimary = true;
      auto eventRef = CreateRef<PointerEvent>(u8"pointermove", init);
      auto &event = *eventRef;
      CHECK(event.TiltX() == 30);
      CHECK(event.TiltY() == 0);
      CHECK(event.AzimuthAngle() == 0);
      CHECK(event.AltitudeAngle() == Catch::Approx(std::numbers::pi / 3));
      CHECK(event.PointerType() == u8"pen");
      CHECK(event.PointerId() == 7);
      CHECK(event.GetIsPrimary());
    }

    SECTION("the tilt follows angles given alone")
    {
      PointerEventInit init;
      init.AltitudeAngle = std::numbers::pi / 4;
      init.AzimuthAngle = std::numbers::pi / 2;
      auto eventRef = CreateRef<PointerEvent>(u8"pointermove", init);
      auto &event = *eventRef;
      CHECK(event.TiltX() == 0);
      CHECK(event.TiltY() == 45);
      CHECK(event.AltitudeAngle() == Catch::Approx(std::numbers::pi / 4));
      CHECK(event.AzimuthAngle() == Catch::Approx(std::numbers::pi / 2));
    }

    SECTION("both pairs given are kept")
    {
      PointerEventInit init;
      init.TiltX = 20;
      init.TiltY = 10;
      init.AltitudeAngle = 1;
      init.AzimuthAngle = 2;
      auto eventRef = CreateRef<PointerEvent>(u8"pointermove", init);
      CHECK(eventRef->TiltX() == 20);
      CHECK(eventRef->TiltY() == 10);
      CHECK(eventRef->AltitudeAngle() == 1);
      CHECK(eventRef->AzimuthAngle() == 2);
    }

    SECTION("a tilt on both axes given alone")
    {
      PointerEventInit init;
      init.TiltX = 45;
      init.TiltY = 45;
      auto eventRef = CreateRef<PointerEvent>(u8"pointermove", init);
      CHECK(eventRef->TiltX() == 45);
      CHECK(eventRef->TiltY() == 45);
      CHECK(eventRef->AzimuthAngle() == Catch::Approx(std::numbers::pi / 4));
    }

    SECTION("legacy elements are null")
    {
      PointerEventInit init;
      auto eventRef = CreateRef<PointerEvent>(u8"pointerover", init);
      auto &event = *eventRef;
      CHECK_FALSE(event.ToElement());
      CHECK_FALSE(event.FromElement());
    }
  }

  TEST_CASE("PointerEvent: from a mouse event", "[DOM][PointerEvent]")
  {
    boo::tests::PaintFixture fixture {u8"<!DOCTYPE html><style>body{margin:0}</style><div id=box "
                                      u8"style=width:50px;height:50px></div>"};
    (void)fixture.Document->UpdateLayout();
    auto *view = fixture.Frame->View();

    auto mouseEvent = [&](utf8_stringview type, MouseButton button, uint16 buttons)
    {
      PlatformMouseEvent platform(PointD(10.5, 20.25), PointD(110.5, 220.25), button,
                                  PlatformEvent::Type::MouseMoved, 1, {}, MonotonicTime::Now(), buttons);
      return MouseEvent::Create(DOMStringAtom(type), view, platform, {}, {}, 1, nullptr);
    };

    SECTION("the types map")
    {
      CHECK(PointerEvent::TypeFromMouseEventType(u8"mousedown") == u8"pointerdown");
      CHECK(PointerEvent::TypeFromMouseEventType(u8"mouseup") == u8"pointerup");
      CHECK(PointerEvent::TypeFromMouseEventType(u8"mousemove") == u8"pointermove");
      CHECK(PointerEvent::TypeFromMouseEventType(u8"mouseover") == u8"pointerover");
      CHECK(PointerEvent::TypeFromMouseEventType(u8"mouseout") == u8"pointerout");
      CHECK(PointerEvent::TypeFromMouseEventType(u8"mouseenter") == u8"pointerenter");
      CHECK(PointerEvent::TypeFromMouseEventType(u8"mouseleave") == u8"pointerleave");
      CHECK(PointerEvent::TypeFromMouseEventType(u8"click") == u8"click");
      CHECK(PointerEvent::TypeFromMouseEventType(u8"auxclick") == u8"auxclick");
      CHECK(PointerEvent::TypeFromMouseEventType(u8"contextmenu") == u8"contextmenu");
      CHECK(PointerEvent::TypeFromMouseEventType(u8"dblclick") == null);
      CHECK_FALSE(PointerEvent::Create(MouseButton::Left, *mouseEvent(u8"dblclick", MouseButton::Left, 0),
                                       MousePointerId, MousePointerEventType()));
    }

    SECTION("a press")
    {
      auto source = mouseEvent(u8"mousedown", MouseButton::Left, 1);
      auto event = PointerEvent::Create(MouseButton::Left, *source, MousePointerId, MousePointerEventType());
      REQUIRE(event);
      CHECK(event->Type() == u8"pointerdown");
      CHECK(event->IsTrusted());
      CHECK(event->Bubbles());
      CHECK(event->Cancelable());
      CHECK(event->Composed());
      CHECK(event->PointerId() == MousePointerId);
      CHECK(event->PointerType() == u8"mouse");
      CHECK(event->GetIsPrimary());
      CHECK(event->Button() == 0);
      CHECK(event->Buttons() == 1);
      CHECK(event->TimeStamp() == source->TimeStamp());
      // A mouse has no pressure: half while a button is held.
      CHECK(event->Pressure() == 0.5f);
      // Pointer events keep fractional coordinates.
      CHECK(event->ClientX() == 10.5);
      CHECK(event->ClientY() == 20.25);
      CHECK(event->ScreenX() == 110.5);
    }

    SECTION("enter and leave don't bubble, and a move without buttons has no pressure")
    {
      auto enter = PointerEvent::Create(MouseButton::PointerHasNotChanged,
                                        *mouseEvent(u8"mouseenter", MouseButton::None, 0), MousePointerId,
                                        MousePointerEventType());
      REQUIRE(enter);
      CHECK_FALSE(enter->Bubbles());
      CHECK_FALSE(enter->Cancelable());
      CHECK_FALSE(enter->Composed());
      CHECK(enter->Pressure() == 0);
      // No button changed.
      CHECK(enter->Button() == -1);
    }

    SECTION("clicks have whole coordinates")
    {
      auto click = PointerEvent::Create(MouseButton::Left, *mouseEvent(u8"click", MouseButton::Left, 0),
                                        MousePointerId, MousePointerEventType());
      REQUIRE(click);
      CHECK(click->ClientX() == 10);
      CHECK(click->ClientY() == 20);
      CHECK(click->PageX() == 10);
    }

    SECTION("a pen's pressure is its force")
    {
      PlatformMouseEvent platform(PointD(1, 1), PointD(1, 1), MouseButton::Left,
                                  PlatformEvent::Type::MousePressed, 1, {}, MonotonicTime::Now(), 1, {},
                                  0.75);
      auto source = MouseEvent::Create(u8"mousedown", view, platform, {}, {}, 1, nullptr);
      auto event = PointerEvent::Create(MouseButton::Left, *source, 2, PenPointerEventType());
      REQUIRE(event);
      CHECK(event->Pressure() == 0.75f);
      CHECK(event->PointerType() == u8"pen");
    }

    SECTION("a move's coalesced events")
    {
      PlatformMouseEvent platform(PointD(5, 5), PointD(5, 5), MouseButton::None,
                                  PlatformEvent::Type::MouseMoved, 0, {}, MonotonicTime::Now());
      auto coalesced = MouseEvent::Create(u8"mousemove", view, platform, {}, {}, 0, nullptr);
      auto source = MouseEvent::Create(u8"mousemove", view, platform, {coalesced}, {}, 0, nullptr);
      auto event = PointerEvent::Create(MouseButton::PointerHasNotChanged, *source, MousePointerId,
                                        MousePointerEventType());
      REQUIRE(event);
      auto coalescedEvents = event->GetCoalescedEvents();
      REQUIRE(coalescedEvents.size() == 1);
      CHECK(coalescedEvents[0]->Type() == u8"pointermove");
      CHECK_FALSE(coalescedEvents[0]->Bubbles());
      CHECK_FALSE(coalescedEvents[0]->Cancelable());

      // They take the event's target as it's dispatched.
      auto &box = fixture.ById(u8"box");
      (void)EventDispatcher::DispatchToTarget(*event, &box);
      CHECK(coalescedEvents[0]->Target().get() == &box);
    }

    SECTION("an event for a pointer's capture")
    {
      auto event = PointerEvent::CreateForPointerCapture(u8"gotpointercapture", fixture.Document->Window(), 3,
                                                         true, TouchPointerEventType());
      CHECK(event->IsTrusted());
      CHECK(event->Bubbles());
      CHECK(event->Composed());
      CHECK_FALSE(event->Cancelable());
      CHECK(event->PointerId() == 3);
      CHECK(event->PointerType() == u8"touch");
      CHECK(event->GetIsPrimary());
    }

    SECTION("an event with no position")
    {
      auto cancel = PointerEvent::Create(u8"pointercancel", fixture.Document->Window(), 4,
                                         TouchPointerEventType(), PointerEvent::IsPrimary::Yes);
      CHECK(cancel->IsTrusted());
      CHECK(cancel->Bubbles());
      CHECK(cancel->Buttons() == 0);
      CHECK(cancel->Pressure() == 0);
      CHECK(cancel->GetIsPrimary());
      auto leave =
        PointerEvent::Create(u8"pointerleave", fixture.Document->Window(), 4, TouchPointerEventType());
      CHECK_FALSE(leave->Bubbles());
      CHECK_FALSE(leave->GetIsPrimary());
    }
  }
}
