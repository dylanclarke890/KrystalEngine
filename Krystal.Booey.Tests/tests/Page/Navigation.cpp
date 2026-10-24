#include "Krystal.Booey/Page/Navigation.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"
#include "Krystal.Booey.Tests/Loader/LoadTestUtils.hpp"
#include "Krystal.Booey/DOM/EventListenerHandle.hpp"
#include "Krystal.Booey/History/BackForwardCache.hpp"
#include "Krystal.Booey/HTML/ErrorEvent.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Page/History.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/Location.hpp"
#include "Krystal.Booey/Page/NavigationCurrentEntryChangeEvent.hpp"

namespace krys::boo::tests
{
  namespace
  {
    constexpr utf8_stringview PageA = u8"https://example.com/a.html";
    constexpr utf8_stringview PageB = u8"https://example.com/b.html";

    class NamedValue final : public html::SerializedScriptValue
    {
    public:
      std::string Name;

      explicit NamedValue(std::string name) noexcept : Name(krys::move(name))
      {
      }
    };

    KRYS_NODISCARD RefPtr<html::SerializedScriptValue> Value(std::string name) noexcept
    {
      return CreateRef<NamedValue>(krys::move(name));
    }

    KRYS_NODISCARD std::string NameOf(const html::SerializedScriptValue *value) noexcept
    {
      auto *named = dynamic_cast<const NamedValue *>(value);
      return named ? named->Name : "null";
    }

    KRYS_NODISCARD std::string TypeName(NavigationNavigationType type) noexcept
    {
      switch (type)
      {
        case NavigationNavigationType::Push:     return "push";
        case NavigationNavigationType::Replace:  return "replace";
        case NavigationNavigationType::Reload:   return "reload";
        case NavigationNavigationType::Traverse: return "traverse";
      }
      return "?";
    }

    struct NavigationFixture : LoadFixture
    {
      List<std::string> Log;
      List<dom::EventListenerHandle> Handles;
      /// @brief Called with each navigate event, to intercept or cancel it.
      Func<void(NavigateEvent &)> OnNavigate;

      NavigationFixture() noexcept
      {
        Strategy.AddText(PageA, u8"<!DOCTYPE html><p id=a>A", u8"text/html");
        Strategy.AddText(PageB, u8"<!DOCTYPE html><p id=b>B", u8"text/html");
        Visit(PageA);
      }

      void Visit(utf8_stringview url) noexcept
      {
        LoadURL(url);
        Run();
        Listen();
      }

      static void Run() noexcept
      {
        html::EventLoop::Current().RunUntilIdle();
      }

      KRYS_NODISCARD Navigation &GetNavigation() const noexcept
      {
        return Document().Window()->GetNavigation();
      }

      /// @brief Logs the navigation's events.
      void Listen() noexcept
      {
        Handles.clear();
        auto &navigation = GetNavigation();
        Handles.push_back(navigation.AddEventListener(
          u8"navigate",
          [this](dom::Event &event)
          {
            auto &navigate = Downcast<NavigateEvent>(event);
            Log.push_back("navigate " + TypeName(navigate.NavigationType()) + " "
                          + Narrow(navigate.Destination()->URL().String())
                          + (navigate.Destination()->SameDocument() ? " same" : "")
                          + (navigate.HashChange() ? " hash" : "")
                          + (navigate.CanIntercept() ? " interceptable" : "")
                          + (navigate.Cancelable() ? " cancelable" : ""));
            if (OnNavigate)
            {
              OnNavigate(navigate);
            }
          }));
        Handles.push_back(navigation.AddEventListener(
          u8"currententrychange",
          [this](dom::Event &event)
          {
            auto &change = Downcast<NavigationCurrentEntryChangeEvent>(event);
            Log.push_back("currententrychange "
                          + (change.NavigationType() ? TypeName(*change.NavigationType()) : "null"));
          }));
        Handles.push_back(navigation.AddEventListener(u8"navigatesuccess", [this](dom::Event &)
                                                      { Log.push_back("navigatesuccess"); }));
        Handles.push_back(navigation.AddEventListener(
          u8"navigateerror", [this](dom::Event &event)
          { Log.push_back("navigateerror " + Narrow(Downcast<html::ErrorEvent>(event).Message())); }));
      }

