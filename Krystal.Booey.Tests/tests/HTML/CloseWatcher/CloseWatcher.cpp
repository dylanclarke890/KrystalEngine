#include "Krystal.Booey/HTML/CloseWatcher/CloseWatcher.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/NavigationTestUtils.hpp"
#include "Krystal.Booey/DOM/AbortController.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/CloseWatcher/CloseWatcherManager.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Platform/PlatformKeyboardEvent.hpp"

namespace krys::boo::html::tests
{
  using boo::tests::NavigationFixture;

  namespace
  {
    /// @brief Records the `cancel` and `close` events of watchers, as `"<name>:<type>"`.
    struct EventLog
    {
      List<std::string> Events;
      List<dom::EventListenerHandle> Handles;
      bool PreventCancel {false};

      void Watch(CloseWatcher &watcher, std::string name) noexcept
      {
        Handles.push_back(watcher.AddEventListener(u8"cancel",
                                                   [this, name](dom::Event &event)
                                                   {
                                                     Events.push_back(name + ":cancel"
                                                                      + (event.Cancelable() ? "!" : ""));
                                                     if (PreventCancel)
                                                     {
                                                       event.PreventDefault();
                                                     }
                                                   }));
        Handles.push_back(watcher.AddEventListener(u8"close", [this, name](dom::Event &)
                                                   { Events.push_back(name + ":close"); }));
      }
    };

    bool PressKey(LocalFrame &frame, utf8_string key) noexcept
    {
      return frame.GetEventHandler().KeyEvent(PlatformKeyboardEvent(PlatformEvent::Type::KeyDown, u8"", u8"",
                                                                    krys::move(key), u8"", u8"", 0, false,
                                                                    false, false, {}, MonotonicTime::Now()));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "CloseWatcher: closing and destroying", "[HTML][CloseWatcher]")
  {
    NavigationFixture fixture(u8"<p>");
    auto &document = *fixture.Document;
    EventLog log;
    auto watcher = CloseWatcher::Create(document).ReleaseValue();
    log.Watch(*watcher, "a");
    CHECK(watcher->IsActive());

    SECTION("A close request from the page fires cancel, which it can always cancel, then close")
    {
      log.PreventCancel = true;
      watcher->RequestClose();
      CHECK(log.Events == List<std::string> {"a:cancel!"});
      CHECK(watcher->IsActive());
      log.PreventCancel = false;
      watcher->RequestClose();
      CHECK(log.Events == List<std::string> {"a:cancel!", "a:cancel!", "a:close"});
      CHECK_FALSE(watcher->IsActive());
      watcher->RequestClose();
      watcher->Close();
      CHECK(log.Events.size() == 3);
    }

    SECTION("Closing fires only close; destroying fires nothing")
    {
      watcher->Close();
      CHECK(log.Events == List<std::string> {"a:close"});
      auto other = CloseWatcher::Create(document).ReleaseValue();
      log.Watch(*other, "b");
      other->Destroy();
      other->RequestClose();
      CHECK(log.Events == List<std::string> {"a:close"});
    }

    SECTION("The event handler attributes")
    {
      bool closed = false;
      watcher->SetOnClose([&](dom::Event &) { closed = true; });
      REQUIRE(watcher->OnClose());
      watcher->RequestClose();
      CHECK(closed);
    }

    SECTION("A signal destroys it as it aborts")
    {
      dom::AbortController controller;
      auto signalled =
        CloseWatcher::Create(document, {.Signal = ShareRefPtr(controller.Signal())}).ReleaseValue();
      CHECK(signalled->IsActive());
      controller.Abort();
      CHECK_FALSE(signalled->IsActive());

      // An aborted signal gives an inactive watcher.
      auto aborted =
        CloseWatcher::Create(document, {.Signal = ShareRefPtr(controller.Signal())}).ReleaseValue();
      CHECK_FALSE(aborted->IsActive());
    }

    SECTION("A document that isn't fully active can't make one")
    {
      auto detached = CreateRef<dom::HTMLDocument>();
      auto result = CloseWatcher::Create(*detached);
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == dom::ExceptionCode::InvalidStateError);
    }

    SECTION("Stopping the document destroys its watchers")
    {
      document.StopActiveDOMObjects();
      CHECK_FALSE(watcher->IsActive());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "CloseWatcher: user activation and groups",
                   "[HTML][CloseWatcher]")
  {
    NavigationFixture fixture(u8"<p>");
    auto &document = *fixture.Document;
    auto &window = *document.Window();
    auto &frame = *fixture.Frame;
    EventLog log;

    SECTION("Without a user activation, watchers share a group, which Escape closes newest first")
    {
      auto a = CloseWatcher::Create(document).ReleaseValue();
      auto b = CloseWatcher::Create(document).ReleaseValue();
      log.Watch(*a, "a");
      log.Watch(*b, "b");
      CHECK_FALSE(PressKey(frame, u8"Escape"));
      CHECK(log.Events == List<std::string> {"b:cancel", "b:close", "a:cancel", "a:close"});
    }

    SECTION("A user activation allows a new group; Escape closes only the last")
    {
      auto a = CloseWatcher::Create(document).ReleaseValue();
      (void)PressKey(frame, u8"x");
      auto b = CloseWatcher::Create(document).ReleaseValue();
      log.Watch(*a, "a");
      log.Watch(*b, "b");
      (void)PressKey(frame, u8"Escape");
      CHECK(log.Events == List<std::string> {"b:cancel", "b:close"});
      CHECK(a->IsActive());
      (void)PressKey(frame, u8"Escape");
      CHECK(log.Events == List<std::string> {"b:cancel", "b:close", "a:cancel", "a:close"});
    }

    SECTION("With a group to spare and a history-action activation, Escape's cancel can be canceled once")
    {
      auto a = CloseWatcher::Create(document).ReleaseValue();
      log.Watch(*a, "a");
      log.PreventCancel = true;
      window.NotifyActivated();
      CHECK(window.GetCloseWatcherManager().CanPreventClose());
      CHECK(window.HasHistoryActionActivation());

      (void)PressKey(frame, u8"Escape");
      CHECK(log.Events == List<std::string> {"a:cancel!"});
      CHECK(a->IsActive());
      CHECK_FALSE(window.HasHistoryActionActivation());

      // The activation was consumed, so the next request closes it.
      (void)PressKey(frame, u8"Escape");
      CHECK(log.Events == List<std::string> {"a:cancel!", "a:cancel", "a:close"});
      CHECK_FALSE(a->IsActive());
    }

    SECTION("Requesting a close from cancel does nothing")
    {
      auto a = CloseWatcher::Create(document).ReleaseValue();
      auto handle = a->AddEventListener(u8"cancel", [&](dom::Event &) { a->RequestClose(); });
      log.Watch(*a, "a");
      a->RequestClose();
      CHECK(log.Events == List<std::string> {"a:cancel!", "a:close"});
    }

    SECTION("A disabled watcher is skipped by Escape")
    {
      bool enabled = false;
      auto a = CloseWatcher::Establish(
        document, [](bool) { return true; }, [&] { log.Events.push_back("a:closed"); },
        [&] { return enabled; });
      (void)PressKey(frame, u8"Escape");
      CHECK(log.Events.empty());
      CHECK(a->IsActive());
      enabled = true;
      (void)PressKey(frame, u8"Escape");
      CHECK(log.Events == List<std::string> {"a:closed"});
    }
  }
}
