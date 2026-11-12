#include "Krystal.Booey/HTML/InputTypes/RadioInputType.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Editing/EditorTestUtils.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Platform/Events/PlatformKeyboardEvent.hpp"
#include "Krystal.Booey/Platform/Events/WindowsKeyboardCodes.hpp"

namespace krys::boo::tests
{
  TEST_CASE("RadioInputType: the arrow keys scroll the next button into view",
            "[HTML][Forms][Input][RadioInputType]")
  {
    EditorFixture fixture(u8"<style>body{margin:0}</style><input type=radio name=r id=a>"
                          u8"<div style='height:2000px'></div><input type=radio name=r id=b>");
    (void)fixture.Document->UpdateLayout();
    fixture.Document->GetSettings().ScrollAnimatorEnabled = false;
    auto &first = Downcast<html::HTMLInputElement>(fixture.ById(u8"a"));
    auto &second = Downcast<html::HTMLInputElement>(fixture.ById(u8"b"));
    first.Focus();
    REQUIRE(fixture.Frame->View()->GetScrollPosition().Y() == 0);

    // The next button in the group is focused, checked and scrolled to the nearest edge of the view.
    (void)fixture.Frame->GetEventHandler().KeyEvent(
      PlatformKeyboardEvent(PlatformEvent::Type::RawKeyDown, u8"", u8"", u8"ArrowDown", u8"ArrowDown",
                            u8"Down", vk::Down, false, false, false, {}, {}));
    CHECK(fixture.Document->FocusedElement() == &second);
    CHECK(second.Checked());
    auto scrollY = fixture.Frame->View()->GetScrollPosition().Y();
    CHECK(scrollY > 0);
    auto bottom = second.BoundingClientRect().MaxY();
    CHECK(bottom <= static_cast<float>(fixture.Frame->View()->VisibleHeight()));
  }
}
