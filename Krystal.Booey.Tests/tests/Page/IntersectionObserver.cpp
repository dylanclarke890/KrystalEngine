#include "Krystal.Booey/Page/IntersectionObserver.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/NavigationTestUtils.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Page/IntersectionObserverEntry.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include <cmath>

namespace krys::boo::tests
{
  namespace
  {
    /// @brief A rect as delivered, in client coordinates.
    struct Rect
    {
      double X {0}, Y {0}, Width {0}, Height {0};

      bool operator==(const Rect &) const = default;
    };

    /// @brief An entry as delivered: the target's ID, whether it intersects, the intersection ratio as a
    /// percentage, and the target's, the intersection's and the root's rects.
    struct Observed
    {
      std::string Id;
      bool IsIntersecting {false};
      long Percent {0};
      Rect Bounds;
      Rect Intersection;
      Rect Root;

      bool operator==(const Observed &) const = default;
    };

    KRYS_NODISCARD Rect ToRect(const dom::DOMRectReadOnly *rect)
    {
      REQUIRE(rect);
      return {rect->X(), rect->Y(), rect->Width(), rect->Height()};
    }

    KRYS_NODISCARD Observed ToObserved(const IntersectionObserverEntry &entry)
    {
      return {Str(entry.Target()->Id()),
              entry.IsIntersecting(),
              std::lround(entry.IntersectionRatio() * 100),
              ToRect(entry.BoundingClientRect()),
              ToRect(entry.IntersectionRect()),
              ToRect(entry.RootBounds())};
    }

    /// @brief An observer that records what it's told, notification by notification.
    struct Recorder
    {
      List<List<Observed>> Notifications;
      RefPtr<IntersectionObserver> Observer;

      explicit Recorder(dom::Document &document, IntersectionObserver::Init &&init = {}) noexcept
      {
        auto observer = IntersectionObserver::Create(
          document,
          [this](const List<Ref<IntersectionObserverEntry>> &entries, IntersectionObserver &)
          {
            auto &notification = Notifications.emplace_back();
            for (auto &entry : entries)
            {
              notification.push_back(ToObserved(*entry));
            }
          },
          krys::move(init));
        REQUIRE_FALSE(observer.HasException());
        Observer = observer.ReleaseValue();
      }

      /// @brief The notifications delivered since the last call.
      KRYS_NODISCARD List<List<Observed>> Take() noexcept
      {
        return std::exchange(Notifications, {});
      }
    };

    KRYS_NODISCARD dom::ExceptionCode CreateError(dom::Document &document, IntersectionObserver::Init &&init)
    {
      auto observer = IntersectionObserver::Create(
        document, [](const List<Ref<IntersectionObserverEntry>> &, IntersectionObserver &) {},
        krys::move(init));
      REQUIRE(observer.HasException());
      return observer.GetException().Code();
    }

    constexpr Rect Viewport {0, 0, 400, 300};
  }

