#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/PointerEvent.hpp"
#include "Krystal.Booey/DOM/TouchEvent.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/Navigator.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Page/PointerCaptureController.hpp"
#include "Krystal.Booey/Platform/PlatformTouchEvent.hpp"
#include <string>

namespace krys::boo::tests
{
  namespace
  {
    using Strings = List<std::string>;

    KRYS_NODISCARD std::string Name(const dom::EventTarget *target)
    {
      auto *node = DynamicDowncast<dom::Node>(target);
      auto *element = node ? DynamicDowncast<dom::Element>(*node) : nullptr;
      if (!element)
      {
        return node ? "document" : "null";
      }
      auto id = element->GetAttribute(u8"id").value_or(dom::DOMString());
      auto name = id.empty() ? element->LocalName().View() : dom::DOMStringView(id);
      return std::string(name.begin(), name.end());
    }

    /// @brief Records the events it handles as `type:target`, with the pointer id after a pointer event's
    /// type (`pointerdown3`) and the lengths of a touch event's lists after its type (`touchstart[1,1,1]`).
    struct Recorder : public dom::EventListenerCallback
    {
      List<std::string> &Records;

      explicit Recorder(List<std::string> &records) noexcept : Records(records)
      {
      }

      void HandleEvent(dom::Event &event) noexcept override
      {
        auto type = std::string(event.Type().View().begin(), event.Type().View().end());
        if (auto *pointerEvent = DynamicDowncast<dom::PointerEvent>(event))
        {
          type += std::to_string(pointerEvent->PointerId());
        }
        if (auto *touchEvent = DynamicDowncast<dom::TouchEvent>(event))
        {
          type += "[" + std::to_string(touchEvent->Touches()->Length()) + ","
                  + std::to_string(touchEvent->TargetTouches()->Length()) + ","
                  + std::to_string(touchEvent->ChangedTouches()->Length()) + "]";
        }
        Records.push_back(type + ":" + Name(event.Target().get()));
      }
    };

    /// @brief Calls @p action when it handles an event.
    template <typename TAction>
    struct ActionListener : public dom::EventListenerCallback
    {
      TAction Action;

      explicit ActionListener(TAction action) noexcept : Action(krys::move(action))
      {
      }

      void HandleEvent(dom::Event &event) noexcept override
      {
        Action(event);
      }
    };

    /// @brief Two 100x50 boxes, `a` above `b`, in a page, recording the pointer, touch and mouse events at
    /// its document.
    struct TouchFixture : PaintFixture
    {
      List<std::string> Records;

      TouchFixture() noexcept
          : PaintFixture(u8"<!DOCTYPE html><style>body{margin:0}div{width:100px;height:50px}</style>"
                         u8"<div id=a></div><div id=b></div>",
                         {200, 100}, true)
      {
        (void)Document->UpdateLayout();
        // Capturing at the document sees the events that don't bubble too.
        Ref<dom::EventListenerCallback> recorder = CreateRef<Recorder>(Records);
        for (auto type :
             {u8"pointerover", u8"pointerout", u8"pointerenter", u8"pointerleave", u8"pointerdown",
              u8"pointermove", u8"pointerup", u8"pointercancel", u8"gotpointercapture",
              u8"lostpointercapture", u8"touchstart", u8"touchmove", u8"touchend", u8"touchcancel",
              u8"mousedown", u8"mousemove", u8"mouseup", u8"click"})
        {
          Document->AddEventListener(dom::DOMStringAtom(type), Ref {recorder}, true);
        }
      }

      KRYS_NODISCARD EventHandler &Handler() noexcept
      {
        return Frame->GetEventHandler();
      }

      KRYS_NODISCARD static PlatformTouchPoint Point(uint32 id, PlatformTouchPoint::State state, double x,
                                                     double y) noexcept
      {
        return PlatformTouchPoint(id, state, DoublePoint(x, y), DoublePoint(x, y), DoubleSize(3, 2), 0,
                                  0.25f);
      }

      bool Touch(PlatformEvent::Type type, List<PlatformTouchPoint> points) noexcept
      {
        PlatformTouchEvent event(type, krys::move(points), {}, MonotonicTime::Now());
        if (type == PlatformEvent::Type::TouchMove)
        {
          event.SetCoalescedEvents({event});
        }
        return Handler().HandleTouchEvent(event);
      }

