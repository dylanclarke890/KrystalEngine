#include "Krystal.Booey/Page/History.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/LoadTestUtils.hpp"
#include "Krystal.Booey/DOM/Events/EventListenerHandle.hpp"
#include "Krystal.Booey/History/BackForwardController.hpp"
#include "Krystal.Booey/History/HistoryItem.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/HTML/Events/PopStateEvent.hpp"
#include "Krystal.Booey/Loader/HistoryController.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/Location.hpp"

namespace krys::boo::tests
{
  namespace
  {
    constexpr utf8_stringview PageA = u8"https://example.com/a.html";
    constexpr utf8_stringview PageB = u8"https://example.com/b.html";

    /// @brief A state that's a name.
    class NamedState final : public html::SerializedScriptValue
    {
    public:
      std::string Name;

      explicit NamedState(std::string name) noexcept : Name(krys::move(name))
      {
      }
    };

    KRYS_NODISCARD RefPtr<html::SerializedScriptValue> State(std::string name) noexcept
    {
      return CreateRef<NamedState>(krys::move(name));
    }

    KRYS_NODISCARD std::string NameOf(const html::SerializedScriptValue *state) noexcept
    {
      auto *named = dynamic_cast<const NamedState *>(state);
      return named ? named->Name : "null";
    }

    struct HistoryAPIFixture : LoadFixture
    {
      List<std::string> PopStates;
      List<dom::EventListenerHandle> Handles;

      HistoryAPIFixture() noexcept
      {
        Strategy.AddText(PageA, u8"<!DOCTYPE html><p id=a>A", u8"text/html");
        Strategy.AddText(PageB, u8"<!DOCTYPE html><p id=b>B", u8"text/html");
        Visit(PageA);
        ListenForPopState();
      }

      void Visit(utf8_stringview url) noexcept
      {
        LoadURL(url);
        Run();
      }

      static void Run() noexcept
      {
        html::EventLoop::Current().RunUntilIdle();
      }

      void ListenForPopState() noexcept
      {
        Handles.push_back(Document().Window()->AddEventListener(
          u8"popstate",
          [this](dom::Event &event)
          {
            auto &popState = Downcast<html::PopStateEvent>(event);
            CHECK(popState.IsTrusted());
            CHECK(popState.GetHistory() == &Document().Window()->GetHistory());
            PopStates.push_back(NameOf(popState.State()));
          }));
      }

      KRYS_NODISCARD History &GetHistory() const noexcept
      {
        return Document().Window()->GetHistory();
      }

      KRYS_NODISCARD std::string CurrentState() noexcept
      {
        auto state = GetHistory().State();
        REQUIRE_FALSE(state.HasException());
        return NameOf(state.ReleaseValue());
      }

      KRYS_NODISCARD uint32 Length() const noexcept
      {
        auto length = GetHistory().Length();
        REQUIRE_FALSE(length.HasException());
        return length.ReleaseValue();
      }
    };
  }

  TEST_CASE("pushState() and replaceState() add and replace the document's entries",
            "[Page][History][html:dom-history-pushstate][html:dom-history-replacestate]"
            "[html:shared-history-push/replace-state-steps][html:can-have-its-url-rewritten]"
            "[html:url-and-history-update-steps][html:dom-history-length][html:dom-history-state]")
  {
    HistoryAPIFixture fixture;
    auto &history = fixture.GetHistory();
    RefPtr<dom::Document> document = ShareRefPtr(&fixture.Document());
    CHECK(fixture.Length() == 1);
    CHECK(fixture.CurrentState() == "null");

    REQUIRE_FALSE(history.PushState(State("one"), u8"", u8"?step=1").HasException());
    CHECK(&fixture.Document() == document.get());
    CHECK(fixture.Document().GetURL().String() == u8"https://example.com/a.html?step=1");
    CHECK(fixture.Length() == 2);
    CHECK(fixture.CurrentState() == "one");
    CHECK_FALSE(fixture.Frame().Loader().IsLoading());
    CHECK(fixture.TestPage->BackForward().CurrentItem()->DocumentSequenceNumber()
          == fixture.TestPage->BackForward().BackItem()->DocumentSequenceNumber());

    REQUIRE_FALSE(history.ReplaceState(State("two"), u8"", u8"#here").HasException());
    CHECK(fixture.Document().GetURL().String() == u8"https://example.com/a.html?step=1#here");
    CHECK(fixture.Length() == 2);
    CHECK(fixture.CurrentState() == "two");

    // Neither fires popstate.
    fixture.Run();
    CHECK(fixture.PopStates.empty());

    SECTION("an empty URL keeps the document's")
    {
      REQUIRE_FALSE(history.PushState(State("three"), u8"").HasException());
      CHECK(fixture.Document().GetURL().String() == u8"https://example.com/a.html?step=1#here");
      CHECK(fixture.Length() == 3);
    }

    SECTION("a URL the document can't have is a SecurityError")
    {
      auto otherOrigin = history.PushState(State("x"), u8"", u8"https://other.example/a.html");
      REQUIRE(otherOrigin.HasException());
      CHECK(otherOrigin.ReleaseException().Code() == dom::ExceptionCode::SecurityError);
      CHECK(history.ReplaceState(State("x"), u8"", u8"http://example.com/a.html").HasException());
      CHECK(fixture.Length() == 2);
    }

    SECTION("a URL with another scheme can change only its fragment")
    {
      LoadFixture other;
      other.LoadMarkup(u8"<!DOCTYPE html>", u8"app://game/menu.html?tab=1");
      html::EventLoop::Current().RunUntilIdle();
      auto &otherHistory = other.Document().Window()->GetHistory();
      CHECK(otherHistory.PushState(nullptr, u8"", u8"?tab=2").HasException());
      CHECK(otherHistory.PushState(nullptr, u8"", u8"options.html?tab=1").HasException());
      REQUIRE_FALSE(otherHistory.PushState(nullptr, u8"", u8"#audio").HasException());
      CHECK(other.Document().GetURL().String() == u8"app://game/menu.html?tab=1#audio");
    }

    SECTION("too many states in a short time are a SecurityError")
    {
      dom::ExceptionOr<void> result;
      for (int32 i = 0; i < 101 && !result.HasException(); ++i)
      {
        result = history.ReplaceState(nullptr, u8"");
      }
      REQUIRE(result.HasException());
      CHECK(result.ReleaseException().Code() == dom::ExceptionCode::SecurityError);
    }
  }

