#include "Krystal.Booey/Rendering/RenderMenuList.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Editing/EditorTestUtils.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/HTML/HTMLSelectElement.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Platform/PlatformKeyboardEvent.hpp"
#include "Krystal.Booey/Platform/PlatformMouseEvent.hpp"
#include "Krystal.Booey/Platform/PopupMenu.hpp"
#include "Krystal.Booey/Platform/PopupMenuClient.hpp"

namespace krys::boo::layout::tests
{
  using boo::tests::EditorFixture;

  namespace
  {
    /// @brief A popup menu that records how it's shown.
    struct RecordingPopupMenu final : PopupMenu
    {
      PopupMenuClient *Client {nullptr};
      IntRect ShownAt;
      int32 ShownIndex {-1};
      bool Visible {false};
      int32 Updates {0};

      explicit RecordingPopupMenu(PopupMenuClient &client) noexcept : Client(&client)
      {
      }

      void Show(const IntRect &rect, LocalFrameView &, int32 selectedIndex) noexcept final
      {
        ShownAt = rect;
        ShownIndex = selectedIndex;
        Visible = true;
      }

      void Hide() noexcept final
      {
        if (!Visible)
        {
          return;
        }
        Visible = false;
        if (Client)
        {
          Client->PopupDidHide();
        }
      }

      void UpdateFromElement() noexcept final
      {
        ++Updates;
      }

      void DisconnectClient() noexcept final
      {
        Client = nullptr;
      }
    };

    /// @brief A chrome client whose popup menus record how they're shown.
    struct PopupChromeClient final : ChromeClient
    {
      mutable RefPtr<RecordingPopupMenu> Popup;

      void AttachRootGraphicsLayer(LocalFrame &, GraphicsLayer *) noexcept final
      {
      }

      KRYS_NODISCARD RefPtr<PopupMenu> CreatePopupMenu(PopupMenuClient &client) const noexcept final
      {
        Popup = CreateRef<RecordingPopupMenu>(client);
        return Popup;
      }
    };

    KRYS_NODISCARD PlatformMouseEvent MouseEvent(PlatformEvent::Type type, IntPoint position) noexcept
    {
      DoublePoint point(position.X(), position.Y());
      int32 clickCount = type == PlatformEvent::Type::MousePressed ? 1 : 0;
      return PlatformMouseEvent(point, point, MouseButton::Left, type, clickCount, {}, MonotonicTime::Now());
    }