      /// @brief The records since the last call at the boxes and the document (not the body or the root).
      KRYS_NODISCARD Strings Take() noexcept
      {
        Strings records;
        for (auto &record : std::exchange(Records, {}))
        {
          if (record.ends_with(":a") || record.ends_with(":b") || record.ends_with(":document"))
          {
            records.push_back(record);
          }
        }
        return records;
      }

      template <typename TAction>
      void On(dom::EventTarget &target, utf8_stringview type, TAction action) noexcept
      {
        Ref<dom::EventListenerCallback> listener = CreateRef<ActionListener<TAction>>(krys::move(action));
        target.AddEventListener(dom::DOMStringAtom(type), krys::move(listener), false);
      }
    };

    using State = PlatformTouchPoint::State;
    using Type = PlatformEvent::Type;
  }

  TEST_CASE("Touch, TouchList and TouchEvent", "[DOM][TouchEvent]")
  {
    TouchFixture fixture;
    auto &a = fixture.ById(u8"a");

    auto touch = dom::Touch::Create(fixture.Frame.get(), &a, 4, {300, 400}, {10, 20}, {3, 2}, 15, 0.5f, 30);
    CHECK(touch->Target() == &a);
    CHECK(touch->Identifier() == 4);
    CHECK(touch->ScreenX() == 300);
    CHECK(touch->PageY() == 20);
    CHECK(touch->ClientX() == 10);
    CHECK(touch->RadiusX() == 3);
    CHECK(touch->RadiusY() == 2);
    CHECK(touch->RotationAngle() == 15);
    CHECK(touch->Force() == 0.5f);
    CHECK(touch->Twist() == 30);

    auto clone = touch->CloneWithNewTarget(nullptr);
    CHECK(clone->Target() == nullptr);
    CHECK(clone->ClientY() == 20);

    auto list = dom::TouchList::Create({touch});
    CHECK(list->Length() == 1);
    CHECK(list->Item(0) == touch.get());
    CHECK(list->Item(1) == nullptr);

    dom::TouchEventInit init;
    init.ChangedTouches.push_back(touch);
    init.CtrlKey = true;
    auto event = CreateRef<dom::TouchEvent>(dom::DOMStringAtom(u8"touchstart"), init);
    CHECK_FALSE(event->IsTrusted());
    CHECK(event->Touches()->Length() == 0);
    CHECK(event->ChangedTouches()->Length() == 1);
    CHECK(event->CtrlKey());

    auto cancel = dom::TouchEvent::Create(list.get(), list.get(), list.get(),
                                          dom::DOMStringAtom(u8"touchcancel"), nullptr, {}, {});
    CHECK(cancel->IsTrusted());
    CHECK(cancel->Bubbles());
    CHECK_FALSE(cancel->Cancelable());
  }

