#include "Krystal.Booey/Page/ResizeObserver/ResizeObserver.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/NavigationTestUtils.hpp"
#include "Krystal.Booey/DOM/Events/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Page/ResizeObserver/ResizeObserverEntry.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"

namespace krys::boo::tests
{
  namespace
  {
    /// @brief An observation as delivered: the target's ID, its content rect, and its content box and border
    /// box sizes (inline by block).
    struct Observed
    {
      std::string Id;
      double X {0}, Y {0}, Width {0}, Height {0};
      double ContentInline {0}, ContentBlock {0};
      double BorderInline {0}, BorderBlock {0};

      bool operator==(const Observed &) const = default;
    };

    KRYS_NODISCARD std::string Narrow(dom::DOMStringView string)
    {
      return std::string(reinterpret_cast<const char *>(string.data()), string.size());
    }

    /// @brief An observer that records what it's told, round by round.
    struct Recorder
    {
      List<List<Observed>> Rounds;
      /// @brief Each entry's device pixel content box size (inline, block).
      List<std::pair<double, double>> DevicePixelSizes;
      Func<void(ResizeObserver &)> OnDeliver;
      Ref<ResizeObserver> Observer;

      explicit Recorder(dom::Document &document) noexcept
          : Observer(ResizeObserver::Create(
              document,
              [this](const List<Ref<ResizeObserverEntry>> &entries, ResizeObserver &observer)
              {
                auto &round = Rounds.emplace_back();
                for (auto &entry : entries)
                {
                  auto &rect = entry->ContentRect();
                  auto &content = entry->ContentBoxSize().front();
                  auto &border = entry->BorderBoxSize().front();
                  auto &devicePixels = entry->DevicePixelContentBoxSize().front();
                  DevicePixelSizes.emplace_back(devicePixels->InlineSize(), devicePixels->BlockSize());
                  round.push_back({Narrow(entry->Target().Id()), rect.X(), rect.Y(), rect.Width(),
                                   rect.Height(), content->InlineSize(), content->BlockSize(),
                                   border->InlineSize(), border->BlockSize()});
                }
                if (OnDeliver)
                {
                  OnDeliver(observer);
                }
              }))
      {
      }

      /// @brief The rounds delivered since the last call.
      KRYS_NODISCARD List<List<Observed>> Take() noexcept
      {
        return std::exchange(Rounds, {});
      }
    };

    void SetStyle(dom::Element &element, dom::DOMStringView style)
    {
      REQUIRE_FALSE(element.SetAttribute(u8"style", dom::DOMString(style)).HasException());
    }
  }

