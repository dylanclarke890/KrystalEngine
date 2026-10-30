#include "Krystal.Booey/Rendering/RenderListBox.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Editing/EditorTestUtils.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/HTMLCollection.hpp"
#include "Krystal.Booey/HTML/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/HTMLSelectElement.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Platform/PlatformKeyboardEvent.hpp"
#include "Krystal.Booey/Platform/PlatformMouseEvent.hpp"
#include "Krystal.Booey/Platform/PlatformWheelEvent.hpp"
#include "Krystal.Booey/Platform/Scrollbar.hpp"

namespace krys::boo::layout::tests
{
  using boo::tests::EditorFixture;

  namespace
  {
    KRYS_NODISCARD PlatformMouseEvent MouseEvent(PlatformEvent::Type type, IntPoint position,
                                                 EnumSet<PlatformEvent::Modifier> modifiers = {}) noexcept
    {
      DoublePoint point(position.X(), position.Y());
      int32 clickCount = type == PlatformEvent::Type::MousePressed ? 1 : 0;
      return PlatformMouseEvent(point, point, MouseButton::Left, type, clickCount, modifiers,
                                MonotonicTime::Now());
    }

    void Click(EventHandler &handler, IntPoint position,
               EnumSet<PlatformEvent::Modifier> modifiers = {}) noexcept
    {
      (void)handler.HandleMousePressEvent(MouseEvent(PlatformEvent::Type::MousePressed, position, modifiers));
      (void)handler.HandleMouseReleaseEvent(
        MouseEvent(PlatformEvent::Type::MouseReleased, position, modifiers));
    }

    bool PressKey(EditorFixture &fixture, utf8_string key, utf8_string identifier, int32 keyCode,
                  EnumSet<PlatformEvent::Modifier> modifiers = {}) noexcept
    {
      return fixture.Frame->GetEventHandler().KeyEvent(
        PlatformKeyboardEvent(PlatformEvent::Type::RawKeyDown, u8"", u8"", key, key, krys::move(identifier),
                              keyCode, false, false, false, modifiers, {}));
    }

