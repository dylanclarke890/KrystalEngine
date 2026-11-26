#include "Krystal.Web/Html/InputTypes/RadioInputType.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Utils/ScopeExit.hpp"
#include "Krystal.Web.Tests/Editing/EditorTestUtils.hpp"
#include "Krystal.Web/Html/Elements/HTMLInputElement.hpp"
#include "Krystal.Web/Input/EventHandler.hpp"
#include "Krystal.Web/Input/LegacyKeyCodes.hpp"
#include "Krystal.Web/Input/PlatformKeyboardEvent.hpp"
#include "Krystal.Web/Page/Frame.hpp"
#include "Krystal.Web/Page/FrameView.hpp"
#include "Krystal.Web/Page/Settings.hpp"

namespace krys::web::tests
{
  TEST_CASE("RadioInputType: the arrow keys scroll the next button into view",
            "[HTML][Forms][Input][RadioInputType]")
  {
    EditorFixture fixture(u8"<style>body{margin:0}</style><input type=radio name=r id=a>"
                          u8"<div style='height:2000px'></div><input type=radio name=r id=b>");
    (void)fixture.Document->UpdateLayout();
    // The settings are shared, so they're put back.
    auto &settings = fixture.Document->GetSettings();
    settings.ScrollAnimatorEnabled = false;
    auto restoreSettings = MakeScopeExit([&] { settings.ScrollAnimatorEnabled = true; });
    auto &first = Downcast<HTMLInputElement>(fixture.ById(u8"a"));
    auto &second = Downcast<HTMLInputElement>(fixture.ById(u8"b"));
    first.Focus();
    REQUIRE(fixture.Frame->View()->GetScrollPosition().Y() == 0);

    // The next button in the group is focused, checked and scrolled to the nearest edge of the view.
    (void)fixture.Frame->GetEventHandler().KeyEvent(
      PlatformKeyboardEvent(PlatformEvent::Type::RawKeyDown, u8"", u8"", u8"ArrowDown", u8"ArrowDown",
                            vk::Down, false, false, false, {}, {}));
    CHECK(fixture.Document->FocusedElement() == &second);
    CHECK(second.Checked());
    auto scrollY = fixture.Frame->View()->GetScrollPosition().Y();
    CHECK(scrollY > 0);
    auto bottom = second.BoundingClientRect().MaxY();
    CHECK(bottom <= static_cast<float>(fixture.Frame->View()->VisibleHeight()));
  }
}