  TEST_CASE("IntersectionObserver: creating", "[Page][IntersectionObserver]")
  {
    NavigationFixture fixture(u8"<div id=a></div>");
    auto &document = *fixture.Document;

    SECTION("The defaults")
    {
      Recorder recorder(document);
      CHECK(recorder.Observer->Root() == nullptr);
      CHECK(Str(recorder.Observer->RootMargin()) == "0px 0px 0px 0px");
      CHECK(Str(recorder.Observer->ScrollMargin()) == "0px 0px 0px 0px");
      CHECK(recorder.Observer->Thresholds() == List<double> {0});
    }

    SECTION("Margins take one to four lengths in pixels or percentages")
    {
      Recorder recorder(document, {.RootMargin = u8"10px 20%", .ScrollMargin = u8" 1px 2px 3px "});
      CHECK(Str(recorder.Observer->RootMargin()) == "10px 20% 10px 20%");
      CHECK(Str(recorder.Observer->ScrollMargin()) == "1px 2px 3px 2px");

      Recorder empty(document, {.RootMargin = u8"", .ScrollMargin = u8"-5px"});
      CHECK(Str(empty.Observer->RootMargin()) == "0px 0px 0px 0px");
      CHECK(Str(empty.Observer->ScrollMargin()) == "-5px -5px -5px -5px");

      CHECK(CreateError(document, {.RootMargin = u8"1em"}) == dom::ExceptionCode::SyntaxError);
      CHECK(CreateError(document, {.RootMargin = u8"calc(10% + 1px)"}) == dom::ExceptionCode::SyntaxError);
      CHECK(CreateError(document, {.ScrollMargin = u8"1px 2px 3px 4px 5px"})
            == dom::ExceptionCode::SyntaxError);
      CHECK(CreateError(document, {.ScrollMargin = u8"auto"}) == dom::ExceptionCode::SyntaxError);
    }

    SECTION("Thresholds are sorted, and must lie between 0 and 1")
    {
      Recorder sorted(document, {.Threshold = List<double> {1, 0.5, 0}});
      CHECK(sorted.Observer->Thresholds() == List<double> {0, 0.5, 1});

      Recorder empty(document, {.Threshold = List<double> {}});
      CHECK(empty.Observer->Thresholds() == List<double> {0});

      CHECK(CreateError(document, {.Threshold = 1.5}) == dom::ExceptionCode::RangeError);
      CHECK(CreateError(document, {.Threshold = List<double> {0, -0.1}}) == dom::ExceptionCode::RangeError);
    }

    SECTION("The root is an element or a document")
    {
      auto &a = fixture.ById(u8"a");
      Recorder byElement(document, {.Root = ShareRefPtr(&a)});
      CHECK(byElement.Observer->Root() == &a);
      CHECK(byElement.Observer->TrackingDocument() == &document);

      Recorder byDocument(document, {.Root = RefPtr<dom::Document>(ShareRefPtr(&document))});
      CHECK(byDocument.Observer->Root() == &document);

      Recorder implicit(document);
      CHECK(implicit.Observer->TrackingDocument() == &document);
    }
  }