  TEST_CASE("EventHandler: touches", "[Page][TouchEvent]")
  {
    TouchFixture fixture;
    auto &a = fixture.ById(u8"a");
    auto &b = fixture.ById(u8"b");

    SECTION("a touch's pointer events, then its touch events, at the element it began on")
    {
      (void)fixture.Touch(Type::TouchStart, {TouchFixture::Point(0, State::TouchPressed, 10, 10)});
      CHECK(fixture.Take()
            == Strings {"pointerover3:a", "pointerenter3:a", "pointerdown3:a", "touchstart[1,1,1]:a"});

      // The touch is captured implicitly, before its next pointer event, and keeps its target as it moves
      // onto b.
      (void)fixture.Touch(Type::TouchMove, {TouchFixture::Point(0, State::TouchMoved, 10, 70)});
      CHECK(fixture.Take() == Strings {"gotpointercapture3:a", "pointermove3:a", "touchmove[1,1,1]:a"});
      CHECK(a.HasPointerCapture(3));

      (void)fixture.Touch(Type::TouchEnd, {TouchFixture::Point(0, State::TouchReleased, 10, 70)});
      CHECK(fixture.Take()
            == Strings {"pointerup3:a", "lostpointercapture3:a", "pointerout3:a", "pointerleave3:a",
                        "touchend[0,0,1]:a"});
    }

    SECTION("the pointer and touch events' properties")
    {
      RefPtr<dom::PointerEvent> down;
      RefPtr<dom::TouchEvent> start;
      fixture.On(a, u8"pointerdown",
                 [&](dom::Event &event) { down = ShareRefPtr(&Downcast<dom::PointerEvent>(event)); });
      fixture.On(a, u8"touchstart",
                 [&](dom::Event &event) { start = ShareRefPtr(&Downcast<dom::TouchEvent>(event)); });
      (void)fixture.Touch(Type::TouchStart, {TouchFixture::Point(2, State::TouchPressed, 10.5, 20.5)});
      REQUIRE(down);
      CHECK(down->IsTrusted());
      CHECK(down->PointerType() == u8"touch");
      CHECK(down->PointerId() == 5);
      CHECK(down->GetIsPrimary());
      CHECK(down->Width() == 6);
      CHECK(down->Height() == 4);
      CHECK(down->Pressure() == 0.25f);
      CHECK(down->ClientX() == 10.5);
      CHECK(down->Buttons() == 1);
      CHECK(down->Button() == 0);
      REQUIRE(start);
      CHECK(start->IsTrusted());
      CHECK(start->Cancelable());
      auto *touch = start->ChangedTouches()->Item(0);
      REQUIRE(touch);
      CHECK(touch->Identifier() == 2);
      CHECK(touch->Target() == &a);
      CHECK(touch->ClientX() == 10);
      CHECK(touch->RadiusX() == 3);
      CHECK(touch->Force() == 0.25f);
    }

    SECTION("two touches: lists per target, and only the first is primary")
    {
      (void)fixture.Touch(Type::TouchStart, {TouchFixture::Point(0, State::TouchPressed, 10, 10),
                                             TouchFixture::Point(1, State::TouchPressed, 10, 70)});
      CHECK(fixture.Take()
            == Strings {"pointerover3:a", "pointerenter3:a", "pointerdown3:a", "pointerover4:b",
                        "pointerenter4:b", "pointerdown4:b", "touchstart[2,1,2]:a", "touchstart[2,1,2]:b"});

      bool aPrimary = false;
      bool bPrimary = true;
      fixture.On(a, u8"pointermove",
                 [&](dom::Event &event) { aPrimary = Downcast<dom::PointerEvent>(event).GetIsPrimary(); });
      fixture.On(b, u8"pointermove",
                 [&](dom::Event &event) { bPrimary = Downcast<dom::PointerEvent>(event).GetIsPrimary(); });
      // A stationary touch has a pointermove, but isn't among the changed touches.
      (void)fixture.Touch(Type::TouchMove, {TouchFixture::Point(0, State::TouchStationary, 10, 10),
                                            TouchFixture::Point(1, State::TouchMoved, 20, 70)});
      CHECK(fixture.Take()
            == Strings {"gotpointercapture3:a", "pointermove3:a", "gotpointercapture4:b", "pointermove4:b",
                        "touchmove[2,1,1]:b"});
      CHECK(aPrimary);
      CHECK_FALSE(bPrimary);

      (void)fixture.Touch(Type::TouchEnd, {TouchFixture::Point(0, State::TouchStationary, 10, 10),
                                           TouchFixture::Point(1, State::TouchReleased, 20, 70)});
      CHECK(fixture.Take()
            == Strings {"pointermove3:a", "pointerup4:b", "lostpointercapture4:b", "pointerout4:b",
                        "pointerleave4:b", "touchend[1,0,1]:b"});
    }

    SECTION("a cancelled touch")
    {
      (void)fixture.Touch(Type::TouchStart, {TouchFixture::Point(0, State::TouchPressed, 10, 10)});
      (void)fixture.Touch(Type::TouchMove, {TouchFixture::Point(0, State::TouchMoved, 10, 20)});
      (void)fixture.Take();
      (void)fixture.Touch(Type::TouchCancel, {TouchFixture::Point(0, State::TouchCancelled, 10, 20)});
      CHECK(fixture.Take()
            == Strings {"pointercancel3:a", "pointerout3:a", "pointerleave3:a", "lostpointercapture3:a",
                        "touchcancel[0,0,1]:a"});

      // The touch's id starts afresh.
      (void)fixture.Touch(Type::TouchStart, {TouchFixture::Point(0, State::TouchPressed, 10, 70)});
      CHECK(fixture.Take()
            == Strings {"pointerover3:b", "pointerenter3:b", "pointerdown3:b", "touchstart[1,1,1]:b"});
    }

    SECTION("cancelling touchstart swallows the event; capture released in pointerdown never starts")
    {
      fixture.On(a, u8"touchstart", [&](dom::Event &event) { event.PreventDefault(); });
      fixture.On(a, u8"pointerdown",
                 [&](dom::Event &event)
                 {
                   (void)a.ReleasePointerCapture(
                     static_cast<int32>(Downcast<dom::PointerEvent>(event).PointerId()));
                 });
      CHECK(fixture.Touch(Type::TouchStart, {TouchFixture::Point(0, State::TouchPressed, 10, 10)}));
      CHECK(fixture.Take()
            == Strings {"pointerover3:a", "pointerenter3:a", "pointerdown3:a", "touchstart[1,1,1]:a"});
      (void)fixture.Touch(Type::TouchMove, {TouchFixture::Point(0, State::TouchMoved, 10, 20)});
      CHECK(fixture.Take() == Strings {"pointermove3:a", "touchmove[1,1,1]:a"});
      CHECK_FALSE(fixture.Touch(Type::TouchEnd, {TouchFixture::Point(0, State::TouchReleased, 10, 20)}));
    }

    SECTION("no mouse events, and no click, come from touches")
    {
      (void)fixture.Touch(Type::TouchStart, {TouchFixture::Point(0, State::TouchPressed, 10, 10)});
      (void)fixture.Touch(Type::TouchEnd, {TouchFixture::Point(0, State::TouchReleased, 10, 10)});
      for (auto &record : fixture.Take())
      {
        CHECK_FALSE(record.starts_with("mouse"));
        CHECK_FALSE(record.starts_with("click"));
      }
    }
  }