  TEST_CASE("ResizeObserver: observing boxes", "[Page][ResizeObserver]")
  {
    NavigationFixture fixture(
      u8"<style>body { margin: 0 } div { box-sizing: content-box }</style>"
      u8"<div id=a style='width: 100px; height: 50px; padding: 10px; border: 2px solid'>"
      u8"</div><div id=b style='display: none'></div>");
    auto &a = fixture.ById(u8"a");
    auto &b = fixture.ById(u8"b");
    Recorder recorder(*fixture.Document);
    auto update = [&]
    {
      fixture.TestPage->UpdateRendering();
    };

    SECTION("The first observation reports the content box, and changes report again")
    {
      recorder.Observer->Observe(a);
      update();
      CHECK(recorder.Take() == List<List<Observed>> {{{"a", 10, 10, 100, 50, 100, 50, 124, 74}}});

      update();
      CHECK(recorder.Take().empty());

      SetStyle(a, u8"width: 80px; height: 50px; padding: 4px; border: 2px solid");
      update();
      CHECK(recorder.Take() == List<List<Observed>> {{{"a", 4, 4, 80, 50, 80, 50, 92, 62}}});

      // Only the padding changing leaves the content box as it was.
      SetStyle(a, u8"width: 80px; height: 50px; padding: 6px; border: 2px solid");
      update();
      CHECK(recorder.Take().empty());
    }

    SECTION("A border box observation reports changes to the border box")
    {
      recorder.Observer->Observe(a, {ResizeObserverBoxOptions::BorderBox});
      update();
      CHECK(recorder.Take().size() == 1);

      SetStyle(a, u8"width: 100px; height: 50px; padding: 12px; border: 2px solid");
      update();
      CHECK(recorder.Take() == List<List<Observed>> {{{"a", 12, 12, 100, 50, 100, 50, 128, 78}}});

      // Observing again with the same box changes nothing; with another, the observation starts over.
      recorder.Observer->Observe(a, {ResizeObserverBoxOptions::BorderBox});
      update();
      CHECK(recorder.Take().empty());
      recorder.Observer->Observe(a);
      update();
      CHECK(recorder.Take().size() == 1);
    }

    SECTION("An element without a box is reported as empty once")
    {
      recorder.Observer->Observe(b);
      update();
      CHECK(recorder.Take() == List<List<Observed>> {{{"b"}}});
      update();
      CHECK(recorder.Take().empty());

      SetStyle(b, u8"width: 10px; height: 20px");
      update();
      CHECK(recorder.Take() == List<List<Observed>> {{{"b", 0, 0, 10, 20, 10, 20, 10, 20}}});
    }

    SECTION("Sizes are in the writing mode's axes and unzoomed")
    {
      SetStyle(a, u8"width: 100px; height: 50px; writing-mode: vertical-lr; zoom: 2");
      recorder.Observer->Observe(a);
      update();
      CHECK(recorder.Take() == List<List<Observed>> {{{"a", 0, 0, 100, 50, 50, 100, 50, 100}}});
    }

    SECTION("The device pixel content box is in whole device pixels, snapped where it sits")
    {
      fixture.TestPage->SetDeviceScaleFactor(1.5f);
      SetStyle(a, u8"width: 101px; height: 50px; margin-left: 0.5px; writing-mode: vertical-lr");
      recorder.Observer->Observe(a, {ResizeObserverBoxOptions::DevicePixelContentBox});
      update();
      REQUIRE(recorder.Take().size() == 1);
      // The content box spans 0.5px to 101.5px, 0.75 to 152.25 device pixels: 152 - 1 = 151 once snapped.
      // Its block axis is horizontal.
      CHECK(recorder.DevicePixelSizes == List<std::pair<double, double>> {{75, 151}});

      // Moving it a little changes its device pixel size but not its content box.
      SetStyle(a, u8"width: 101px; height: 50px; margin-left: 0px; writing-mode: vertical-lr");
      update();
      CHECK(recorder.Take().size() == 1);
      CHECK(recorder.DevicePixelSizes.back() == std::pair<double, double> {75, 152});
    }

    SECTION("Unobserving, disconnecting and destroying the observer stop the reports")
    {
      recorder.Observer->Observe(a);
      recorder.Observer->Observe(b);
      update();
      CHECK(recorder.Take().front().size() == 2);

      recorder.Observer->Unobserve(a);
      SetStyle(a, u8"width: 10px");
      SetStyle(b, u8"width: 10px");
      update();
      CHECK(recorder.Take() == List<List<Observed>> {{{"b", 0, 0, 10, 0, 10, 0, 10, 0}}});

      recorder.Observer->Disconnect();
      SetStyle(b, u8"width: 20px");
      update();
      CHECK(recorder.Take().empty());

      // An observer that's destroyed leaves its targets.
      {
        Recorder other(*fixture.Document);
        other.Observer->Observe(a);
        REQUIRE(a.ResizeObserverDataIfExists());
        CHECK(a.ResizeObserverDataIfExists()->Observers.size() == 1);
      }
      CHECK(a.ResizeObserverDataIfExists()->Observers.empty());
      update();
    }
  }