  TEST_CASE("IntersectionObserver: the implicit root", "[Page][IntersectionObserver]")
  {
    NavigationFixture fixture(u8"<style>html { overflow: hidden } body { margin: 0 }</style>"
                              u8"<div id=a style='height: 100px'></div><div style='height: 400px'></div>"
                              u8"<div id=b style='width: 50px; height: 100px'></div>");
    auto &a = fixture.ById(u8"a");
    auto &b = fixture.ById(u8"b");
    auto &window = *fixture.Document->Window();
    auto update = [&]
    {
      fixture.TestPage->UpdateRendering();
    };

    SECTION("Each target is reported first, then when it crosses a threshold")
    {
      Recorder recorder(*fixture.Document);
      recorder.Observer->Observe(a);
      recorder.Observer->Observe(b);
      CHECK(fixture.Document->NumberOfIntersectionObservers() == 1);
      update();
      CHECK(recorder.Take()
            == List<List<Observed>> {{
              {"a", true, 100, {0, 0, 400, 100}, {0, 0, 400, 100}, Viewport},
              {"b", false, 0, {0, 500, 50, 100}, {}, Viewport},
            }});

      update();
      CHECK(recorder.Take().empty());

      window.ScrollTo(0, 250);
      update();
      CHECK(recorder.Take()
            == List<List<Observed>> {{
              {"a", false, 0, {0, -250, 400, 100}, {}, Viewport},
              {"b", true, 50, {0, 250, 50, 100}, {0, 250, 50, 50}, Viewport},
            }});
    }

    SECTION("Thresholds split the ratio into ranges")
    {
      Recorder recorder(*fixture.Document, {.Threshold = List<double> {0, 0.5, 1}});
      recorder.Observer->Observe(b);
      update();
      CHECK(recorder.Take().size() == 1);

      window.ScrollTo(0, 220);
      update();
      auto notifications = recorder.Take();
      REQUIRE(notifications.size() == 1);
      CHECK(notifications[0][0].Percent == 20);

      window.ScrollTo(0, 230);
      update();
      CHECK(recorder.Take().empty());

      window.ScrollTo(0, 260);
      update();
      notifications = recorder.Take();
      REQUIRE(notifications.size() == 1);
      CHECK(notifications[0][0].Percent == 60);

      window.ScrollTo(0, 300);
      update();
      notifications = recorder.Take();
      REQUIRE(notifications.size() == 1);
      CHECK(notifications[0][0].Percent == 100);
    }

    SECTION("The root margin grows the root")
    {
      Recorder pixels(*fixture.Document, {.RootMargin = u8"0px 0px 250px 0px"});
      pixels.Observer->Observe(b);
      Recorder percent(*fixture.Document, {.RootMargin = u8"0px 0px 100%"});
      percent.Observer->Observe(b);
      update();
      CHECK(pixels.Take()
            == List<List<Observed>> {{
              {"b", true, 50, {0, 500, 50, 100}, {0, 500, 50, 50}, {0, 0, 400, 550}},
            }});
      CHECK(percent.Take()
            == List<List<Observed>> {{
              {"b", true, 100, {0, 500, 50, 100}, {0, 500, 50, 100}, {0, 0, 400, 600}},
            }});
    }

    SECTION("Records can be taken before they're delivered")
    {
      Recorder recorder(*fixture.Document);
      recorder.Observer->Observe(a);
      (void)fixture.Document->UpdateLayout();
      CHECK(recorder.Observer->UpdateObservations(fixture.TestPage->MainFrame())
            == IntersectionObserver::NeedNotify::Yes);
      auto records = recorder.Observer->TakeRecords();
      REQUIRE(records.size() == 1);
      CHECK(ToObserved(*records[0])
            == Observed {"a", true, 100, {0, 0, 400, 100}, {0, 0, 400, 100}, Viewport});
      CHECK(recorder.Observer->TakeRecords().empty());

      update();
      CHECK(recorder.Take().empty());
    }

    SECTION("Unobserving and disconnecting stop the reports")
    {
      Recorder recorder(*fixture.Document);
      recorder.Observer->Observe(a);
      recorder.Observer->Observe(b);
      update();
      (void)recorder.Take();

      recorder.Observer->Unobserve(a);
      CHECK_FALSE(recorder.Observer->IsObserving(a));
      window.ScrollTo(0, 250);
      update();
      auto notifications = recorder.Take();
      REQUIRE(notifications.size() == 1);
      REQUIRE(notifications[0].size() == 1);
      CHECK(notifications[0][0].Id == "b");

      recorder.Observer->Disconnect();
      CHECK(fixture.Document->NumberOfIntersectionObservers() == 0);
      window.ScrollTo(0, 0);
      update();
      CHECK(recorder.Take().empty());
    }
  }