  TEST_CASE("EventHandler: a pen's pointer events", "[Page][PointerEvent]")
  {
    TouchFixture fixture;
    auto &a = fixture.ById(u8"a");

    auto penEvent = [](PlatformEvent::Type type, MouseButton button, uint16 buttons, double force)
    {
      PlatformMouseEvent event(DoublePoint(10, 10), DoublePoint(10, 10), button, type,
                               type == PlatformEvent::Type::MouseMoved ? 0 : 1, {}, MonotonicTime::Now(),
                               buttons, {}, force, SyntheticClickType::NoTap, PenPointerID,
                               dom::PenPointerEventType());
      event.SetPenOrientation({30, 0, 45});
      if (type == PlatformEvent::Type::MouseMoved)
      {
        event.SetCoalescedEvents({event});
      }
      return event;
    };

    RefPtr<dom::PointerEvent> move;
    RefPtr<dom::PointerEvent> down;
    fixture.On(a, u8"pointermove",
               [&](dom::Event &event) { move = ShareRefPtr(&Downcast<dom::PointerEvent>(event)); });
    fixture.On(a, u8"pointerdown",
               [&](dom::Event &event) { down = ShareRefPtr(&Downcast<dom::PointerEvent>(event)); });

    // Hovering is a move without buttons.
    (void)fixture.Handler().MouseMoved(penEvent(PlatformEvent::Type::MouseMoved, MouseButton::None, 0, 0));
    REQUIRE(move);
    CHECK(move->PointerType() == u8"pen");
    CHECK(move->PointerId() == PenPointerID);
    CHECK(move->Pressure() == 0);
    CHECK(move->Buttons() == 0);
    CHECK(move->TiltX() == 30);
    CHECK(move->TiltY() == 0);
    CHECK(move->Twist() == 45);
    CHECK(move->AzimuthAngle() == 0);
    CHECK(move->AltitudeAngle() > 1.04);
    CHECK(move->AltitudeAngle() < 1.05);
    REQUIRE(move->GetCoalescedEvents().size() == 1);
    CHECK(move->GetCoalescedEvents()[0]->TiltX() == 30);

    (void)fixture.Handler().HandleMousePressEvent(
      penEvent(PlatformEvent::Type::MousePressed, MouseButton::Left, 1, 0.75));
    REQUIRE(down);
    CHECK(down->Pressure() == 0.75f);
    CHECK(down->Buttons() == 1);
    CHECK(down->TiltX() == 30);
  }

  TEST_CASE("Navigator: maxTouchPoints", "[Page][TouchEvent]")
  {
    TouchFixture fixture;
    auto &navigator = fixture.Document->Window()->GetNavigator();
    CHECK(navigator.MaxTouchPoints() == 0);

    auto environment = fixture.Frame->GetMediaEnvironment();
    environment.MaxTouchPoints = 10;
    fixture.Frame->SetMediaEnvironment(environment);
    CHECK(navigator.MaxTouchPoints() == 10);
  }
}