    bool Wheel(EventHandler &handler, IntPoint position, float deltaY) noexcept
    {
      return handler.HandleWheelEvent(
        PlatformWheelEvent(position, position, 0, -deltaY, 0, deltaY > 0 ? -1.0f : 1.0f,
                           PlatformWheelEventGranularity::ScrollByPixelWheelEvent, {}, MonotonicTime::Now()));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderListBox: items and scrolling",
                   "[Rendering][RenderListBox]")
  {
    EditorFixture fixture(
      u8"<style>body{margin:0}select{margin:0}</style>"
      u8"<select id=s size=3><option>Apple</option><optgroup label=Group><option>Banana</option>"
      u8"<option>Cherry</option></optgroup><option>Date</option><option>Elderberry</option>"
      u8"</select>");

    auto &select = Downcast<html::HTMLSelectElement>(fixture.ById(u8"s"));
    auto &listBox = Downcast<RenderListBox>(*select.Renderer());
    auto &handler = fixture.Frame->GetEventHandler();

    // Three items tall, as wide as its widest item, with a scrollbar.
    CHECK(listBox.Size() == 3u);
    CHECK_FALSE(select.UsesMenuList());
    REQUIRE(listBox.VerticalScrollbar());
    CHECK(listBox.VerticalScrollbar()->Enabled());
    // The list box draws its items itself; only the optgroup has a (empty) renderer.
    CHECK(DumpWithIds(fixture) == u8R"(layer at (0,0) size 400x300
  RenderView at (0,0) size 400x300
layer at (0,0) size 400x44
  RenderBlock {HTML} at (0,0) size 400x44
    RenderBody {BODY} at (0,0) size 400x44
      RenderListBox {SELECT} at (0,0) size 74x37 [bgcolor=#FFFFFF] [border: (1px inset #808080)] id="s"
        RenderInline {OPTGROUP} at (0,0) size 0x0
)");

    // A list box doesn't select an option of its own.
    CHECK(select.SelectedIndex() == -1);

    // Items are found by their offset; the optgroup's label is an item too.
    auto item = listBox.ItemBoundingBoxRect({}, 1);
    CHECK(listBox.ListIndexAtOffset(LayoutSize(item.X() + 1_lu, item.Y() + 1_lu)) == 1);

    auto changes = fixture.Listen(EventNames::Change);

    SECTION("Clicking selects the item under the mouse")
    {
      auto banana = listBox.ItemBoundingBoxRect({}, 2);
      Click(handler, RoundedIntPoint(FloatPoint(banana.Center())));
      CHECK(select.SelectedIndex() == 1);
      CHECK(changes->Count == 1);

      // Clicking the optgroup's label selects nothing.
      auto group = listBox.ItemBoundingBoxRect({}, 1);
      Click(handler, RoundedIntPoint(FloatPoint(group.Center())));
      CHECK(select.SelectedIndex() == 1);
    }

    SECTION("The keys move the selection and scroll to it")
    {
      select.Focus();
      CHECK(PressKey(fixture, u8"ArrowDown", u8"Down", vk::Down));
      CHECK(select.SelectedIndex() == 0);
      CHECK(PressKey(fixture, u8"End", u8"End", vk::End));
      CHECK(select.SelectedIndex() == 4);
      CHECK(listBox.ListIndexIsVisible(5));
      CHECK_FALSE(listBox.ListIndexIsVisible(0));
      CHECK(listBox.GetScrollPosition() == ScrollPosition(0, 3));
      CHECK(changes->Count == 2);
    }

    SECTION("The wheel scrolls by items")
    {
      auto center = RoundedIntPoint(FloatPoint(listBox.ItemBoundingBoxRect({}, 1).Center()));
      CHECK(Wheel(handler, center, 100));
      CHECK(listBox.GetScrollPosition() == ScrollPosition(0, 3));
      CHECK(Wheel(handler, center, -100));
      CHECK(listBox.GetScrollPosition() == ScrollPosition(0, 0));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderListBox: multiple selection",
                   "[Rendering][RenderListBox]")
  {
    EditorFixture fixture(u8"<style>body{margin:0}select{margin:0}</style>"
                          u8"<select id=s multiple><option>A</option><option>B</option><option>C</option>"
                          u8"<option>D</option></select>");

    auto &select = Downcast<html::HTMLSelectElement>(fixture.ById(u8"s"));
    auto &listBox = Downcast<RenderListBox>(*select.Renderer());
    auto &handler = fixture.Frame->GetEventHandler();
    CHECK(listBox.Size() == 4u);
    CHECK_FALSE(listBox.VerticalScrollbar()->Enabled());

    auto at = [&](int32 index)
    {
      return RoundedIntPoint(FloatPoint(listBox.ItemBoundingBoxRect({}, index).Center()));
    };

    // A click selects one item; shift-click extends to another; ctrl-click toggles one.
    Click(handler, at(0));
    Click(handler, at(2), {PlatformEvent::Modifier::ShiftKey});
    CHECK(select.SelectedOptions()->Length() == 3uz);
    Click(handler, at(1), {PlatformEvent::Modifier::ControlKey});
    CHECK(select.SelectedOptions()->Length() == 2uz);
    CHECK_FALSE(select.Item(1u)->Selected());

    // Shift and the arrow keys extend the selection from the anchor.
    CHECK(PressKey(fixture, u8"ArrowDown", u8"Down", vk::Down, {PlatformEvent::Modifier::ShiftKey}));
    CHECK(select.SelectedOptions()->Length() == 3uz);
  }
  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "RenderListBox: items of both directions are drawn in visual order",
                   "[Rendering][RenderListBox]")
  {
    // "ab", then Hebrew shin and lamed: the Hebrew is drawn right to left, lamed first.
    boo::tests::PaintFixture fixture(u8"<!DOCTYPE html><select size=2><option>abשל</option></select>");
    CHECK(boo::tests::DrawnText(*fixture.PaintedDisplayList(), u8"של") == u8"לש");
  }
}