      /// @brief Logs how @p result's promises settle.
      void Track(NavigationResult &result) noexcept
      {
        result.Committed->Then([this](NavigationHistoryEntry *entry)
                               { Log.push_back(Settled("committed", entry)); },
                               [this](const dom::Exception &exception)
                               { Log.push_back("committed rejected " + Narrow(exception.Message())); });
        result.Finished->Then([this](NavigationHistoryEntry *entry)
                              { Log.push_back(Settled("finished", entry)); },
                              [this](const dom::Exception &exception)
                              { Log.push_back("finished rejected " + Narrow(exception.Message())); });
      }

      KRYS_NODISCARD static std::string Settled(std::string name, NavigationHistoryEntry *entry) noexcept
      {
        auto url = entry ? entry->URL() : null;
        return name + " " + (url ? Narrow(*url) : "none");
      }
    };
  }

  TEST_CASE("The Navigation API's entries", "[Page][Navigation]")
  {
    NavigationFixture fixture;
    auto *navigation = &fixture.GetNavigation();
    REQUIRE(navigation->Entries().size() == 1);
    auto *entryA = navigation->CurrentEntry();
    REQUIRE(entryA);
    CHECK(entryA->URL() == PageA);
    CHECK_FALSE(entryA->Key().empty());
    CHECK_FALSE(entryA->Id().empty());
    CHECK(entryA->Index() == 0);
    CHECK(entryA->SameDocument());
    CHECK_FALSE(navigation->CanGoBack());
    CHECK_FALSE(navigation->CanGoForward());
    // The first load activates the document too, with nothing to come from.
    REQUIRE(navigation->Activation());
    CHECK(navigation->Activation()->NavigationType() == NavigationNavigationType::Push);
    CHECK(navigation->Activation()->From() == nullptr);

    // The next document of the origin takes the entries over.
    auto keyA = entryA->Key();
    fixture.Visit(PageB);
    navigation = &fixture.GetNavigation();
    REQUIRE(navigation->Entries().size() == 2);
    CHECK(navigation->Entries()[0]->Key() == keyA);
    CHECK_FALSE(navigation->Entries()[0]->SameDocument());
    CHECK(navigation->CurrentEntry()->URL() == PageB);
    CHECK(navigation->CanGoBack());
    REQUIRE(navigation->Activation());
    CHECK(navigation->Activation()->NavigationType() == NavigationNavigationType::Push);
    CHECK(navigation->Activation()->From() == navigation->Entries()[0].get());
    CHECK(&navigation->Activation()->Entry() == navigation->CurrentEntry());

    SECTION("pushState() and replaceState() update them")
    {
      auto &history = fixture.Document().Window()->GetHistory();
      REQUIRE_FALSE(history.PushState(Value("pushed"), u8"", u8"/pushed").HasException());
      fixture.Run();
      CHECK(navigation->Entries().size() == 3);
      CHECK(navigation->CurrentEntry()->URL() == u8"https://example.com/pushed");

      bool disposed = false;
      auto disposeListener =
        navigation->CurrentEntry()->AddEventListener(u8"dispose", [&](dom::Event &) { disposed = true; });
      REQUIRE_FALSE(history.ReplaceState(nullptr, u8"", u8"/replaced").HasException());
      fixture.Run();
      CHECK(disposed);
      CHECK(navigation->Entries().size() == 3);
      CHECK(navigation->CurrentEntry()->URL() == u8"https://example.com/replaced");
      CHECK(
        fixture.Log
        == List<std::string> {"navigate push https://example.com/pushed same interceptable cancelable",
                              "currententrychange push", "navigatesuccess",
                              "navigate replace https://example.com/replaced same interceptable cancelable",
                              "currententrychange replace", "navigatesuccess"});
    }

    SECTION("updateCurrentEntry() gives the current entry a state")
    {
      REQUIRE_FALSE(navigation->UpdateCurrentEntry({Value("state")}).HasException());
      CHECK(NameOf(navigation->CurrentEntry()->GetState()) == "state");
      CHECK(fixture.Log == List<std::string> {"currententrychange null"});
    }
  }

