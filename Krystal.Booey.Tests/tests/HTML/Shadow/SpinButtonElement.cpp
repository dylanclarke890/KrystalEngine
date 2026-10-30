#include "Krystal.Booey/HTML/Shadow/SpinButtonElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Editing/EditorTestUtils.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Platform/PlatformMouseEvent.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"

namespace krys::boo::tests
{
  namespace
  {
    KRYS_NODISCARD PlatformMouseEvent MouseEvent(PlatformEvent::Type type, IntPoint position) noexcept
    {
      DoublePoint point(position.X(), position.Y());
      int32 clickCount = type == PlatformEvent::Type::MousePressed ? 1 : 0;
      return PlatformMouseEvent(
        point, point, type == PlatformEvent::Type::MouseMoved ? MouseButton::None : MouseButton::Left, type,
        clickCount, {}, MonotonicTime::Now());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SpinButtonElement: number fields step with their spin button",
                   "[HTML][SpinButtonElement]")
  {
    EditorFixture fixture(u8"<style>body{margin:0}</style><input type=number id=number value=5>"
                          u8"<input id=text><input type=range id=range>");
    auto &number = Downcast<html::HTMLInputElement>(fixture.ById(u8"number"));
    auto &handler = fixture.Frame->GetEventHandler();

    // Only steppable fields that aren't ranges have one.
    REQUIRE(number.InnerSpinButtonElement());
    CHECK_FALSE(Downcast<html::HTMLInputElement>(fixture.ById(u8"text")).InnerSpinButtonElement());
    CHECK_FALSE(Downcast<html::HTMLInputElement>(fixture.ById(u8"range")).InnerSpinButtonElement());

    auto &spinButton = Downcast<html::SpinButtonElement>(*number.InnerSpinButtonElement());
    auto bounds = spinButton.GetRenderBox()->AbsoluteBoundingBoxRect();
    IntPoint upper(bounds.X() + bounds.Width() / 2, bounds.Y() + bounds.Height() / 4);
    IntPoint lower(bounds.X() + bounds.Width() / 2, bounds.MaxY() - bounds.Height() / 4);

    // Moving over the upper half and pressing steps up, and focuses the field; the lower half steps down.
    (void)handler.MouseMoved(MouseEvent(PlatformEvent::Type::MouseMoved, upper));
    CHECK(spinButton.GetUpDownState() == html::SpinButtonElement::UpDownState::Up);
    (void)handler.HandleMousePressEvent(MouseEvent(PlatformEvent::Type::MousePressed, upper));
    (void)handler.HandleMouseReleaseEvent(MouseEvent(PlatformEvent::Type::MouseReleased, upper));
    CHECK(number.Value() == u8"6");
    CHECK(fixture.Document->FocusedElement() == &number);

    (void)handler.MouseMoved(MouseEvent(PlatformEvent::Type::MouseMoved, lower));
    CHECK(spinButton.GetUpDownState() == html::SpinButtonElement::UpDownState::Down);
    (void)handler.HandleMousePressEvent(MouseEvent(PlatformEvent::Type::MousePressed, lower));
    (void)handler.HandleMouseReleaseEvent(MouseEvent(PlatformEvent::Type::MouseReleased, lower));
    (void)handler.HandleMousePressEvent(MouseEvent(PlatformEvent::Type::MousePressed, lower));
    (void)handler.HandleMouseReleaseEvent(MouseEvent(PlatformEvent::Type::MouseReleased, lower));
    CHECK(number.Value() == u8"4");

    // Leaving it releases the capture; a read-only field doesn't step.
    (void)handler.MouseMoved(MouseEvent(PlatformEvent::Type::MouseMoved, {bounds.X() - 40, bounds.Y()}));
    CHECK(spinButton.GetUpDownState() == html::SpinButtonElement::UpDownState::Indeterminate);
    number.SetAttribute(u8"readonly", u8"");
    (void)fixture.Document->UpdateLayout();
    (void)handler.MouseMoved(MouseEvent(PlatformEvent::Type::MouseMoved, upper));
    (void)handler.HandleMousePressEvent(MouseEvent(PlatformEvent::Type::MousePressed, upper));
    (void)handler.HandleMouseReleaseEvent(MouseEvent(PlatformEvent::Type::MouseReleased, upper));
    CHECK(number.Value() == u8"4");
  }
}
