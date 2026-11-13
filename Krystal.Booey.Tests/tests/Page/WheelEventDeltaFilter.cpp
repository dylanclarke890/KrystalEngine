#include "Krystal.Booey/Page/WheelEventDeltaFilter.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Animation/AnimationTestUtils.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Platform/Events/PlatformWheelEvent.hpp"

namespace krys::boo::tests
{
  namespace
  {
    using Phase = PlatformWheelEventPhase;

    KRYS_NODISCARD PlatformWheelEvent PanStep(Phase phase, float deltaX, float deltaY,
                                              Phase momentumPhase = Phase::None) noexcept
    {
      return PlatformWheelEvent(IntPoint(), IntPoint(), deltaX, deltaY, 0, 0,
                                PlatformWheelEventGranularity::ScrollByPixelWheelEvent, {}, {})
        .CopyAsTouchpadPan(phase, momentumPhase);
    }
  }

  TEST_CASE("WheelEventDeltaFilter: a touchpad's pan keeps to its axis",
            "[Page][Scrolling][WheelEventDeltaFilter]")
  {
    WheelEventDeltaFilter filter;

    SECTION("while its recent steps all go mostly one way, the other way's delta is dropped")
    {
      filter.UpdateFromEvent(PanStep(Phase::Began, -2, -30));
      CHECK(filter.FilteredDelta() == FloatSize(0, -30));
      filter.UpdateFromEvent(PanStep(Phase::Changed, -3, -30));
      CHECK(filter.FilteredDelta() == FloatSize(0, -30));
      filter.UpdateFromEvent(PanStep(Phase::Changed, -30, -2));
      CHECK(filter.FilteredDelta() == FloatSize(-30, -2));

      // Three steps sideways, and it keeps to that axis instead.
      filter.UpdateFromEvent(PanStep(Phase::Changed, -30, -3));
      filter.UpdateFromEvent(PanStep(Phase::Changed, -30, -4));
      CHECK(filter.FilteredDelta() == FloatSize(-30, 0));
    }

    SECTION("it starts over when the gesture ends")
    {
      filter.UpdateFromEvent(PanStep(Phase::Changed, -30, -2));
      filter.UpdateFromEvent(PanStep(Phase::Changed, -30, -2));
      filter.UpdateFromEvent(PanStep(Phase::Ended, 0, 0));
      CHECK(filter.FilteredDelta() == FloatSize());
      filter.UpdateFromEvent(PanStep(Phase::Began, -2, -30));
      CHECK(filter.FilteredDelta() == FloatSize(0, -30));
    }

    SECTION("only the gesture's own steps are filtered, not its momentum or a wheel's notches")
    {
      CHECK(WheelEventDeltaFilter::ShouldApplyFilteringForEvent(PanStep(Phase::Began, 0, 0)));
      CHECK(WheelEventDeltaFilter::ShouldApplyFilteringForEvent(PanStep(Phase::Changed, 0, 0)));
      CHECK_FALSE(
        WheelEventDeltaFilter::ShouldApplyFilteringForEvent(PanStep(Phase::None, 0, 0, Phase::Changed)));
      CHECK_FALSE(WheelEventDeltaFilter::ShouldApplyFilteringForEvent(
        PlatformWheelEvent(IntPoint(), IntPoint(), 0, -30, 0, 1,
                           PlatformWheelEventGranularity::ScrollByPixelWheelEvent, {}, {})));
    }
  }

  TEST_CASE("WheelEventDeltaFilter: a pan that wanders a little scrolls the view along its axis",
            "[Page][Scrolling][WheelEventDeltaFilter]")
  {
    AnimationPageFixture fixture {
      u8"<!DOCTYPE html><style>body{margin:0;width:1000px;height:1000px}</style>"};
    auto &handler = fixture.TestPage->MainFrame().GetEventHandler();
    auto &view = *fixture.TestPage->MainFrame().View();
    auto pan = [&](Phase phase, float deltaX, float deltaY)
    {
      PlatformWheelEvent event(IntPoint(10, 10), IntPoint(10, 10), deltaX, deltaY, 0, 0,
                               PlatformWheelEventGranularity::ScrollByPixelWheelEvent, {}, {});
      return handler.HandleWheelEvent(event.CopyAsTouchpadPan(phase, Phase::None));
    };

    (void)pan(Phase::Began, 0, 0);
    CHECK(pan(Phase::Changed, -2, -30));
    CHECK(pan(Phase::Changed, -2, -30));
    CHECK(view.GetScrollPosition().X() == 0);
    CHECK(view.GetScrollPosition().Y() == 60);
  }
}
