#include "Krystal.Booey/CSS/Media/MediaQueryList.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Media/MediaQueryListEvent.hpp"
#include "Krystal.Booey/CSS/Queries/MediaEnvironment.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryMatcher.hpp"
#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"

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

    /// @brief Shows @p document in a new frame with an 800x600 view.
    KRYS_NODISCARD Ref<LocalFrame> ShowInFrame(dom::Document &document) noexcept
    {
      auto frame = LocalFrame::Create();
      frame->CreateView({800, 600});
      frame->SetDocument(ShareRefPtr(&document));
      return frame;
    }

    /// @brief A parsed document, evaluated against an 800x600 screen view.
    struct MatchMediaFixture : ValueFixture
    {
      Ref<dom::HTMLDocument> Document {Parse(u8"<!DOCTYPE html><p>")};
      Ref<LocalFrame> Frame {ShowInFrame(*Document)};

      MatchMediaFixture()
      {
        mq::MediaEnvironment environment;
        environment.ScreenSize = FloatSize {1'920, 1'080};
        Frame->SetMediaEnvironment(environment);
      }

      /// @brief Resizes the view to be @p width wide.
      void Resize(int32 width)
      {
        Frame->View()->Resize({width, 600});
      }
    };
  }

  TEST_CASE_METHOD(MatchMediaFixture, "MediaQueryList: matches", "[CSS][MediaQueryList]")
  {
    SECTION("Queries are evaluated against the environment")
    {
      REQUIRE(Document->Window()->MatchMedia(u8"all")->Matches());
      REQUIRE(Document->Window()->MatchMedia(u8"")->Matches());
      REQUIRE(Document->Window()->MatchMedia(u8"screen")->Matches());
      REQUIRE_FALSE(Document->Window()->MatchMedia(u8"print")->Matches());
      REQUIRE(Document->Window()->MatchMedia(u8"(min-width: 500px)")->Matches());
      REQUIRE_FALSE(Document->Window()->MatchMedia(u8"(min-width: 1000px)")->Matches());
      REQUIRE(Document->Window()->MatchMedia(u8"print, (orientation: landscape)")->Matches());
      REQUIRE_FALSE(Document->Window()->MatchMedia(u8"not all")->Matches());
      REQUIRE_FALSE(Document->Window()->MatchMedia(u8"(unknown-feature)")->Matches());
    }

    SECTION("Font-relative units resolve against the initial font")
    {
      // The initial font is the bundled Liberation Sans at 16px: an x-height of 8.453125px, a zero width of
      // 8.8984375px, a cap height of 11.0078125px, a line spacing of 18px and no water ideogram (1em).
      REQUIRE(Document->Window()->MatchMedia(u8"(width: 50em)")->Matches());
      REQUIRE(Document->Window()->MatchMedia(u8"(width: 50rem)")->Matches());
      REQUIRE(Document->Window()->MatchMedia(u8"(min-width: 94.6rex) and (max-width: 94.7rex)")->Matches());
      REQUIRE(Document->Window()->MatchMedia(u8"(min-width: 44.4rlh) and (max-width: 44.5rlh)")->Matches());
      REQUIRE(Document->Window()->MatchMedia(u8"(min-width: 94.6ex) and (max-width: 94.7ex)")->Matches());
      REQUIRE(Document->Window()->MatchMedia(u8"(min-width: 89.9ch) and (max-width: 90ch)")->Matches());
      REQUIRE(Document->Window()->MatchMedia(u8"(width: 50ic)")->Matches());
      REQUIRE(Document->Window()->MatchMedia(u8"(min-width: 72.6cap) and (max-width: 72.7cap)")->Matches());
      REQUIRE(Document->Window()->MatchMedia(u8"(min-width: 44.4lh) and (max-width: 44.5lh)")->Matches());
      REQUIRE_FALSE(Document->Window()->MatchMedia(u8"(min-width: 51em)")->Matches());
    }

    SECTION("Nothing matches without a frame or a document element")
    {
      Frame->SetDocument(nullptr);
      REQUIRE_FALSE(Document->Window()->MatchMedia(u8"all")->Matches());

      auto empty = CreateRef<dom::HTMLDocument>();
      auto emptyFrame = ShowInFrame(*empty);
      REQUIRE_FALSE(empty->Window()->MatchMedia(u8"all")->Matches());
    }

    SECTION("Without a view there's no media type, and conditions are unknown")
    {
      Frame->SetView(nullptr);
      REQUIRE(Document->Window()->MatchMedia(u8"all")->Matches());
      REQUIRE_FALSE(Document->Window()->MatchMedia(u8"screen")->Matches());
      REQUIRE_FALSE(Document->Window()->MatchMedia(u8"(min-width: 0)")->Matches());
      REQUIRE_FALSE(Document->Window()->MatchMedia(u8"not all and (min-width: 0)")->Matches());
    }

    SECTION("The result is kept until the queries are evaluated again")
    {
      auto list = Document->Window()->MatchMedia(u8"(min-width: 500px)");
      REQUIRE(list->Matches());
      Resize(400);
      REQUIRE(list->Matches());
      REQUIRE_FALSE(Document->Window()->MatchMedia(u8"(min-width: 500px)")->Matches());

      Document->EvaluateMediaQueriesAndReportChanges();
      REQUIRE_FALSE(list->Matches());
    }
  }

  TEST_CASE_METHOD(MatchMediaFixture, "MediaQueryList: media", "[CSS][MediaQueryList]")
  {
    REQUIRE(Document->Window()->MatchMedia(u8"(MIN-WIDTH: 100PX)")->Media() == u8"(min-width: 100px)");
    REQUIRE(Document->Window()->MatchMedia(u8"screen   and (color),PRINT")->Media()
            == u8"screen and (color), print");
    REQUIRE(Document->Window()->MatchMedia(u8"")->Media() == u8"");
    REQUIRE(Document->Window()->MatchMedia(u8"(min-width: 100px")->Media() == u8"(min-width: 100px)");
    REQUIRE(Document->Window()->MatchMedia(u8"foo bar")->Media() == u8"not all");
  }

  TEST_CASE_METHOD(MatchMediaFixture, "MediaQueryList: change events", "[CSS][MediaQueryList]")
  {
    List<ChangeRecord> records;
    Ref<dom::EventListenerCallback> recorder = CreateRef<ChangeRecorder>(records);

    auto wide = Document->Window()->MatchMedia(u8"(min-width: 500px)");
    auto narrow = Document->Window()->MatchMedia(u8"(max-width: 499px)");
    auto print = Document->Window()->MatchMedia(u8"print");
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
      wide = Document->Window()->MatchMedia(u8"print");

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