  TEST_CASE("navigate fires for each navigation, and can cancel it", "[Page][Navigation]")
  {
    NavigationFixture fixture;
    RefPtr<dom::Document> document = ShareRefPtr(&fixture.Document());

    SECTION("a fragment")
    {
      fixture.Document().Window()->GetLocation().Hash(u8"part");
      CHECK(fixture.Log
            == List<std::string> {
              "navigate push https://example.com/a.html#part same hash interceptable cancelable",
              "currententrychange push"});
    }

    SECTION("another document")
    {
      REQUIRE_FALSE(fixture.Document().Window()->GetLocation().Assign(PageB).HasException());
      fixture.Run();
      CHECK(fixture.Log
            == List<std::string> {"navigate push https://example.com/b.html interceptable cancelable"});
      CHECK(fixture.Document().GetElementById(u8"b"));
    }

    SECTION("canceling it stops the navigation")
    {
      fixture.OnNavigate = [](NavigateEvent &event)
      {
        event.PreventDefault();
      };
      REQUIRE_FALSE(fixture.Document().Window()->GetLocation().Assign(PageB).HasException());
      fixture.Run();
      CHECK(&fixture.Document() == document.get());
      CHECK(fixture.Log
            == List<std::string> {"navigate push https://example.com/b.html interceptable cancelable",
                                  "navigateerror Navigation aborted"});
    }

    SECTION("a reload")
    {
      auto result = fixture.GetNavigation().Reload();
      fixture.Run();
      CHECK(fixture.Log.front() == "navigate reload https://example.com/a.html interceptable cancelable");
      CHECK(&fixture.Document() != document.get());
    }
  }

  TEST_CASE("Intercepted navigations commit without a load", "[Page][Navigation]")
  {
    NavigationFixture fixture;
    RefPtr<dom::Document> document = ShareRefPtr(&fixture.Document());
    Func<void(dom::ExceptionOr<void>)> finish;
    fixture.OnNavigate = [&](NavigateEvent &event)
    {
      NavigateEvent::NavigationInterceptOptions options;
      options.Handler = [&](Func<void(dom::ExceptionOr<void>)> &&completion)
      {
        fixture.Log.push_back("handler");
        finish = krys::move(completion);
      };
      REQUIRE_FALSE(event.Intercept(*document, krys::move(options)).HasException());
    };

    auto result = fixture.GetNavigation().Navigate(u8"/view", {{Value("info")}, Value("state")});
    fixture.Track(result);
    fixture.Run();

    CHECK(&fixture.Document() == document.get());
    CHECK(fixture.Document().GetURL().String() == u8"https://example.com/view");
    auto &navigation = fixture.GetNavigation();
    REQUIRE(navigation.Transition());
    CHECK(navigation.Transition()->From().URL() == PageA);
    CHECK(NameOf(navigation.CurrentEntry()->GetState()) == "state");
    CHECK(fixture.Log
          == List<std::string> {"navigate push https://example.com/view interceptable cancelable",
                                "currententrychange push", "handler", "committed https://example.com/view"});

    SECTION("the handler succeeding finishes it")
    {
      fixture.Log.clear();
      finish({});
      fixture.Run();
      CHECK(fixture.Log == List<std::string> {"navigatesuccess", "finished https://example.com/view"});
      CHECK(navigation.Transition() == nullptr);
    }

    SECTION("the handler failing is an error")
    {
      fixture.Log.clear();
      finish(dom::Exception {dom::ExceptionCode::NetworkError, u8"no data"});
      fixture.Run();
      CHECK(fixture.Log == List<std::string> {"navigateerror no data", "finished rejected no data"});
    }

    SECTION("going back is a traversal within the document")
    {
      finish({});
      fixture.Run();
      fixture.Log.clear();
      auto back = navigation.Back();
      fixture.Track(back);
      fixture.Run();
      CHECK(fixture.Log.front()
            == "navigate traverse https://example.com/a.html same interceptable cancelable");
      CHECK(fixture.Document().GetURL().String() == PageA);
      CHECK(navigation.CurrentEntry()->Index() == 0);
      CHECK(navigation.CanGoForward());
    }
  }

