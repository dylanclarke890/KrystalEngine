#include "Krystal.Booey/HTML/Shadow/SliderThumbElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Editing/EditorTestUtils.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Platform/Events/PlatformMouseEvent.hpp"

namespace krys::boo::tests
{
  namespace
  {
    KRYS_NODISCARD PlatformMouseEvent MouseEvent(PlatformEvent::Type type, IntPoint position) noexcept
    {
      DoublePoint point(position.X(), position.Y());
      int32 clickCount = type == PlatformEvent::Type::MousePressed ? 1 : 0;
      return PlatformMouseEvent(point, point, MouseButton::Left, type, clickCount, {}, MonotonicTime::Now());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "SliderThumbElement: a drag releases the mouse when the slider's renderer goes",
                   "[HTML][SliderThumbElement]")
  {
    EditorFixture fixture(u8"<style>body{margin:0}input{margin:0}div{height:50px}</style>"
                          u8"<input type=range id=range value=50><div id=other></div>");
    auto &input = Downcast<html::HTMLInputElement>(fixture.ById(u8"range"));
    auto &handler = fixture.Frame->GetEventHandler();
    auto moves = CreateRef<CountingListener>();
    fixture.ById(u8"other").AddEventListener(dom::DOMStringAtom(EventNames::MouseMove), moves, false);

    // Pressing on the thumb starts a drag, which captures the mouse.
    (void)handler.HandleMousePressEvent(MouseEvent(PlatformEvent::Type::MousePressed, {64, 10}));

    // Hiding the slider mid-drag releases it, so moves go to what's under the mouse.
    input.SetAttribute(u8"style", u8"display:none");
    (void)fixture.Document->UpdateLayout();
    (void)handler.MouseMoved(MouseEvent(PlatformEvent::Type::MouseMoved, {64, 10}));
    CHECK(moves->Count == 1);
  }
}
