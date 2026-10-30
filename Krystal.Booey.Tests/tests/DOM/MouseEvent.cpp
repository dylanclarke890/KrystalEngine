#include "Krystal.Booey/DOM/MouseEvent.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Platform/PlatformMouseEvent.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    /// @brief Keeps the last mouse event it handles, with its offsets and layer position read during
    /// dispatch.
    struct OffsetRecorder : public EventListenerCallback
    {
      double OffsetX {-1}, OffsetY {-1};
      int32 LayerX {-1}, LayerY {-1};
      double PageX {-1}, ClientX {-1};

      void HandleEvent(Event &event) noexcept override
      {
        auto &mouse = static_cast<MouseEvent &>(event);
        OffsetX = mouse.OffsetX();
        OffsetY = mouse.OffsetY();
        LayerX = mouse.LayerX();
        LayerY = mouse.LayerY();
        PageX = mouse.PageX();
        ClientX = mouse.ClientX();
      }
    };
  }

  TEST_CASE("MouseEvent: from a dictionary", "[DOM][MouseEvent]")
  {
    Ref<MouseEvent> defaults = CreateRef<MouseEvent>(DOMStringAtom(u8"click"));
    CHECK(defaults->Button() == 0);
    CHECK(defaults->Buttons() == 0);
    CHECK(defaults->ButtonDown());
    CHECK(defaults->Which() == 1);
    CHECK(defaults->ClientX() == 0);
    CHECK_FALSE(defaults->CtrlKey());
    CHECK_FALSE(defaults->IsTrusted());
    CHECK_FALSE(defaults->Bubbles());
    CHECK(defaults->RelatedTarget() == nullptr);
    CHECK(defaults->IsMouseEvent());

    Ref<EventTarget> related = CreateRef<EventTarget>();
    MouseEventInit init;
    init.Bubbles = true;
    init.ScreenX = 30.7;
    init.ScreenY = 40;
    init.ClientX = 10.5;
    init.ClientY = 20.25;
    init.Button = 2;
    init.Buttons = 3;
    init.CtrlKey = true;
    init.ModifierCapsLock = true;
    init.RelatedTarget = related;
    init.Detail = 2;
    Ref<MouseEvent> event = CreateRef<MouseEvent>(DOMStringAtom(u8"mousedown"), init);
    CHECK(event->Bubbles());
    CHECK(event->Detail() == 2);
    CHECK(event->ScreenX() == 30);
    CHECK(event->ScreenY() == 40);
    // Coordinates are floored; with no view the page is the client area.
    CHECK(event->ClientX() == 10);
    CHECK(event->ClientY() == 20);
    CHECK(event->X() == 10);
    CHECK(event->PageX() == 10);
    CHECK(event->PageY() == 20);
    CHECK(event->Button() == 2);
    CHECK(event->ButtonType() == MouseButton::Right);
    CHECK(event->Buttons() == 3);
    CHECK(event->Which() == 3);
    CHECK(event->CtrlKey());
    CHECK_FALSE(event->ShiftKey());
    CHECK(event->GetModifierState(u8"Control"));
    CHECK(event->GetModifierState(u8"CapsLock"));
    CHECK_FALSE(event->GetModifierState(u8"Alt"));
    CHECK_FALSE(event->GetModifierState(u8"Fn"));
    CHECK(event->RelatedTarget() == related);
    // Movement is only kept for moves.
    MouseEventInit moved;
    moved.MovementX = 4;
    CHECK(CreateRef<MouseEvent>(DOMStringAtom(u8"mousemove"), moved)->MovementX() == 4);

    SECTION("initMouseEvent")
    {
      event->InitMouseEvent(DOMStringAtom(u8"mouseup"), false, true, 1, 1, 2, 3, 4, false, true, true, false,
                            1, nullptr);
      CHECK(event->Type() == u8"mouseup");
      CHECK_FALSE(event->Bubbles());
      CHECK(event->Cancelable());
      CHECK(event->Detail() == 1);
      CHECK(event->ScreenX() == 1);
      CHECK(event->ClientY() == 4);
      CHECK_FALSE(event->CtrlKey());
      CHECK(event->AltKey());
      CHECK(event->ShiftKey());
      CHECK(event->ButtonType() == MouseButton::Middle);
      CHECK(event->RelatedTarget() == nullptr);
    }

    SECTION("which buttons can activate")
    {
      CHECK(MouseEvent::CanTriggerActivationBehavior(*defaults));
      CHECK_FALSE(MouseEvent::CanTriggerActivationBehavior(*event));
      MouseEventInit right;
      right.Button = 2;
      CHECK_FALSE(
        MouseEvent::CanTriggerActivationBehavior(*CreateRef<MouseEvent>(DOMStringAtom(u8"click"), right)));
      CHECK(IsAnyClick(DOMStringAtom(u8"auxclick")));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "MouseEvent: from the platform", "[DOM][MouseEvent]")
  {
    boo::tests::PaintFixture fixture {
      u8"<!DOCTYPE html><style>body{margin:0}</style>"
      u8"<div id=box style=margin-left:20px;margin-top:10px;width:50px;height:50px;"
      u8"position:relative;border:5px solid></div>"};
    (void)fixture.Document->UpdateLayout();
    auto &box = fixture.ById(u8"box");

    PlatformMouseEvent platform(DoublePoint(32.6, 27.2), DoublePoint(132, 227), MouseButton::Left,
                                PlatformEvent::Type::MousePressed, 1, {PlatformEvent::Modifier::ShiftKey},
                                MonotonicTime::Now(), 1, DoublePoint(3, 4));

    SECTION("enter and leave don't bubble")
    {
      auto enter = MouseEvent::Create(DOMStringAtom(u8"mouseenter"), fixture.Frame->View(), platform, {}, {},
                                      0, nullptr);
      CHECK_FALSE(enter->Bubbles());
      CHECK_FALSE(enter->Cancelable());
      CHECK_FALSE(enter->Composed());
      auto down =
        MouseEvent::Create(DOMStringAtom(u8"mousedown"), fixture.Frame->View(), platform, {}, {}, 1, nullptr);
      CHECK(down->Bubbles());
      CHECK(down->Cancelable());
      CHECK(down->Composed());
      CHECK(down->IsTrusted());
      CHECK(down->ShiftKey());
      CHECK(down->ScreenX() == 132);
      CHECK(down->ClientX() == 32);
      CHECK(down->Detail() == 1);
      CHECK(down->Buttons() == 1);
      // Movement is only kept for moves.
      CHECK(down->MovementX() == 0);
      CHECK(
        MouseEvent::Create(DOMStringAtom(u8"mousemove"), fixture.Frame->View(), platform, {}, {}, 0, nullptr)
          ->MovementY()
        == 4);
    }

    SECTION("offsets are relative to the target's border box, layer positions to its layer")
    {
      Ref<OffsetRecorder> recorder = CreateRef<OffsetRecorder>();
      box.AddEventListener(DOMStringAtom(u8"mousedown"), Ref<EventListenerCallback> {recorder}, false);
      (void)box.DispatchMouseEvent(platform, DOMStringAtom(u8"mousedown"), 1);
      CHECK(recorder->OffsetX == 13);
      CHECK(recorder->OffsetY == 17);
      CHECK(recorder->LayerX == 12);
      CHECK(recorder->LayerY == 17);
      CHECK(recorder->PageX == 32);
      CHECK(recorder->ClientX == 32);
    }

    SECTION("page zoom scales the page and client coordinates")
    {
      fixture.Frame->SetPageZoomFactor(2);
      (void)fixture.Document->UpdateLayout();
      auto down =
        MouseEvent::Create(DOMStringAtom(u8"mousedown"), fixture.Frame->View(), platform, {}, {}, 1, nullptr);
      CHECK(down->PageX() == 16);
      CHECK(down->ClientX() == 16);
    }
  }
}