  TEST_CASE("Traversing to another document", "[Page][Navigation]")
  {
    bool cached = GENERATE(true, false);
    BackForwardCache::Singleton().SetMaxSize(cached ? 1 : 0);
    struct ResetCache
    {
      ~ResetCache() noexcept
      {
        BackForwardCache::Singleton().SetMaxSize(0);
      }
    } resetCache;

    NavigationFixture fixture;
    RefPtr<dom::Document> documentA = ShareRefPtr(&fixture.Document());
    fixture.Visit(PageB);
    auto keyB = fixture.GetNavigation().CurrentEntry()->Key();

    // The promises are the second document's, which goes away before they settle.
    auto back = fixture.GetNavigation().Back();
    fixture.Track(back);
    fixture.Run();

    // A document from the back/forward cache comes back with its window, and navigate fires there once it's
    // restored; otherwise it fires at the second document, and there's a new window.
    CHECK((&fixture.Document() == documentA.get()) == cached);
    CHECK(fixture.Document().GetElementById(u8"a"));
    auto &navigation = fixture.GetNavigation();
    REQUIRE(navigation.Entries().size() == 2);
    CHECK(navigation.CurrentEntry()->Index() == 0);
    CHECK(navigation.Entries()[1]->Key() == keyB);
    CHECK(navigation.CanGoForward());
    REQUIRE(navigation.Activation());
    CHECK(navigation.Activation()->NavigationType() == NavigationNavigationType::Traverse);
    CHECK(navigation.Activation()->From() == navigation.Entries()[1].get());
    CHECK(fixture.Log
          == (cached ? List<std::string> {}
                     : List<std::string> {"navigate traverse https://example.com/a.html"}));
  }

  TEST_CASE("A file's navigations can be intercepted when only its query or fragment changes",
            "[Page][Navigation]")
  {
    NavigationFixture fixture;
    fixture.Strategy.AddText(u8"file:///pages/index.html", u8"<!DOCTYPE html><p>File", u8"text/html");
    fixture.Visit(u8"file:///pages/index.html");
    List<bool> canIntercept;
    fixture.OnNavigate = [&](NavigateEvent &event)
    {
      canIntercept.push_back(event.CanIntercept());
      event.PreventDefault();
    };
    (void)fixture.GetNavigation().Navigate(u8"?topic=history");
    (void)fixture.GetNavigation().Navigate(u8"other.html");
    fixture.Run();
    CHECK(canIntercept == List<bool> {true, false});
  }

  TEST_CASE("The Navigation API's methods' errors", "[Page][Navigation]")
  {
    NavigationFixture fixture;
    auto &navigation = fixture.GetNavigation();

    auto invalid = navigation.Navigate(u8"https://exa mple.com/");
    CHECK(invalid.Committed->GetStatus() == NavigationPromise::Status::Rejected);
    CHECK(invalid.Finished->GetException()->Code() == dom::ExceptionCode::SyntaxError);

    auto unknownKey = navigation.TraverseTo(u8"no such key");
    CHECK(unknownKey.Finished->GetException()->Code() == dom::ExceptionCode::InvalidStateError);

    auto back = navigation.Back();
    CHECK(back.Finished->GetException()->Code() == dom::ExceptionCode::InvalidStateError);

    auto current = navigation.TraverseTo(navigation.CurrentEntry()->Key());
    CHECK(current.Finished->GetStatus() == NavigationPromise::Status::Fulfilled);
    CHECK(current.Finished->Entry() == navigation.CurrentEntry());
  }
}