  TEST_CASE("ResizeObserver: targets and depth", "[Page][ResizeObserver]")
  {
    NavigationFixture fixture(
      u8"<style>body { margin: 0 }</style>"
      u8"<div id=outer style='width: 200px; height: 100px'><div id=inner style='height: 10px'></div></div>");
    auto &outer = fixture.ById(u8"outer");
    auto &inner = fixture.ById(u8"inner");
    Recorder recorder(*fixture.Document);
    auto update = [&]
    {
      fixture.TestPage->UpdateRendering();
    };

    SECTION("Resizing a deeper element in a callback is reported in the same update")
    {
      recorder.Observer->Observe(outer);
      recorder.Observer->Observe(inner);
      update();
      CHECK(recorder.Take().size() == 1);

      recorder.OnDeliver = [&](ResizeObserver &)
      {
        SetStyle(inner, u8"height: 30px");
      };
      SetStyle(outer, u8"width: 150px; height: 100px");
      update();
      auto rounds = recorder.Take();
      REQUIRE(rounds.size() == 2);
      CHECK(rounds[0].size() == 2); // The outer box, and the inner box as it's as wide.
      CHECK(rounds[1] == List<Observed> {{"inner", 0, 0, 150, 30, 150, 30, 150, 30}});
    }

    SECTION("Resizing a shallower element in a callback is left for the next update")
    {
      recorder.Observer->Observe(outer, {ResizeObserverBoxOptions::BorderBox});
      recorder.Observer->Observe(inner);
      update();
      CHECK(recorder.Take().size() == 1);

      recorder.OnDeliver = [&](ResizeObserver &)
      {
        recorder.OnDeliver = nullptr;
        SetStyle(outer, u8"width: 200px; height: 100px; padding-top: 5px");
      };
      SetStyle(inner, u8"height: 20px");
      update();
      CHECK(recorder.Take() == List<List<Observed>> {{{"inner", 0, 0, 200, 20, 200, 20, 200, 20}}});

      update();
      CHECK(recorder.Take() == List<List<Observed>> {{{"outer", 0, 5, 200, 100, 200, 100, 200, 105}}});
    }

    SECTION("A removed target is reported as empty; a destroyed one is forgotten")
    {
      recorder.Observer->Observe(inner);
      update();
      (void)recorder.Take();

      Ref<dom::Element> removed = ShareRef(inner);
      removed->Remove();
      update();
      CHECK(recorder.Take() == List<List<Observed>> {{{"inner"}}});

      {
        auto element = fixture.Document->CreateElement(u8"div").ReleaseValue();
        recorder.Observer->Observe(*element);
        recorder.Observer->Unobserve(*element);
      }
      update();
      CHECK(recorder.Take().empty());
    }
  }

  TEST_CASE("ResizeObserver: the last remembered size", "[Page][ResizeObserver]")
  {
    NavigationFixture fixture(u8"<style>body { margin: 0 }</style>"
                              u8"<div id=box style='contain-intrinsic-size: auto 300px'>"
                              u8"<div style='height: 120px'></div></div>");
    auto &box = fixture.ById(u8"box");
    auto update = [&]
    {
      fixture.TestPage->UpdateRendering();
    };
    auto height = [&]
    {
      (void)fixture.Document->UpdateLayout();
      auto *renderBox = DynamicDowncast<RenderBox>(box.Renderer());
      REQUIRE(renderBox);
      return renderBox->Height().ToInt();
    };

    update();
    CHECK(box.LastRememberedLogicalHeight() == LayoutUnit(120));
    CHECK(box.LastRememberedLogicalWidth() == LayoutUnit(400));

    // While its contents are skipped, its height is the last remembered one rather than the fallback.
    SetStyle(box, u8"contain-intrinsic-size: auto 300px; content-visibility: hidden");
    CHECK(height() == 120);
    update();
    CHECK(box.LastRememberedLogicalHeight() == LayoutUnit(120));

    // Without auto, the size is forgotten.
    SetStyle(box, u8"contain-intrinsic-size: 300px; content-visibility: hidden");
    CHECK(height() == 300);
    CHECK(box.LastRememberedLogicalHeight() == null);

    // A removed element forgets its size at the next update.
    SetStyle(box, u8"contain-intrinsic-size: auto 300px");
    update();
    CHECK(box.LastRememberedLogicalHeight() == LayoutUnit(120));
    Ref<dom::Element> removed = ShareRef(box);
    removed->Remove();
    update();
    CHECK(removed->LastRememberedLogicalHeight() == null);
  }
}
