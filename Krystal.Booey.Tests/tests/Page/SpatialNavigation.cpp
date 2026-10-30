#include "Krystal.Booey/Page/SpatialNavigation.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/HTMLSelectElement.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Platform/PlatformKeyboardEvent.hpp"
#include "Krystal.Booey/Platform/WindowsKeyboardCodes.hpp"
#include <string>

namespace krys::boo::tests
{
  namespace
  {
    using Direction = dom::FocusDirection;

    /// @brief A laid out document in a 400x300 view.
    struct NavigationFixture : PaintFixture
    {
      explicit NavigationFixture(utf8_stringview markup) noexcept
          : PaintFixture(u8"<!DOCTYPE html><style>body{margin:0}</style>" + utf8_string(markup), {400, 300})
      {
        (void)Document->UpdateLayout();
      }

      bool Move(Direction direction) noexcept
      {
        return html::FocusAlgorithms::DirectionalNavigation(*Document, direction);
      }

      /// @brief The focused element's id, or "none".
      KRYS_NODISCARD std::string Focused() const noexcept
      {
        auto *element = Document->FocusedElement();
        if (!element)
        {
          return "none";
        }
        auto id = element->GetAttribute(u8"id").value_or(dom::DOMString());
        return std::string(id.begin(), id.end());
      }

      void Focus(utf8_stringview id) noexcept
      {
        Downcast<html::HTMLElement>(ById(id)).Focus();
      }

      bool Key(utf8_string key, utf8_string keyIdentifier, int32 keyCode,
               EnumSet<PlatformEvent::Modifier> modifiers = {}) noexcept
      {
        return Frame->GetEventHandler().KeyEvent(PlatformKeyboardEvent(
          PlatformEvent::Type::KeyDown, u8"", u8"", krys::move(key), u8"", krys::move(keyIdentifier), keyCode,
          false, false, false, modifiers, MonotonicTime::Now()));
      }
    };

    /// @brief Turns spatial navigation on while it lives (the settings are shared).
    struct SpatialNavigationOn
    {
      Settings &Value;

      explicit SpatialNavigationOn(Settings &settings) noexcept : Value(settings)
      {
        Value.SpatialNavigationEnabled = true;
      }

      ~SpatialNavigationOn() noexcept
      {
        Value.SpatialNavigationEnabled = false;
      }
    };

