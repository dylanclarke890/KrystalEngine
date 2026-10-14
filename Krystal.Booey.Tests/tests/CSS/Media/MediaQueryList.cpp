#include "Krystal.Booey/CSS/Media/MediaQueryList.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Media/MediaQueryListEvent.hpp"
#include "Krystal.Booey/CSS/Queries/MediaEnvironment.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryMatcher.hpp"
#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    KRYS_NODISCARD Ref<dom::HTMLDocument> Parse(utf8_string &&input) noexcept
    {
      auto document = CreateRef<dom::HTMLDocument>();
      auto parser = CreateRef<html::HTMLDocumentParser>(*document);
      parser->InputStream().Append(krys::move(input), IsEOF(true));
      (void)parser->PumpTokenizer();
      return document;
    }

    /// @brief What a `change` event carried.
    struct ChangeRecord
    {
      CSSOMString Media;
      bool Matches {false};
      bool IsTrusted {false};
      const dom::EventTarget *Target {nullptr};
    };

    /// @brief Records the `change` events it handles, in order, across every listener sharing @p records.
    struct ChangeRecorder : public dom::EventListenerCallback
    {
      List<ChangeRecord> &Records;

      explicit ChangeRecorder(List<ChangeRecord> &records) noexcept : Records(records)
      {
      }

      void HandleEvent(dom::Event &event) noexcept override
      {
        // Only `change` listeners are added, and `change` is only fired with a `MediaQueryListEvent`.
        auto &change = static_cast<MediaQueryListEvent &>(event);
        Records.push_back({change.Media(), change.Matches(), change.IsTrusted(), event.Target().get()});
      }
    };

    /// @brief A parsed document, evaluated against an 800x600 screen view.
    struct MatchMediaFixture : ValueFixture
    {
      Ref<dom::HTMLDocument> Document {Parse(u8"<!DOCTYPE html><p>")};
      mq::MediaEnvironment Environment {};

      MatchMediaFixture()
      {
        Environment.LayoutSize = IntSize {800, 600};
        Environment.ScreenSize = FloatSize {1'920, 1'080};
        mq::SetMediaEnvironment(Environment);
      }

      ~MatchMediaFixture() noexcept
      {
        mq::SetMediaEnvironment(null);
      }

      /// @brief Changes the environment to have a view @p width wide.
      void Resize(int32 width)
      {
        Environment.LayoutSize = IntSize {width, 600};
        mq::SetMediaEnvironment(Environment);
      }
    };
  }

  TEST_CASE_METHOD(MatchMediaFixture, "MediaQueryList: matches", "[CSS][MediaQueryList]")
  {
    SECTION("Queries are evaluated against the environment")
    {
      REQUIRE(Document->MatchMedia(u8"all")->Matches());
      REQUIRE(Document->MatchMedia(u8"")->Matches());
      REQUIRE(Document->MatchMedia(u8"screen")->Matches());
      REQUIRE_FALSE(Document->MatchMedia(u8"print")->Matches());
      REQUIRE(Document->MatchMedia(u8"(min-width: 500px)")->Matches());
      REQUIRE_FALSE(Document->MatchMedia(u8"(min-width: 1000px)")->Matches());
      REQUIRE(Document->MatchMedia(u8"print, (orientation: landscape)")->Matches());
      REQUIRE_FALSE(Document->MatchMedia(u8"not all")->Matches());
      REQUIRE_FALSE(Document->MatchMedia(u8"(unknown-feature)")->Matches());
    }

    SECTION("Font-relative units resolve against the initial font")
    {
      REQUIRE(Document->MatchMedia(u8"(width: 50em)")->Matches());
      REQUIRE(Document->MatchMedia(u8"(width: 50rem)")->Matches());
      REQUIRE(Document->MatchMedia(u8"(width: 100rex)")->Matches());
      REQUIRE(Document->MatchMedia(u8"(min-width: 42rlh) and (max-width: 42.2rlh)")->Matches());
      REQUIRE(Document->MatchMedia(u8"(width: 100ex)")->Matches());
      REQUIRE(Document->MatchMedia(u8"(width: 100ch)")->Matches());
      REQUIRE(Document->MatchMedia(u8"(width: 50ic)")->Matches());
      REQUIRE(Document->MatchMedia(u8"(width: 50cap)")->Matches());
      REQUIRE(Document->MatchMedia(u8"(min-width: 42lh) and (max-width: 42.2lh)")->Matches());
      REQUIRE_FALSE(Document->MatchMedia(u8"(min-width: 51em)")->Matches());
    }

    SECTION("Nothing matches without a view or a document element")
    {
      mq::SetMediaEnvironment(null);
      REQUIRE_FALSE(Document->MatchMedia(u8"all")->Matches());

      mq::SetMediaEnvironment(Environment);
      auto empty = CreateRef<dom::HTMLDocument>();
      REQUIRE_FALSE(empty->MatchMedia(u8"all")->Matches());
    }

    SECTION("The result is kept until the queries are evaluated again")
    {
      auto list = Document->MatchMedia(u8"(min-width: 500px)");
      REQUIRE(list->Matches());
      Resize(400);
      REQUIRE(list->Matches());
      REQUIRE_FALSE(Document->MatchMedia(u8"(min-width: 500px)")->Matches());

      Document->EvaluateMediaQueriesAndReportChanges();
      REQUIRE_FALSE(list->Matches());
    }
  }

  TEST_CASE_METHOD(MatchMediaFixture, "MediaQueryList: media", "[CSS][MediaQueryList]")
  {
    REQUIRE(Document->MatchMedia(u8"(MIN-WIDTH: 100PX)")->Media() == u8"(min-width: 100px)");
    REQUIRE(Document->MatchMedia(u8"screen   and (color),PRINT")->Media() == u8"screen and (color), print");
    REQUIRE(Document->MatchMedia(u8"")->Media() == u8"");
    REQUIRE(Document->MatchMedia(u8"(min-width: 100px")->Media() == u8"(min-width: 100px)");
    REQUIRE(Document->MatchMedia(u8"foo bar")->Media() == u8"not all");
  }

  TEST_CASE_METHOD(MatchMediaFixture, "MediaQueryList: change events", "[CSS][MediaQueryList]")
  {
    List<ChangeRecord> records;
    Ref<dom::EventListenerCallback> recorder = CreateRef<ChangeRecorder>(records);

    auto wide = Document->MatchMedia(u8"(min-width: 500px)");
    auto narrow = Document->MatchMedia(u8"(max-width: 499px)");
    auto print = Document->MatchMedia(u8"print");
    wide->AddEventListener(u8"change", Ref {recorder}, false);
    narrow->AddListener(recorder);
    print->AddListener(recorder);

    SECTION("Lists whose result changed get one trusted event each, in creation order")
    {
      Resize(400);
      Document->EvaluateMediaQueriesAndReportChanges();
      REQUIRE(records.size() == 2u);
      REQUIRE(records[0].Media == u8"(min-width: 500px)");
      REQUIRE_FALSE(records[0].Matches);
      REQUIRE(records[0].IsTrusted);
      REQUIRE(records[0].Target == wide.get());
      REQUIRE(records[1].Target == narrow.get());
      REQUIRE(records[1].Media == u8"(max-width: 499px)");
      REQUIRE(records[1].Matches);

      Document->EvaluateMediaQueriesAndReportChanges();
      REQUIRE(records.size() == 2u);

      Resize(800);
      Document->EvaluateMediaQueriesAndReportChanges();
      REQUIRE(records.size() == 4u);
      REQUIRE(records[2].Matches);
      REQUIRE_FALSE(records[3].Matches);
    }

    SECTION("A change that's undone before evaluation isn't reported")
    {
      Resize(400);
      Resize(800);
      Document->EvaluateMediaQueriesAndReportChanges();
      REQUIRE(records.empty());
    }

    SECTION("A scheduled change is reported at the next dispatch")
    {
      Resize(400);
      Document->GetMediaQueryMatcher().EvaluateAll(MediaQueryMatcher::EventMode::Schedule);
      REQUIRE(records.empty());
      REQUIRE_FALSE(wide->Matches());

      Document->EvaluateMediaQueriesAndReportChanges();
      REQUIRE(records.size() == 2u);
    }

    SECTION("Removed listeners and released lists get nothing")
    {
      narrow->RemoveListener(recorder);
      narrow->AddListener(nullptr);
      narrow->RemoveListener(nullptr);
      wide = Document->MatchMedia(u8"print");

      Resize(400);
      Document->EvaluateMediaQueriesAndReportChanges();
      REQUIRE(records.empty());
    }

    SECTION("Adding the same listener twice dispatches once")
    {
      narrow->AddListener(recorder);
      narrow->AddEventListener(u8"change", Ref {recorder}, false);

      Resize(400);
      Document->EvaluateMediaQueriesAndReportChanges();
      REQUIRE(records.size() == 2u);
    }
  }

  TEST_CASE("MediaQueryListEvent: constructor", "[CSS][MediaQueryList]")
  {
    auto defaults = CreateRef<MediaQueryListEvent>(u8"change");
    REQUIRE(defaults->Type() == u8"change");
    REQUIRE(defaults->Media().empty());
    REQUIRE_FALSE(defaults->Matches());
    REQUIRE_FALSE(defaults->IsTrusted());
    REQUIRE_FALSE(defaults->Bubbles());

    MediaQueryListEventInit init;
    init.Media = u8"(color)";
    init.Matches = true;
    init.Bubbles = true;
    auto event = CreateRef<MediaQueryListEvent>(u8"custom", init);
    REQUIRE(event->Type() == u8"custom");
    REQUIRE(event->Media() == u8"(color)");
    REQUIRE(event->Matches());
    REQUIRE(event->Bubbles());
  }
}
