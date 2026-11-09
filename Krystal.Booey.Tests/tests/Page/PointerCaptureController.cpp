#include "Krystal.Booey/Page/PointerCaptureController.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/DOM/Events/EventListener.hpp"
#include "Krystal.Booey/DOM/Events/PointerEvent.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/Page.hpp"
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

    /// @brief Records the events it handles as `type:target`, with `!` after the type of a pointer event.
    struct Recorder : public dom::EventListenerCallback
    {
      List<std::string> &Records;

      explicit Recorder(List<std::string> &records) noexcept : Records(records)
      {
      }

      void HandleEvent(dom::Event &event) noexcept override
      {
        auto type = std::string(event.Type().View().begin(), event.Type().View().end());
        Records.push_back(type + (event.IsPointerEvent() ? "!" : "") + ":" + Name(event.Target().get()));
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

    /// @brief Two 100x50 boxes, `a` above `b`, in a page, recording the pointer and mouse events at its
    /// document.
    struct PointerFixture : PaintFixture
    {
      List<std::string> Records;

      explicit PointerFixture(bool recordPointerMoves = true) noexcept
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
              u8"lostpointercapture", u8"mouseover", u8"mouseout", u8"mouseenter", u8"mouseleave",
              u8"mousedown", u8"mousemove", u8"mouseup", u8"click", u8"auxclick"})
        {
          if (recordPointerMoves || type != utf8_stringview(u8"pointermove"))
          {
            Document->AddEventListener(dom::DOMStringAtom(type), Ref {recorder}, true);
          }
        }
      }

      KRYS_NODISCARD EventHandler &Handler() noexcept
      {
        return Frame->GetEventHandler();
      }

      KRYS_NODISCARD PointerCaptureController &Controller() noexcept
      {
        return TestPage->GetPointerCaptureController();
      }

      KRYS_NODISCARD static PlatformMouseEvent Event(PlatformEvent::Type type, double x, double y,
                                                     MouseButton button, uint16 buttons) noexcept
      {
        PlatformMouseEvent event(DoublePoint(x, y), DoublePoint(x, y), button, type,
                                 type == PlatformEvent::Type::MouseMoved ? 0 : 1, {}, MonotonicTime::Now(),
                                 buttons);
        if (type == PlatformEvent::Type::MouseMoved)
        {
          event.SetCoalescedEvents({event});
        }
        return event;
      }

      void Move(double x, double y, uint16 buttons = 0) noexcept
      {
        (void)Handler().MouseMoved(Event(PlatformEvent::Type::MouseMoved, x, y, MouseButton::None, buttons));
      }

      void Press(double x, double y, MouseButton button = MouseButton::Left, uint16 buttons = 1) noexcept
      {
        (void)Handler().HandleMousePressEvent(
          Event(PlatformEvent::Type::MousePressed, x, y, button, buttons));
      }

      void Release(double x, double y, MouseButton button = MouseButton::Left, uint16 buttons = 0) noexcept
      {
        (void)Handler().HandleMouseReleaseEvent(
          Event(PlatformEvent::Type::MouseReleased, x, y, button, buttons));
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
      void On(dom::Element &element, utf8_stringview type, TAction action) noexcept
      {
        Ref<dom::EventListenerCallback> listener = CreateRef<ActionListener<TAction>>(krys::move(action));
        element.AddEventListener(dom::DOMStringAtom(type), krys::move(listener), false);
      }
    };
  }

  TEST_CASE("PointerCaptureController: pointer events from the mouse", "[Page][PointerEvent]")
  {
    PointerFixture fixture;
    auto &a = fixture.ById(u8"a");

    SECTION("each comes before its compatibility mouse event; clicks are pointer events")
    {
      fixture.Move(10, 10);
      CHECK(fixture.Take()
            == Strings {"pointerover!:a", "mouseover:a", "pointerenter!:a", "mouseenter:a", "pointermove!:a",
                        "mousemove:a"});
      fixture.Press(10, 10);
      CHECK(fixture.Take() == Strings {"pointerdown!:a", "mousedown:a"});
      fixture.Release(10, 10);
      CHECK(fixture.Take() == Strings {"pointerup!:a", "mouseup:a", "click!:a"});

      fixture.Move(10, 70);
      CHECK(fixture.Take()
            == Strings {"pointerout!:a", "mouseout:a", "pointerleave!:a", "mouseleave:a", "pointerover!:b",
                        "mouseover:b", "pointerenter!:b", "mouseenter:b", "pointermove!:b", "mousemove:b"});
    }

    SECTION("the pointer events' properties")
    {
      RefPtr<dom::PointerEvent> down;
      RefPtr<dom::PointerEvent> click;
      fixture.On(a, u8"pointerdown",
                 [&](dom::Event &event) { down = ShareRefPtr(&Downcast<dom::PointerEvent>(event)); });
      fixture.On(a, u8"click",
                 [&](dom::Event &event) { click = ShareRefPtr(&Downcast<dom::PointerEvent>(event)); });
      fixture.Press(10.5, 20.5);
      fixture.Release(10.5, 20.5);
      REQUIRE(down);
      CHECK(down->PointerId() == MousePointerID);
      CHECK(down->PointerType() == u8"mouse");
      CHECK(down->GetIsPrimary());
      CHECK(down->Pressure() == 0.5f);
      CHECK(down->ClientX() == 10.5);
      REQUIRE(click);
      CHECK(click->PointerId() == MousePointerID);
      CHECK(click->Detail() == 1);
      CHECK(click->ClientX() == 10);
    }

    SECTION("pointerrawupdate comes before each pointermove, and can't be cancelled")
    {
      List<Ref<dom::PointerEvent>> rawUpdates;
      fixture.On(a, u8"pointerrawupdate",
                 [&](dom::Event &event)
                 {
                   fixture.Records.push_back("raw:a");
                   rawUpdates.push_back(ShareRef(Downcast<dom::PointerEvent>(event)));
                   event.PreventDefault();
                 });

      fixture.Move(10, 10);
      CHECK(fixture.Take()
            == Strings {"pointerover!:a", "mouseover:a", "pointerenter!:a", "mouseenter:a", "raw:a",
                        "pointermove!:a", "mousemove:a"});
      REQUIRE(rawUpdates.size() == 1uz);
      auto &rawUpdate = *rawUpdates[0];
      CHECK(rawUpdate.IsTrusted());
      CHECK(rawUpdate.Bubbles());
      CHECK(rawUpdate.Composed());
      CHECK_FALSE(rawUpdate.Cancelable());
      CHECK_FALSE(rawUpdate.DefaultPrevented());
      CHECK(rawUpdate.ClientX() == 10);
      auto coalesced = rawUpdate.GetCoalescedEvents();
      REQUIRE(coalesced.size() == 1uz);
      CHECK(coalesced[0]->Type() == EventNames::PointerRawUpdate);
      CHECK(rawUpdate.GetPredictedEvents().empty());

      // Pressing and releasing the only button have none; a chorded button is a move, and has one.
      fixture.Press(10, 10);
      CHECK(fixture.Take() == Strings {"pointerdown!:a", "mousedown:a"});
      fixture.Press(10, 10, MouseButton::Right, 3);
      CHECK(fixture.Take() == Strings {"raw:a", "pointermove!:a", "mousedown:a"});
    }

    SECTION("a cancelled pointerdown stops the compatibility mouse events until the release")
    {
      fixture.Move(10, 10);
      (void)fixture.Take();
      fixture.On(a, u8"pointerdown", [](dom::Event &event) { event.PreventDefault(); });
      fixture.Press(10, 10);
      CHECK(fixture.Take() == Strings {"pointerdown!:a"});
      fixture.Move(20, 20, 1);
      CHECK(fixture.Take() == Strings {"pointermove!:a"});
      // The click isn't a compatibility mouse event.
      fixture.Release(20, 20);
      CHECK(fixture.Take() == Strings {"pointerup!:a", "click!:a"});
      fixture.Move(30, 20);
      CHECK(fixture.Take() == Strings {"pointermove!:a", "mousemove:a"});
    }

    SECTION("a second button is a pointermove")
    {
      fixture.Move(10, 10);
      fixture.Press(10, 10);
      (void)fixture.Take();
      fixture.Press(10, 10, MouseButton::Right, 3);
      CHECK(fixture.Take() == Strings {"pointermove!:a", "mousedown:a"});
      fixture.Release(10, 10, MouseButton::Right, 1);
      auto records = fixture.Take();
      REQUIRE(records.size() >= 2);
      CHECK(records[0] == "pointermove!:a");
      CHECK(records[1] == "mouseup:a");
      fixture.Release(10, 10);
      records = fixture.Take();
      REQUIRE(records.size() >= 2);
      CHECK(records[0] == "pointerup!:a");
      CHECK(records[1] == "mouseup:a");
    }

    SECTION("a simulated click is a pointer event from no pointing device")
    {
      RefPtr<dom::PointerEvent> click;
      fixture.On(a, u8"click",
                 [&](dom::Event &event) { click = ShareRefPtr(&Downcast<dom::PointerEvent>(event)); });
      Downcast<html::HTMLElement>(a).Click();
      REQUIRE(click);
      CHECK(click->PointerId() == static_cast<PointerID>(-1));
      CHECK(click->PointerType().empty());
      CHECK_FALSE(click->IsTrusted());
      CHECK(click->ClientX() == 0);
    }
  }

  TEST_CASE("PointerCaptureController: moves with no pointermove listener", "[Page][PointerEvent]")
  {
    PointerFixture fixture(false);
    auto &a = fixture.ById(u8"a");
    CHECK_FALSE(fixture.Document->HasPointerMoveListener());
    CHECK_FALSE(fixture.Document->HasPointerRawUpdateListener());

    SECTION("the boundary and mouse events still fire")
    {
      fixture.Move(10, 10);
      CHECK(fixture.Take()
            == Strings {"pointerover!:a", "mouseover:a", "pointerenter!:a", "mouseenter:a", "mousemove:a"});
    }

    SECTION("a cancelled pointerdown still stops the compatibility mouse events until the release")
    {
      fixture.Move(10, 10);
      (void)fixture.Take();
      fixture.On(a, u8"pointerdown", [](dom::Event &event) { event.PreventDefault(); });
      fixture.Press(10, 10);
      CHECK(fixture.Take() == Strings {"pointerdown!:a"});
      fixture.Move(20, 20, 1);
      CHECK(fixture.Take().empty());
      fixture.Release(20, 20);
      CHECK(fixture.Take() == Strings {"pointerup!:a", "click!:a"});
      fixture.Move(30, 20);
      CHECK(fixture.Take() == Strings {"mousemove:a"});
    }

    SECTION("a pointer's capture still applies")
    {
      fixture.Move(10, 10);
      fixture.Press(10, 10);
      (void)fixture.Take();
      REQUIRE_FALSE(a.SetPointerCapture(MousePointerID).HasException());
      fixture.Move(10, 70, 1);
      CHECK(fixture.Take() == Strings {"gotpointercapture!:a", "mousemove:a"});
    }

    SECTION("a listener however it's added is noticed")
    {
      fixture.Move(10, 10);
      uint32 moves = 0;
      SECTION("on a node")
      {
        fixture.On(a, u8"pointermove", [&](dom::Event &) { ++moves; });
      }
      SECTION("as a handler")
      {
        Downcast<html::HTMLElement>(a).SetOnPointerMove([&](dom::Event &) { ++moves; });
      }
      SECTION("on the window")
      {
        fixture.Document->Window()->SetOnPointerMove([&](dom::Event &) { ++moves; });
      }
      SECTION("on a node adopted into the document")
      {
        auto other = fixture.Document->Implementation().CreateHTMLDocument(u8"other").ReleaseValue();
        auto div = other->CreateElement(u8"div").ReleaseValue();
        fixture.On(*div, u8"pointermove", [](dom::Event &) {});
        REQUIRE_FALSE(a.AppendChild(*div).HasException());
        CHECK(fixture.Document->HasPointerMoveListener());
        fixture.On(a, u8"pointermove", [&](dom::Event &) { ++moves; });
      }
      CHECK(fixture.Document->HasPointerMoveListener());
      fixture.Move(20, 10);
      CHECK(moves == 1);
    }

    SECTION("pointerrawupdate is noticed on its own, and can add a pointermove listener")
    {
      fixture.Move(10, 10);
      uint32 moves = 0;
      uint32 rawUpdates = 0;
      fixture.On(a, u8"pointerrawupdate",
                 [&](dom::Event &)
                 {
                   if (!rawUpdates++)
                   {
                     fixture.On(a, u8"pointermove", [&](dom::Event &) { ++moves; });
                   }
                 });
      CHECK(fixture.Document->HasPointerRawUpdateListener());
      CHECK_FALSE(fixture.Document->HasPointerMoveListener());
      fixture.Move(20, 10);
      CHECK(rawUpdates == 1);
      CHECK(moves == 1);
    }
  }

  TEST_CASE("PointerCaptureController: pointer capture", "[Page][PointerEvent]")
  {
    PointerFixture fixture;
    auto &a = fixture.ById(u8"a");

    auto captureOnPress = [&]
    {
      fixture.On(a, u8"pointerdown",
                 [&](dom::Event &event)
                 {
                   auto pointerId = static_cast<int32>(Downcast<dom::PointerEvent>(event).PointerId());
                   CHECK_FALSE(a.SetPointerCapture(pointerId).HasException());
                   CHECK(a.HasPointerCapture(pointerId));
                 });
    };

    SECTION("a captured pointer's events go to the capturing element until it's released")
    {
      captureOnPress();
      fixture.Move(10, 10);
      fixture.Press(10, 10);
      (void)fixture.Take();
      // Capture takes effect before the next event, which goes to the capturing element wherever it is.
      fixture.Move(10, 70, 1);
      CHECK(fixture.Take() == Strings {"gotpointercapture!:a", "pointermove!:a", "mousemove:a"});
      CHECK(fixture.Controller().PointerCaptureElement(fixture.Document.get(), MousePointerID) == &a);
      CHECK(a.HasPointerCapture(MousePointerID));

      // The release releases it implicitly, and the click goes to the capturing element.
      fixture.Release(10, 70);
      auto records = fixture.Take();
      REQUIRE(records.size() >= 3);
      CHECK(records[0] == "pointerup!:a");
      CHECK(records[1] == "lostpointercapture!:a");
      CHECK(std::ranges::find(records, "click!:a") != records.end());
      CHECK_FALSE(a.HasPointerCapture(MousePointerID));
      CHECK_FALSE(fixture.Controller().PointerCaptureElement(fixture.Document.get(), MousePointerID));

      // Then the mouse's events go where it is.
      fixture.Move(10, 75);
      records = fixture.Take();
      CHECK(std::ranges::find(records, "pointermove!:b") != records.end());
    }

    SECTION("releasing the capture")
    {
      captureOnPress();
      fixture.Press(10, 10);
      fixture.Move(10, 70, 1);
      (void)fixture.Take();
      CHECK_FALSE(a.ReleasePointerCapture(MousePointerID).HasException());
      CHECK_FALSE(a.HasPointerCapture(MousePointerID));
      // The capture lasts until the pointer's next event, as the release is only pending.
      CHECK(fixture.Take().empty());
      CHECK(fixture.Controller().PointerCaptureElement(fixture.Document.get(), MousePointerID) == &a);
      fixture.Move(10, 75, 1);
      auto records = fixture.Take();
      REQUIRE_FALSE(records.empty());
      CHECK(records[0] == "lostpointercapture!:a");
      CHECK(std::ranges::find(records, "pointermove!:b") != records.end());
      CHECK_FALSE(fixture.Controller().PointerCaptureElement(fixture.Document.get(), MousePointerID));
    }

    SECTION("removing the capturing element fires lostpointercapture at the document")
    {
      captureOnPress();
      fixture.Press(10, 10);
      fixture.Move(10, 70, 1);
      (void)fixture.Take();
      Ref protectedA = ShareRef(a);
      (void)a.Remove();
      CHECK_FALSE(a.HasPointerCapture(MousePointerID));
      html::EventLoop::Current().RunUntilIdle();
      CHECK(fixture.Take() == Strings {"lostpointercapture!:document"});
    }

    SECTION("capturing an unknown pointer, from a disconnected element, or without a press")
    {
      auto unknown = a.SetPointerCapture(99);
      REQUIRE(unknown.HasException());
      CHECK(unknown.GetException().Code() == dom::ExceptionCode::NotFoundError);
      auto unknownRelease = a.ReleasePointerCapture(99);
      REQUIRE(unknownRelease.HasException());
      CHECK(unknownRelease.GetException().Code() == dom::ExceptionCode::NotFoundError);

      auto disconnected = fixture.Document->CreateElement(u8"div").ReleaseValue();
      auto result = disconnected->SetPointerCapture(MousePointerID);
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == dom::ExceptionCode::InvalidStateError);

      // The mouse isn't pressed, so nothing captures it.
      CHECK_FALSE(a.SetPointerCapture(MousePointerID).HasException());
      CHECK_FALSE(a.HasPointerCapture(MousePointerID));
    }

    SECTION("cancelling a pointer")
    {
      fixture.Move(10, 10);
      fixture.Press(10, 10);
      (void)fixture.Take();
      fixture.Controller().CancelPointer(MousePointerID, IntPoint(10, 10));
      CHECK(fixture.Take() == Strings {"pointercancel!:a", "pointerout!:a", "pointerleave!:a"});
      CHECK(fixture.Controller().HasCancelledPointerEventForIdentifier(MousePointerID));
      // Only once.
      fixture.Controller().CancelPointer(MousePointerID, IntPoint(10, 10));
      CHECK(fixture.Take().empty());

      // The page's main document changing forgets it.
      fixture.TestPage->DidChangeMainDocument(fixture.Document.get());
      CHECK_FALSE(fixture.Controller().HasCancelledPointerEventForIdentifier(MousePointerID));
    }
  }
}
