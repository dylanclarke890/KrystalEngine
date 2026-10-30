#include "Krystal.Booey/HTML/HTMLDialogElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/NavigationTestUtils.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/ToggleEvent.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLButtonElement.hpp"
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

    /// @brief Records the dialog's events, as `"<type>"`, or `"<type>:<old>><new>"` for toggle events.
    struct DialogEventLog
    {
      List<std::string> Events;
      List<dom::EventListenerHandle> Handles;
      bool PreventCancel {false};
      bool PreventBeforeToggle {false};

      explicit DialogEventLog(HTMLDialogElement &dialog) noexcept
      {
        for (auto type : {u8"beforetoggle", u8"toggle"})
        {
          Handles.push_back(dialog.AddEventListener(
            type,
            [this](dom::Event &event)
            {
              auto &toggle = Downcast<dom::ToggleEvent>(event);
              Events.push_back(Narrow(event.Type().View()) + ":" + Narrow(toggle.OldState()) + ">"
                               + Narrow(toggle.NewState()));
              if (PreventBeforeToggle && event.Type() == u8"beforetoggle")
              {
                event.PreventDefault();
              }
            }));
        }
        Handles.push_back(dialog.AddEventListener(u8"cancel",
                                                  [this](dom::Event &event)
                                                  {
                                                    Events.push_back(event.Cancelable() ? "cancel!"
                                                                                        : "cancel");
                                                    if (PreventCancel)
                                                    {
                                                      event.PreventDefault();
                                                    }
                                                  }));
        Handles.push_back(
          dialog.AddEventListener(u8"close", [this](dom::Event &) { Events.push_back("close"); }));
      }

      /// @brief The events so far, after running the queued tasks (toggle and close are queued).
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
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLDialogElement: showing and closing",
                   "[HTML][HTMLDialogElement]")
  {
    NavigationFixture fixture(u8"<dialog id=d><form method=dialog><button id=ok value=yes>OK</button>"
                              u8"<button id=plain>X</button></form></dialog>");
    auto &document = *fixture.Document;
    auto &dialog = fixture.Get<HTMLDialogElement>(u8"d");
    DialogEventLog log(dialog);

    REQUIRE_FALSE(dialog.Show().HasException());
    CHECK(dialog.Open());
    CHECK_FALSE(dialog.IsModal());
    CHECK_FALSE(dialog.IsInTopLayer());
    CHECK_FALSE(dialog.Matches(u8":modal").ReleaseValue());
    CHECK(document.OpenDialogs().contains(ShareRef(dialog)));
    CHECK(dialog.GetCloseWatcher());
    CHECK(log.Take() == List<std::string> {"beforetoggle:closed>open", "toggle:closed>open"});

    SECTION("Showing an open dialog again does nothing; showing it modally throws")
    {
      REQUIRE_FALSE(dialog.Show().HasException());
      auto result = dialog.ShowModal();
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == dom::ExceptionCode::InvalidStateError);
      CHECK(log.Take().empty());
    }

    SECTION("Closing sets the return value if given, and fires close")
    {
      dialog.Close(dom::DOMString(u8"done"));
      CHECK_FALSE(dialog.Open());
      CHECK(Narrow(dialog.ReturnValue()) == "done");
      CHECK_FALSE(document.OpenDialogs().contains(ShareRef(dialog)));
      CHECK_FALSE(dialog.GetCloseWatcher());
      CHECK(log.Take() == List<std::string> {"beforetoggle:open>closed", "toggle:open>closed", "close"});

      REQUIRE_FALSE(dialog.Show().HasException());
      dialog.Close();
      CHECK(Narrow(dialog.ReturnValue()) == "done");
    }

    SECTION("Canceling beforetoggle keeps it closed")
    {
      dialog.Close();
      (void)log.Take();
      log.PreventBeforeToggle = true;
      REQUIRE_FALSE(dialog.Show().HasException());
      CHECK_FALSE(dialog.Open());
      CHECK(log.Take() == List<std::string> {"beforetoggle:closed>open"});
    }

    SECTION("A method=dialog form closes it with its submitter's value")
    {
      fixture.Get<HTMLButtonElement>(u8"plain").Click();
      html::EventLoop::Current().RunUntilIdle();
      CHECK_FALSE(dialog.Open());
      CHECK(Narrow(dialog.ReturnValue()).empty());

      REQUIRE_FALSE(dialog.Show().HasException());
      fixture.Get<HTMLButtonElement>(u8"ok").Click();
      html::EventLoop::Current().RunUntilIdle();
      CHECK_FALSE(dialog.Open());
      CHECK(Narrow(dialog.ReturnValue()) == "yes");
      CHECK(fixture.Navigations().empty());
    }

    SECTION("Setting the open attribute opens it without the events")
    {
      dialog.Open(false);
      CHECK_FALSE(document.OpenDialogs().contains(ShareRef(dialog)));
      CHECK_FALSE(dialog.GetCloseWatcher());
      dialog.Open(true);
      CHECK(document.OpenDialogs().contains(ShareRef(dialog)));
      CHECK(dialog.GetCloseWatcher());
      CHECK(log.Take().empty());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLDialogElement: modal dialogs", "[HTML][HTMLDialogElement]")
  {
    NavigationFixture fixture(u8"<button id=before>B</button><dialog id=d><p>Hi<input id=field></dialog>");
    auto &document = *fixture.Document;
    auto &dialog = fixture.Get<HTMLDialogElement>(u8"d");
    auto &before = Downcast<HTMLElement>(fixture.ById(u8"before"));
    auto &field = fixture.ById(u8"field");

    SECTION("It must be connected and closed")
    {
      auto detached = document.CreateElement(u8"dialog").ReleaseValue();
      auto result = Downcast<HTMLDialogElement>(*detached).ShowModal();
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == dom::ExceptionCode::InvalidStateError);
    }

    before.Focus();
    REQUIRE(document.FocusedElement() == &before);
    REQUIRE_FALSE(dialog.ShowModal().HasException());
    (void)document.UpdateLayout();

    CHECK(dialog.IsModal());
    CHECK(dialog.IsInTopLayer());
    CHECK(dialog.Matches(u8":modal").ReleaseValue());
    CHECK(document.ActiveModalDialog() == &dialog);
    // The dialog focusing steps focus its focus delegate.
    CHECK(document.FocusedElement() == &field);

    // The rest of the document is inert.
    CHECK(FocusAlgorithms::IsInert(before));
    CHECK_FALSE(FocusAlgorithms::IsInert(field));
    REQUIRE(before.Renderer());
    CHECK(before.Renderer()->Style().EffectiveInert());
    REQUIRE(field.Renderer());
    CHECK_FALSE(field.Renderer()->Style().EffectiveInert());
    before.Focus();
    CHECK(document.FocusedElement() == &field);

    SECTION("Closing it restores the focus and the document")
    {
      dialog.Close();
      CHECK_FALSE(dialog.IsModal());
      CHECK(document.FocusedElement() == &before);
      CHECK_FALSE(document.ActiveModalDialog());
      CHECK_FALSE(dialog.Matches(u8":modal").ReleaseValue());
      CHECK_FALSE(FocusAlgorithms::IsInert(before));

      // It leaves the top layer at the next rendering update.
      CHECK(document.IsPendingTopLayerRemoval(dialog));
      fixture.TestPage->UpdateRendering();
      CHECK_FALSE(dialog.IsInTopLayer());
      REQUIRE(before.Renderer());
      CHECK_FALSE(before.Renderer()->Style().EffectiveInert());
    }

    SECTION("Removing it closes it as a modal")
    {
      Ref<HTMLDialogElement> keep = ShareRef(dialog);
      REQUIRE_FALSE(dialog.Remove().HasException());
      CHECK_FALSE(dialog.IsModal());
      CHECK_FALSE(dialog.IsInTopLayer());
      CHECK_FALSE(dialog.GetCloseWatcher());
      CHECK(document.OpenDialogs().empty());
      CHECK_FALSE(document.ActiveModalDialog());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLDialogElement: close requests", "[HTML][HTMLDialogElement]")
  {
    NavigationFixture fixture(
      u8"<style>body { margin: 0 } #d { inset: auto; left: 100px; top: 100px; margin: 0;"
      u8" padding: 0; border: 0; width: 100px; height: 50px }</style><dialog id=d></dialog>");
    auto &dialog = fixture.Get<HTMLDialogElement>(u8"d");
    auto &frame = *fixture.Frame;
    DialogEventLog log(dialog);

    SECTION("closedby reflects its known values")
    {
      CHECK(Narrow(dialog.ClosedBy()).empty());
      dialog.ClosedBy(u8"ANY");
      CHECK(Narrow(dialog.ClosedBy()) == "any");
      dialog.ClosedBy(u8"nope");
      CHECK(Narrow(dialog.ClosedBy()).empty());
      CHECK(dialog.ComputedClosedByState() == HTMLDialogElement::ClosedByState::None);
    }

    SECTION("Escape cancels a modal dialog, which can't refuse without a user activation")
    {
      REQUIRE_FALSE(dialog.ShowModal().HasException());
      (void)log.Take();
      log.PreventCancel = true;
      PressKey(frame, u8"Escape");
      CHECK_FALSE(dialog.Open());
      CHECK(log.Take()
            == List<std::string> {"cancel", "beforetoggle:open>closed", "toggle:open>closed", "close"});
    }

    SECTION("requestClose() fires a cancelable cancel, then closes with its return value")
    {
      REQUIRE_FALSE(dialog.ShowModal().HasException());
      (void)log.Take();
      log.PreventCancel = true;
      dialog.RequestClose(dom::DOMString(u8"r"));
      CHECK(dialog.Open());
      log.PreventCancel = false;
      dialog.RequestClose(dom::DOMString(u8"r"));
      CHECK_FALSE(dialog.Open());
      CHECK(Narrow(dialog.ReturnValue()) == "r");
      CHECK(log.Take()
            == List<std::string> {"cancel!", "cancel!", "beforetoggle:open>closed", "toggle:open>closed",
                                  "close"});
    }

    SECTION("closedby=none ignores Escape, but not requestClose()")
    {
      dialog.ClosedBy(u8"none");
      REQUIRE_FALSE(dialog.ShowModal().HasException());
      PressKey(frame, u8"Escape");
      CHECK(dialog.Open());
      dialog.RequestClose();
      CHECK_FALSE(dialog.Open());
    }

    SECTION("A non-modal dialog takes close requests only with closedby")
    {
      REQUIRE_FALSE(dialog.Show().HasException());
      PressKey(frame, u8"Escape");
      CHECK(dialog.Open());
      dialog.ClosedBy(u8"closerequest");
      PressKey(frame, u8"Escape");
      CHECK_FALSE(dialog.Open());
    }

    SECTION("closedby=any closes it on a click outside it")
    {
      dialog.ClosedBy(u8"any");
      REQUIRE_FALSE(dialog.ShowModal().HasException());
      (void)fixture.Document->UpdateLayout();

      Click(frame, 120, 110);
      CHECK(dialog.Open());
      Click(frame, 10, 10);
      CHECK_FALSE(dialog.Open());
      // No task ran between the showing and the closing, so one toggle event reports both.
      CHECK(log.Take()
            == List<std::string> {"beforetoggle:closed>open", "cancel!", "beforetoggle:open>closed",
                                  "toggle:closed>closed", "close"});
    }

    SECTION("A click outside a dialog without closedby=any leaves it open")
    {
      REQUIRE_FALSE(dialog.ShowModal().HasException());
      (void)fixture.Document->UpdateLayout();
      Click(frame, 10, 10);
      CHECK(dialog.Open());
    }
  }
}