  TEST_CASE("Going back and forward between pushed states fires popstate",
            "[Page][History][html:dom-history-go][html:dom-history-back][html:dom-history-forward]"
            "[html:delta-traverse][html:the-popstateevent-interface]")
  {
    HistoryAPIFixture fixture;
    auto &history = fixture.GetHistory();
    RefPtr<dom::Document> document = ShareRefPtr(&fixture.Document());
    REQUIRE_FALSE(history.PushState(State("one"), u8"", u8"/one").HasException());
    REQUIRE_FALSE(history.PushState(State("two"), u8"", u8"/two").HasException());

    // A history navigation happens on a later task.
    REQUIRE_FALSE(history.Back().HasException());
    CHECK(fixture.PopStates.empty());
    fixture.Run();
    CHECK(fixture.PopStates == List<std::string> {"one"});
    CHECK(fixture.Document().GetURL().String() == u8"https://example.com/one");
    CHECK(fixture.CurrentState() == "one");
    CHECK(&fixture.Document() == document.get());

    REQUIRE_FALSE(history.Go(-1).HasException());
    fixture.Run();
    CHECK(fixture.PopStates == List<std::string> {"one", "null"});
    CHECK(fixture.Document().GetURL().String() == PageA);

    REQUIRE_FALSE(history.Go(2).HasException());
    fixture.Run();
    CHECK(fixture.PopStates == List<std::string> {"one", "null", "two"});
    CHECK(fixture.Document().GetURL().String() == u8"https://example.com/two");

    SECTION("going past the list does nothing")
    {
      REQUIRE_FALSE(history.Forward().HasException());
      fixture.Run();
      CHECK(fixture.PopStates.size() == 3);
    }

    SECTION("a fragment navigation fires popstate too")
    {
      fixture.Document().Window()->GetLocation().Hash(u8"part");
      CHECK(fixture.PopStates == List<std::string> {"one", "null", "two", "null"});
    }
  }

  TEST_CASE(
    "History's other members",
    "[Page][History][html:the-history-interface][html:dom-history][html:dom-history-scroll-restoration]"
    "[html:dom-history-go]")
  {
    HistoryAPIFixture fixture;
    auto &history = fixture.GetHistory();

    SECTION("go() to another document loads it")
    {
      fixture.Visit(PageB);
      REQUIRE_FALSE(fixture.Document().Window()->GetHistory().Back().HasException());
      fixture.Run();
      CHECK(fixture.Document().GetElementById(u8"a"));
    }

    SECTION("scrollRestoration is the current entry's")
    {
      CHECK(history.GetScrollRestoration().ReleaseValue() == History::ScrollRestoration::Auto);
      REQUIRE_FALSE(history.SetScrollRestoration(History::ScrollRestoration::Manual).HasException());
      CHECK(history.GetScrollRestoration().ReleaseValue() == History::ScrollRestoration::Manual);
      CHECK_FALSE(fixture.Frame().Loader().History().CurrentItem()->ShouldRestoreScrollPosition());
    }

    SECTION("a document that isn't fully active can't use it")
    {
      Ref<History> oldHistory = ShareRef(history);
      fixture.Visit(PageB);
      auto length = oldHistory->Length();
      REQUIRE(length.HasException());
      CHECK(length.ReleaseException().Code() == dom::ExceptionCode::SecurityError);
      CHECK(oldHistory->PushState(State("x"), u8"").HasException());
    }

    SECTION("go(0) reloads")
    {
      RefPtr<dom::Document> before = ShareRefPtr(&fixture.Document());
      REQUIRE_FALSE(history.Go().HasException());
      fixture.Run();
      CHECK(&fixture.Document() != before.get());
      CHECK(fixture.Document().GetElementById(u8"a"));
    }
  }
}