  TEST_CASE("IntersectionObserver: scrollers and roots", "[Page][IntersectionObserver]")
  {
    NavigationFixture fixture(
      u8"<style>body { margin: 0 }</style>"
      u8"<div id=scroller style='overflow: hidden; width: 200px; height: 100px'>"
      u8"<div style='height: 150px'></div><div id=t style='height: 50px'></div></div>");
    auto &scroller = fixture.ById(u8"scroller");
    auto &t = fixture.ById(u8"t");
    auto update = [&]
    {
      fixture.TestPage->UpdateRendering();
    };

    SECTION("An element root reports its scrolled contents")
    {
      Recorder recorder(*fixture.Document, {.Root = ShareRefPtr(&scroller)});
      recorder.Observer->Observe(t);
      update();
      CHECK(recorder.Take()
            == List<List<Observed>> {{
              {"t", false, 0, {0, 150, 200, 50}, {}, {0, 0, 200, 100}},
            }});

      scroller.SetScrollTop(100);
      update();
      CHECK(recorder.Take()
            == List<List<Observed>> {{
              {"t", true, 100, {0, 50, 200, 50}, {0, 50, 200, 50}, {0, 0, 200, 100}},
            }});
    }

    SECTION("The implicit root sees through the scroller's clip, grown by the scroll margin")
    {
      Recorder clipped(*fixture.Document);
      clipped.Observer->Observe(t);
      Recorder margin(*fixture.Document, {.ScrollMargin = u8"60px"});
      margin.Observer->Observe(t);
      update();
      CHECK(clipped.Take()
            == List<List<Observed>> {{
              {"t", false, 0, {0, 150, 200, 50}, {}, Viewport},
            }});
      // The scroll margin also grows the root.
      CHECK(margin.Take()
            == List<List<Observed>> {{
              {"t", true, 20, {0, 150, 200, 50}, {0, 150, 200, 10}, {-60, -60, 520, 420}},
            }});
    }

    SECTION("A target outside the root is not intersecting")
    {
      Recorder recorder(*fixture.Document, {.Root = ShareRefPtr(&t)});
      recorder.Observer->Observe(scroller);
      update();
      CHECK(recorder.Take() == List<List<Observed>> {{{"scroller", false, 0, {}, {}, {}}}});
    }
  }

  TEST_CASE("IntersectionObserver: targets and roots going away", "[Page][IntersectionObserver]")
  {
    NavigationFixture fixture(u8"<style>body { margin: 0 }</style><div id=a style='height: 100px'></div>");
    auto &a = fixture.ById(u8"a");
    auto update = [&]
    {
      fixture.TestPage->UpdateRendering();
    };

    SECTION("A removed target stops intersecting")
    {
      Recorder recorder(*fixture.Document);
      recorder.Observer->Observe(a);
      update();
      CHECK(recorder.Take().size() == 1);

      Ref<dom::Element> removed = ShareRef(a);
      removed->Remove();
      update();
      CHECK(recorder.Take() == List<List<Observed>> {{{"a", false, 0, {}, {}, {}}}});
      update();
      CHECK(recorder.Take().empty());
    }

    SECTION("A destroyed target is forgotten")
    {
      Recorder recorder(*fixture.Document);
      {
        auto element = fixture.Document->CreateElement(u8"div").ReleaseValue();
        recorder.Observer->Observe(*element);
      }
      // The target is kept until its first observation.
      update();
      CHECK(recorder.Take().size() == 1);
      CHECK_FALSE(recorder.Observer->HasObservationTargets());
      CHECK(fixture.Document->NumberOfIntersectionObservers() == 0);
    }

    SECTION("A destroyed root disconnects its observers")
    {
      RefPtr<dom::Element> root = fixture.Document->CreateElement(u8"div").ReleaseValue();
      Recorder recorder(*fixture.Document, {.Root = root});
      recorder.Observer->Observe(a);
      CHECK(recorder.Observer->HasObservationTargets());
      root = nullptr;
      CHECK(recorder.Observer->Root() == nullptr);
      CHECK_FALSE(recorder.Observer->HasObservationTargets());
      CHECK(fixture.Document->NumberOfIntersectionObservers() == 0);
    }

    SECTION("A destroyed observer leaves its targets and its root")
    {
      {
        Recorder recorder(*fixture.Document,
                          {.Root = RefPtr<dom::Document>(ShareRefPtr(fixture.Document.get()))});
        recorder.Observer->Observe(a);
        REQUIRE(a.IntersectionObserverDataIfExists());
        CHECK(a.IntersectionObserverDataIfExists()->Registrations.size() == 1);
        CHECK(fixture.Document->IntersectionObserverDataIfExists()->Observers.size() == 1);
      }
      CHECK(a.IntersectionObserverDataIfExists()->Registrations.empty());
      CHECK(fixture.Document->IntersectionObserverDataIfExists()->Observers.empty());
      CHECK(fixture.Document->NumberOfIntersectionObservers() == 0);
    }
  }
}