    /// @brief Absolutely positioned 40x40 buttons named by their ids, at the given positions.
    KRYS_NODISCARD utf8_string Buttons(std::initializer_list<std::tuple<const char *, int32, int32>> buttons)
    {
      std::string markup =
        "<style>button{position:absolute;width:40px;height:40px;padding:0;border:0}</style>";
      for (auto [id, x, y] : buttons)
      {
        markup += std::string("<button id=") + id + " style=left:" + std::to_string(x)
                  + "px;top:" + std::to_string(y) + "px></button>";
      }
      return utf8_string(markup.begin(), markup.end());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SpatialNavigation: the focus moves by direction",
                   "[Page][SpatialNavigation]")
  {
    SECTION("about a grid, from the edge of the view")
    {
      NavigationFixture fixture {Buttons(
        {{"a", 20, 20}, {"b", 100, 20}, {"c", 180, 20}, {"d", 20, 100}, {"e", 100, 100}, {"f", 180, 100}})};
      // With nothing focused, it starts from the edge it moves away from.
      CHECK(fixture.Move(Direction::Down));
      CHECK(fixture.Focused() == "a");
      CHECK(fixture.Document->FocusedElement()->HasFocusVisible());
      CHECK(fixture.Move(Direction::Right));
      CHECK(fixture.Focused() == "b");
      CHECK(fixture.Move(Direction::Down));
      CHECK(fixture.Focused() == "e");
      CHECK(fixture.Move(Direction::Right));
      CHECK(fixture.Focused() == "f");
      CHECK(fixture.Move(Direction::Left));
      CHECK(fixture.Focused() == "e");
      CHECK(fixture.Move(Direction::Up));
      CHECK(fixture.Focused() == "b");

      // At the edge there's nothing to move to, and the view can't scroll.
      CHECK_FALSE(fixture.Move(Direction::Up));
      CHECK(fixture.Focused() == "b");
    }

    SECTION("an aligned element wins over a nearer one that isn't")
    {
      // `far` is level with `start`; `near` is closer but entirely below it.
      NavigationFixture fixture {Buttons({{"start", 20, 20}, {"near", 80, 80}, {"far", 200, 20}})};
      fixture.Focus(u8"start");
      CHECK(fixture.Move(Direction::Right));
      CHECK(fixture.Focused() == "far");
    }

    SECTION("of elements that overlap, the one on top")
    {
      NavigationFixture fixture {Buttons({{"start", 20, 20}, {"under", 100, 20}, {"over", 110, 20}})
                                 + u8"<style>#over{z-index:1}</style>"};
      fixture.Focus(u8"start");
      CHECK(fixture.Move(Direction::Right));
      CHECK(fixture.Focused() == "over");
    }

    SECTION("an element out of view: the view scrolls a line at a time until it's in view")
    {
      NavigationFixture fixture {Buttons({{"top", 20, 20}, {"bottom", 20, 400}})
                                 + u8"<div style=height:1000px></div>"};
      auto &view = *fixture.Frame->View();
      fixture.Focus(u8"top");
      CHECK(fixture.Move(Direction::Down));
      CHECK(fixture.Focused() == "top");
      CHECK(view.GetScrollPosition() == ScrollPosition(0, 40));
      CHECK(fixture.Move(Direction::Down));
      CHECK(fixture.Focused() == "top");
      CHECK(view.GetScrollPosition() == ScrollPosition(0, 80));
      // Within a line of the view, it's focused (and scrolled into view).
      CHECK(fixture.Move(Direction::Down));
      CHECK(fixture.Focused() == "bottom");
      html::EventLoop::Current().RunUntilIdle();
      CHECK(view.GetScrollPosition().Y() > 80);
    }

    SECTION("nothing in the direction: the view scrolls")
    {
      NavigationFixture fixture {Buttons({{"only", 20, 20}}) + u8"<div style=height:1000px></div>"};
      fixture.Focus(u8"only");
      CHECK(fixture.Move(Direction::Down));
      CHECK(fixture.Frame->View()->GetScrollPosition() == ScrollPosition(0, 40));
      CHECK(fixture.Focused() == "only");
    }

    SECTION("into and out of a scrollable box")
    {
      NavigationFixture fixture {
        u8"<style>button{display:block;width:40px;height:40px;margin:0;padding:0;border:0}</style>"
        u8"<button id=above></button>"
        u8"<div id=box style=height:100px;width:100px;overflow:auto>"
        u8"<button id=first></button><div style=height:50px></div><button id=last></button></div>"
        u8"<button id=below></button>"};
      auto &box = fixture.ById(u8"box");
      fixture.Focus(u8"above");
      CHECK(fixture.Move(Direction::Down));
      CHECK(fixture.Focused() == "first");
      // `last` is partly in the box's visible part, so it's focused.
      CHECK(fixture.Move(Direction::Down));
      CHECK(fixture.Focused() == "last");
      // With nothing more below it in the box, the box scrolls (to its end), and then the focus leaves it.
      CHECK(fixture.Move(Direction::Down));
      CHECK(fixture.Focused() == "last");
      CHECK(box.ScrollTop() == 30);
      CHECK(fixture.Move(Direction::Down));
      CHECK(fixture.Focused() == "below");
      CHECK(fixture.Move(Direction::Up));
      CHECK(fixture.Focused() == "last");
    }

    SECTION("an image map's areas")
    {
      NavigationFixture fixture {u8"<img usemap=#m width=200 height=40 style=display:block;margin-top:20px>"
                                 u8"<map name=m><area id=left href=#a shape=rect coords=0,0,80,40>"
                                 u8"<area id=right href=#b shape=rect coords=120,0,200,40></map>"};
      CHECK(fixture.Move(Direction::Right));
      CHECK(fixture.Focused() == "left");
      CHECK(fixture.Move(Direction::Right));
      CHECK(fixture.Focused() == "right");
      CHECK(fixture.Move(Direction::Left));
      CHECK(fixture.Focused() == "left");
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SpatialNavigation: the arrow keys under the setting",
                   "[Page][SpatialNavigation]")
  {
    SECTION("they scroll without it, and move the focus with it")
    {
      NavigationFixture fixture {Buttons({{"a", 20, 20}, {"b", 20, 100}})
                                 + u8"<div style=height:1000px></div>"};
      fixture.Focus(u8"a");
      CHECK(fixture.Key(u8"ArrowDown", u8"Down", vk::Down));
      CHECK(fixture.Focused() == "a");
      CHECK(fixture.Frame->View()->GetScrollPosition() == ScrollPosition(0, 40));
      fixture.Frame->View()->SetScrollPosition({});

      SpatialNavigationOn on {fixture.Document->GetSettings()};
      CHECK(fixture.Key(u8"ArrowDown", u8"Down", vk::Down));
      CHECK(fixture.Focused() == "b");
      CHECK(fixture.Frame->View()->GetScrollPosition() == ScrollPosition());
      CHECK(fixture.Key(u8"ArrowUp", u8"Up", vk::Up));
      CHECK(fixture.Focused() == "a");
      // Not with modifiers.
      CHECK_FALSE(fixture.Key(u8"ArrowDown", u8"Down", vk::Down, {PlatformEvent::Modifier::ShiftKey}));
      CHECK(fixture.Focused() == "a");
    }

    SECTION("radios move the focus without checking")
    {
      NavigationFixture fixture {u8"<input type=radio name=r id=one checked style=display:block>"
                                 u8"<input type=radio name=r id=two style=display:block>"};
      fixture.Focus(u8"one");
      SpatialNavigationOn on {fixture.Document->GetSettings()};
      CHECK(fixture.Key(u8"ArrowDown", u8"Down", vk::Down));
      CHECK(fixture.Focused() == "two");
      CHECK(Downcast<html::HTMLInputElement>(fixture.ById(u8"one")).Checked());
      CHECK_FALSE(Downcast<html::HTMLInputElement>(fixture.ById(u8"two")).Checked());
    }

    SECTION("a list box changes its selection until it reaches its end, then the focus moves")
    {
      NavigationFixture fixture {u8"<select id=list size=3 style=display:block><option>A<option>B</select>"
                                 u8"<button id=after style=display:block;width:40px;height:40px></button>"};
      auto &list = Downcast<html::HTMLSelectElement>(fixture.ById(u8"list"));
      list.Focus();
      SpatialNavigationOn on {fixture.Document->GetSettings()};
      CHECK(fixture.Key(u8"ArrowDown", u8"Down", vk::Down));
      CHECK(list.SelectedIndex() == 0);
      CHECK(fixture.Key(u8"ArrowDown", u8"Down", vk::Down));
      CHECK(list.SelectedIndex() == 1);
      CHECK(fixture.Focused() == "list");
      // At the last option, Down moves the focus on.
      CHECK(fixture.Key(u8"ArrowDown", u8"Down", vk::Down));
      CHECK(fixture.Focused() == "after");
    }
  }
}
