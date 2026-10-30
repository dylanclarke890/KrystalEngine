#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/NavigationTestUtils.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/PopoverData.hpp"
#include "Krystal.Booey/DOM/ToggleEvent.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLButtonElement.hpp"
#include "Krystal.Booey/HTML/HTMLDialogElement.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Platform/PlatformKeyboardEvent.hpp"
#include "Krystal.Booey/Platform/PlatformMouseEvent.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::html::tests
{
  using boo::tests::NavigationFixture;

  namespace
  {
    KRYS_NODISCARD std::string Narrow(dom::DOMStringView string)
    {
      return std::string(reinterpret_cast<const char *>(string.data()), string.size());
    }

    KRYS_NODISCARD std::string NarrowOrNull(const Maybe<dom::DOMString> &string)
    {
      return string ? Narrow(dom::DOMStringView(*string)) : std::string("null");
    }

    /// @brief Records a popover's toggle events, as `"<id>:<type>:<old>><new>"`.
    struct PopoverEventLog
    {
      List<std::string> Events;
      List<dom::EventListenerHandle> Handles;
      bool PreventBeforeToggle {false};

      void Watch(HTMLElement &popover) noexcept
      {
        auto id = NarrowOrNull(popover.GetAttribute(u8"id"));
        for (auto type : {u8"beforetoggle", u8"toggle"})
        {
          Handles.push_back(popover.AddEventListener(
            type,
            [this, id](dom::Event &event)
            {
              auto &toggle = Downcast<dom::ToggleEvent>(event);
              Events.push_back(id + ":" + Narrow(event.Type().View()) + ":" + Narrow(toggle.OldState()) + ">"
                               + Narrow(toggle.NewState()));
              if (PreventBeforeToggle && event.Type() == u8"beforetoggle")
              {
                event.PreventDefault();
              }
            }));
        }
      }

      /// @brief The events so far, after running the queued tasks (toggle is queued).
      KRYS_NODISCARD List<std::string> Take() noexcept
      {
        html::EventLoop::Current().RunUntilIdle();
        return std::exchange(Events, {});
      }
    };

    void PressKey(LocalFrame &frame, utf8_string key) noexcept
    {
      (void)frame.GetEventHandler().KeyEvent(PlatformKeyboardEvent(PlatformEvent::Type::KeyDown, u8"", u8"",
                                                                   krys::move(key), u8"", u8"", 0, false,
                                                                   false, false, {}, MonotonicTime::Now()));
    }

    void Click(LocalFrame &frame, double x, double y) noexcept
    {
      auto &handler = frame.GetEventHandler();
      (void)handler.HandleMousePressEvent(
        PlatformMouseEvent(DoublePoint(x, y), DoublePoint(x, y), MouseButton::Left,
                           PlatformEvent::Type::MousePressed, 1, {}, MonotonicTime::Now()));
      (void)handler.HandleMouseReleaseEvent(
        PlatformMouseEvent(DoublePoint(x, y), DoublePoint(x, y), MouseButton::Left,
                           PlatformEvent::Type::MouseReleased, 1, {}, MonotonicTime::Now()));
    }

    KRYS_NODISCARD dom::ExceptionCode ShowError(HTMLElement &element) noexcept
    {
      auto result = element.ShowPopover();
      return result.HasException() ? result.GetException().Code() : dom::ExceptionCode {};
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Popover: showing and hiding", "[HTML][Popover]")
  {
    NavigationFixture fixture(u8"<div id=p popover><input id=field autofocus></div><div id=plain></div>"
                              u8"<button id=before>B</button>");
    auto &document = *fixture.Document;
    auto &popover = Downcast<HTMLElement>(fixture.ById(u8"p"));
    auto &plain = Downcast<HTMLElement>(fixture.ById(u8"plain"));
    PopoverEventLog log;
    log.Watch(popover);

    SECTION("popover reflects its known values")
    {
      CHECK(NarrowOrNull(plain.Popover()) == "null");
      CHECK(NarrowOrNull(popover.Popover()) == "auto");
      popover.Popover(dom::DOMString(u8"HINT"));
      CHECK(NarrowOrNull(popover.Popover()) == "hint");
      CHECK(popover.GetPopoverState() == dom::PopoverState::Hint);
      popover.Popover(dom::DOMString(u8"nope"));
      CHECK(NarrowOrNull(popover.Popover()) == "manual");
      CHECK(popover.GetPopoverState() == dom::PopoverState::Manual);
      popover.Popover(null);
      CHECK_FALSE(popover.HasAttribute(u8"popover"));
      CHECK(popover.GetPopoverState() == dom::PopoverState::None);
      CHECK_FALSE(popover.GetPopoverData());
      popover.Popover(dom::DOMString(u8"auto"));
    }

    SECTION("Only connected popovers show")
    {
      CHECK(ShowError(plain) == dom::ExceptionCode::NotSupportedError);
      auto detached = document.CreateElement(u8"div").ReleaseValue();
      (void)detached->SetAttribute(dom::DOMStringAtom(u8"popover"), dom::DOMString());
      CHECK(ShowError(Downcast<HTMLElement>(*detached)) == dom::ExceptionCode::InvalidStateError);
      CHECK(log.Take().empty());
    }

    Downcast<HTMLElement>(fixture.ById(u8"before")).Focus();
    REQUIRE_FALSE(popover.ShowPopover().HasException());
    (void)document.UpdateLayout();
    CHECK(popover.IsPopoverShowing());
    CHECK(popover.IsInTopLayer());
    CHECK(popover.Matches(u8":popover-open").ReleaseValue());
    CHECK(document.TopmostAutoOrHintPopover() == &popover);
    REQUIRE(popover.GetPopoverData());
    CHECK(popover.GetPopoverData()->CloseWatcher());
    REQUIRE(popover.Renderer());
    // The popover focusing steps focus its autofocus delegate.
    CHECK(document.FocusedElement() == &fixture.ById(u8"field"));
    CHECK(log.Take() == List<std::string> {"p:beforetoggle:closed>open", "p:toggle:closed>open"});

    SECTION("Showing it again does nothing")
    {
      REQUIRE_FALSE(popover.ShowPopover().HasException());
      CHECK(log.Take().empty());
    }

    SECTION("Hiding it returns the focus, and it leaves the top layer at the next rendering update")
    {
      REQUIRE_FALSE(popover.HidePopover().HasException());
      CHECK_FALSE(popover.IsPopoverShowing());
      CHECK_FALSE(popover.Matches(u8":popover-open").ReleaseValue());
      CHECK_FALSE(document.TopmostAutoOrHintPopover());
      CHECK_FALSE(popover.GetPopoverData()->CloseWatcher());
      CHECK(document.FocusedElement() == &fixture.ById(u8"before"));
      CHECK(log.Take() == List<std::string> {"p:beforetoggle:open>closed", "p:toggle:open>closed"});
      fixture.TestPage->UpdateRendering();
      CHECK_FALSE(popover.IsInTopLayer());
    }

    SECTION("togglePopover() toggles, unless forced")
    {
      CHECK_FALSE(popover.TogglePopover().ReleaseValue());
      CHECK(popover.TogglePopover(true).ReleaseValue());
      CHECK(popover.TogglePopover(TogglePopoverOptions {.Force = true}).ReleaseValue());
      CHECK_FALSE(popover.TogglePopover(false).ReleaseValue());
      CHECK(plain.TogglePopover(false).HasException());
    }

    SECTION("Canceling beforetoggle keeps it hidden; toggle events coalesce")
    {
      REQUIRE_FALSE(popover.HidePopover().HasException());
      REQUIRE_FALSE(popover.ShowPopover().HasException());
      REQUIRE_FALSE(popover.HidePopover().HasException());
      CHECK(log.Take()
            == List<std::string> {"p:beforetoggle:open>closed", "p:beforetoggle:closed>open",
                                  "p:beforetoggle:open>closed", "p:toggle:open>closed"});
      log.PreventBeforeToggle = true;
      REQUIRE_FALSE(popover.ShowPopover().HasException());
      CHECK_FALSE(popover.IsPopoverShowing());
    }

    SECTION("Changing its state or removing it hides it")
    {
      popover.Popover(dom::DOMString(u8"manual"));
      CHECK_FALSE(popover.IsPopoverShowing());
      CHECK(popover.GetPopoverState() == dom::PopoverState::Manual);
      CHECK(log.Take() == List<std::string> {"p:beforetoggle:open>closed", "p:toggle:open>closed"});

      REQUIRE_FALSE(popover.ShowPopover().HasException());
      (void)log.Take();
      Ref<HTMLElement> keep = ShareRef(popover);
      REQUIRE_FALSE(popover.Remove().HasException());
      CHECK_FALSE(popover.IsPopoverShowing());
      CHECK_FALSE(popover.IsInTopLayer());
      // Without events.
      CHECK(log.Take().empty());
    }

    SECTION("Escape hides it through its close watcher")
    {
      PressKey(*fixture.Frame, u8"Escape");
      CHECK_FALSE(popover.IsPopoverShowing());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Popover: stacks", "[HTML][Popover]")
  {
    NavigationFixture fixture(
      u8"<style>body { margin: 0 } [popover] { inset: auto; margin: 0; padding: 0; border: 0; width: 50px;"
      u8" height: 50px } #a { left: 0; top: 0 } #b { left: 100px; top: 0 } #c { left: 200px; top: 0 }"
      u8" #m { left: 300px; top: 0 }</style>"
      u8"<div id=a popover><div id=b popover></div></div><div id=c popover></div><div id=m "
      u8"popover=manual></div>"
      u8"<div id=h popover=hint></div><div id=h2 popover=hint></div>");
    auto &document = *fixture.Document;
    auto &a = Downcast<HTMLElement>(fixture.ById(u8"a"));
    auto &b = Downcast<HTMLElement>(fixture.ById(u8"b"));
    auto &c = Downcast<HTMLElement>(fixture.ById(u8"c"));
    auto &m = Downcast<HTMLElement>(fixture.ById(u8"m"));
    auto &h = Downcast<HTMLElement>(fixture.ById(u8"h"));
    auto &h2 = Downcast<HTMLElement>(fixture.ById(u8"h2"));

    REQUIRE_FALSE(m.ShowPopover().HasException());
    REQUIRE_FALSE(a.ShowPopover().HasException());
    REQUIRE_FALSE(b.ShowPopover().HasException());

    SECTION("A nested popover keeps its ancestors; another hides the auto popovers but not manual ones")
    {
      CHECK(a.IsPopoverShowing());
      auto autoPopovers = document.ShowingAutoPopoverList();
      REQUIRE(autoPopovers.size() == 2);
      CHECK(autoPopovers[0].get() == &a);
      CHECK(autoPopovers[1].get() == &b);
      CHECK(b.TopmostPopoverAncestor(nullptr, false).get() == &a);

      REQUIRE_FALSE(c.ShowPopover().HasException());
      CHECK_FALSE(a.IsPopoverShowing());
      CHECK_FALSE(b.IsPopoverShowing());
      CHECK(c.IsPopoverShowing());
      CHECK(m.IsPopoverShowing());
    }

    SECTION("Hiding a popover hides the popovers above it")
    {
      REQUIRE_FALSE(a.HidePopover().HasException());
      CHECK_FALSE(b.IsPopoverShowing());
      CHECK(m.IsPopoverShowing());
    }

    SECTION("A popover shown from inside another is nested in it")
    {
      REQUIRE_FALSE(b.HidePopover().HasException());
      REQUIRE_FALSE(c.ShowPopover({.Source = ShareRefPtr(&b)}).HasException());
      // b isn't showing, but it's in a, so c is nested in a.
      CHECK(a.IsPopoverShowing());
      REQUIRE_FALSE(c.HidePopover().HasException());

      REQUIRE_FALSE(b.ShowPopover().HasException());
      REQUIRE_FALSE(c.ShowPopover({.Source = ShareRefPtr(&b)}).HasException());
      CHECK(a.IsPopoverShowing());
      CHECK(b.IsPopoverShowing());
      CHECK(c.GetPopoverData()->Invoker() == &b);
      CHECK(b.InvokedPopover() == &c);
    }

    SECTION("A click hides the popovers it's outside of")
    {
      (void)document.UpdateLayout();
      Click(*fixture.Frame, 120, 10);
      CHECK(a.IsPopoverShowing());
      CHECK(b.IsPopoverShowing());
      Click(*fixture.Frame, 10, 10);
      CHECK(a.IsPopoverShowing());
      CHECK_FALSE(b.IsPopoverShowing());
      Click(*fixture.Frame, 10, 200);
      CHECK_FALSE(a.IsPopoverShowing());
      CHECK(m.IsPopoverShowing());
    }

    SECTION("Hint popovers have their own stack")
    {
      REQUIRE_FALSE(h.ShowPopover().HasException());
      CHECK(a.IsPopoverShowing());
      CHECK(b.IsPopoverShowing());
      CHECK(document.TopmostAutoOrHintPopover() == &h);
      CHECK(h.GetPopoverData()->OpenedInPopoverMode() == dom::PopoverState::Hint);
      CHECK_FALSE(document.HintStackParent());

      // A hint popover hides the other hint popovers.
      REQUIRE_FALSE(h2.ShowPopover().HasException());
      CHECK_FALSE(h.IsPopoverShowing());

      // An auto popover hides the hint popovers.
      REQUIRE_FALSE(c.ShowPopover().HasException());
      CHECK_FALSE(h2.IsPopoverShowing());
    }

    SECTION("A hint popover opened from an auto popover hides with it")
    {
      REQUIRE_FALSE(h.ShowPopover({.Source = ShareRefPtr(&b)}).HasException());
      CHECK(document.HintStackParent() == &b);
      CHECK(b.IsPopoverShowing());

      // An auto popover opened from a hint popover is downgraded to a hint popover.
      REQUIRE_FALSE(c.ShowPopover({.Source = ShareRefPtr(&h)}).HasException());
      CHECK(c.GetPopoverData()->OpenedInPopoverMode() == dom::PopoverState::Hint);
      CHECK(h.IsPopoverShowing());
      CHECK(b.IsPopoverShowing());

      REQUIRE_FALSE(b.HidePopover().HasException());
      CHECK_FALSE(h.IsPopoverShowing());
      CHECK_FALSE(c.IsPopoverShowing());
      CHECK_FALSE(document.HintStackParent());
      CHECK(a.IsPopoverShowing());
    }

    SECTION("Showing a dialog hides the popovers it isn't in")
    {
      auto dialog = document.CreateElement(u8"dialog").ReleaseValue();
      REQUIRE_FALSE(b.AppendChild(*dialog).HasException());
      REQUIRE_FALSE(Downcast<HTMLDialogElement>(*dialog).Show().HasException());
      CHECK(b.IsPopoverShowing());
      Downcast<HTMLDialogElement>(*dialog).Close();

      REQUIRE_FALSE(c.AppendChild(*dialog).HasException());
      REQUIRE_FALSE(Downcast<HTMLDialogElement>(*dialog).ShowModal().HasException());
      CHECK_FALSE(a.IsPopoverShowing());
      CHECK(m.IsPopoverShowing());
    }

    SECTION("A modal dialog can't be a popover")
    {
      auto dialog = document.CreateElement(u8"dialog").ReleaseValue();
      (void)dialog->SetAttribute(dom::DOMStringAtom(u8"popover"), dom::DOMString());
      REQUIRE_FALSE(document.Body()->AppendChild(*dialog).HasException());
      auto &modal = Downcast<HTMLDialogElement>(*dialog);
      REQUIRE_FALSE(modal.ShowModal().HasException());
      CHECK(ShowError(modal) == dom::ExceptionCode::InvalidStateError);
      modal.Close();
      REQUIRE_FALSE(modal.ShowPopover().HasException());
      CHECK(modal.ShowModal().GetException().Code() == dom::ExceptionCode::InvalidStateError);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Popover: invokers", "[HTML][Popover]")
  {
    NavigationFixture fixture(
      u8"<button id=t popovertarget=p>T</button><button id=after>A</button>"
      u8"<div id=p popover><button id=inner>I</button><button id=hide popovertarget=p "
      u8"popovertargetaction=hide>H</button></div>"
      u8"<input id=i type=button popovertarget=p><button id=d disabled popovertarget=p></button>"
      u8"<form><button id=s popovertarget=p></button></form>");
    auto &document = *fixture.Document;
    auto &popover = Downcast<HTMLElement>(fixture.ById(u8"p"));
    auto &trigger = fixture.Get<HTMLButtonElement>(u8"t");

    SECTION("The popover target element is a button's, if it can invoke it")
    {
      CHECK(trigger.PopoverTargetElement().get() == &popover);
      CHECK(fixture.Get<HTMLInputElement>(u8"i").PopoverTargetElement().get() == &popover);
      CHECK_FALSE(fixture.Get<HTMLButtonElement>(u8"d").PopoverTargetElement());
      CHECK_FALSE(fixture.Get<HTMLButtonElement>(u8"s").PopoverTargetElement());

      CHECK(Narrow(trigger.PopoverTargetAction()) == "toggle");
      trigger.PopoverTargetAction(u8"SHOW");
      CHECK(Narrow(trigger.PopoverTargetAction()) == "show");

      // Setting the element sets the attribute to the empty string; it's still found when it moves.
      auto &after = fixture.Get<HTMLButtonElement>(u8"after");
      after.PopoverTargetElementForBindings(&popover);
      CHECK(NarrowOrNull(after.GetAttribute(u8"popovertarget")) == "");
      CHECK(after.PopoverTargetElement().get() == &popover);
      (void)after.SetAttribute(dom::DOMStringAtom(u8"popovertarget"), dom::DOMString(u8"nope"));
      CHECK_FALSE(after.PopoverTargetElement());
      after.PopoverTargetElementForBindings(nullptr);
      CHECK_FALSE(after.HasAttribute(u8"popovertarget"));
    }

    SECTION("Activating the button toggles the popover, which it's the trigger of")
    {
      trigger.Focus();
      trigger.Click();
      CHECK(popover.IsPopoverShowing());
      CHECK(popover.GetPopoverData()->Invoker() == &trigger);
      CHECK(trigger.InvokedPopover() == &popover);
      (void)document.UpdateStyleIfNeeded();
      REQUIRE(trigger.Renderer());
      CHECK(trigger.Renderer()->Style().IsPopoverInvoker());

      // The trigger's focus navigation scope holds the popover, which follows it in sequential navigation.
      CHECK(FocusAlgorithms::IsFocusNavigationScopeOwner(trigger));
      auto order = FocusAlgorithms::SequentialFocusNavigationOrder(document);
      auto position = [&](dom::DOMStringView id)
      {
        return std::ranges::find(order, &fixture.ById(id)) - order.begin();
      };
      CHECK(position(u8"inner") == position(u8"t") + 1);
      CHECK(position(u8"after") > position(u8"hide"));

      // Activations inside the popover don't toggle it, unless the button inside targets it.
      fixture.Get<HTMLButtonElement>(u8"inner").Click();
      CHECK(popover.IsPopoverShowing());
      fixture.Get<HTMLButtonElement>(u8"hide").Click();
      CHECK_FALSE(popover.IsPopoverShowing());
      CHECK_FALSE(trigger.InvokedPopover());
      CHECK_FALSE(FocusAlgorithms::IsFocusNavigationScopeOwner(trigger));

      fixture.Get<HTMLInputElement>(u8"i").Click();
      CHECK(popover.IsPopoverShowing());
      trigger.Click();
      CHECK_FALSE(popover.IsPopoverShowing());
    }

    SECTION("popovertargetaction=show only shows it")
    {
      trigger.PopoverTargetAction(u8"show");
      trigger.Click();
      trigger.Click();
      CHECK(popover.IsPopoverShowing());
    }
  }
}