    bool PressKey(EditorFixture &fixture, utf8_string key, utf8_string identifier, int32 keyCode) noexcept
    {
      return fixture.Frame->GetEventHandler().KeyEvent(
        PlatformKeyboardEvent(PlatformEvent::Type::RawKeyDown, u8"", u8"", key, key, krys::move(identifier),
                              keyCode, false, false, false, {}, {}));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderMenuList: the selected option's text",
                   "[Rendering][RenderMenuList]")
  {
    auto chromeClient = CreateUnique<PopupChromeClient>();
    EditorFixture fixture(u8"<style>select{margin:0}</style>"
                          u8"<select id=s><option>Apple</option><option id=b>Banana split</option></select>",
                          krys::move(chromeClient));

    auto &select = Downcast<html::HTMLSelectElement>(fixture.ById(u8"s"));
    auto &menuList = Downcast<RenderMenuList>(*select.Renderer());
    CHECK(menuList.Text() == u8"Apple");

    // The menu list is as wide as its widest option, and changes its text as the selection changes.
    auto width = menuList.Width();
    select.SelectedIndex(1);
    (void)fixture.Document->UpdateLayout();
    CHECK(menuList.Text() == u8"Banana split");
    CHECK(menuList.Width() == width);
    // The text sits in an anonymous block, padded on the right for the theme's arrow.
    CHECK(DumpWithIds(fixture) == u8R"(layer at (0,0) size 400x300
  RenderView at (0,0) size 400x300
layer at (0,0) size 400x23
  RenderBlock {HTML} at (0,0) size 400x23
    RenderBody {BODY} at (0,0) size 400x23
      RenderMenuList {SELECT} at (0,0) size 84x23 [bgcolor=#FFFFFF] [border: (1px solid #000000)] id="s"
        RenderBlock (anonymous) at (1,1) size 82x21
          RenderText at (5,5) size 56x11
            text run at (5,5) width 56: "Banana split"
)");

    // An empty select still has a line.
    select.SelectedIndex(-1);
    CHECK(menuList.Text() == u8"\n");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderMenuList: the popup", "[Rendering][RenderMenuList]")
  {
    auto chromeClient = CreateUnique<PopupChromeClient>();
    auto &client = *chromeClient;
    EditorFixture fixture(u8"<style>body{margin:0}select{margin:0}</style>"
                          u8"<select id=s><option>Apple</option><option disabled>Banana</option>"
                          u8"<optgroup label=Group><option>Cherry</option></optgroup><hr></select>",
                          krys::move(chromeClient));

    auto &select = Downcast<html::HTMLSelectElement>(fixture.ById(u8"s"));
    auto &menuList = Downcast<RenderMenuList>(*select.Renderer());
    auto &handler = fixture.Frame->GetEventHandler();
    auto inputs = fixture.Listen(EventNames::Input);
    auto changes = fixture.Listen(EventNames::Change);

    // Pressing the menu list shows the popup below it, at the selected item.
    (void)handler.HandleMousePressEvent(MouseEvent(PlatformEvent::Type::MousePressed, {5, 5}));
    REQUIRE(client.Popup);
    auto &popup = *client.Popup;
    CHECK(popup.Visible);
    CHECK(menuList.PopupIsVisible());
    CHECK(popup.ShownIndex == 0);
    CHECK(popup.ShownAt == IntRect(0, 0, menuList.Width().ToInt(), menuList.Height().ToInt()));
    (void)handler.HandleMouseReleaseEvent(MouseEvent(PlatformEvent::Type::MouseReleased, {5, 5}));

    // The popup asks the menu list for its items.
    auto &popupClient = *popup.Client;
    CHECK(popupClient.ListSize() == 5);
    CHECK(popupClient.ItemText(0) == u8"Apple");
    CHECK_FALSE(popupClient.ItemIsEnabled(1));
    CHECK(popupClient.ItemIsLabel(2));
    CHECK(popupClient.ItemText(2) == u8"Group");
    CHECK(popupClient.ItemText(3) == u8"    Cherry");
    CHECK(popupClient.ItemIsSeparator(4));
    CHECK(popupClient.ItemIsSelected(0));

    // Choosing an item hides the popup and selects its option, firing input and change.
    popup.Hide();
    popupClient.ValueChanged(3);
    CHECK(select.SelectedIndex() == 2);
    CHECK(menuList.Text() == u8"Cherry");
    CHECK(inputs->Count == 1);
    CHECK(changes->Count == 1);
    CHECK_FALSE(menuList.PopupIsVisible());

    // Space shows the popup too; losing the focus hides it.
    CHECK(fixture.Document->FocusedElement() == &select);
    (void)fixture.Frame->GetEventHandler().KeyEvent(PlatformKeyboardEvent(PlatformEvent::Type::Char, u8" ",
                                                                          u8" ", u8" ", u8"Space", u8"U+0020",
                                                                          ' ', false, false, false, {}, {}));
    CHECK(menuList.PopupIsVisible());
    select.Blur();
    CHECK_FALSE(menuList.PopupIsVisible());

    // The arrow keys change the selection without the popup.
    CHECK_FALSE(PressKey(fixture, u8"ArrowUp", u8"Up", vk::Up));
    CHECK(select.SelectedIndex() == 2);
    select.Focus();
    CHECK(PressKey(fixture, u8"ArrowUp", u8"Up", vk::Up));
    CHECK(select.SelectedIndex() == 0);
    CHECK(changes->Count == 2);
  }
}
