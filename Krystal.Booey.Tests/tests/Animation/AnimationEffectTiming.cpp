#include "Krystal.Booey/Animation/AnimationEffectTiming.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::tests
{
  namespace
  {
    /// @brief A time-based effect: 1s delay, 2s iterations, 0.5s end delay.
    KRYS_NODISCARD AnimationEffectTiming MakeTiming(double iterations = 1,
                                                    PlaybackDirection direction = PlaybackDirection::Normal,
                                                    FillMode fill = FillMode::None) noexcept
    {
      AnimationEffectTiming timing;
      timing.SpecifiedStartDelay = Seconds {1};
      timing.SpecifiedEndDelay = Seconds {0.5};
      timing.SpecifiedIterationDuration = Seconds {2};
      timing.Iterations = iterations;
      timing.Direction = direction;
      timing.Fill = fill;
      timing.UpdateComputedProperties(null, 1);
      return timing;
    }

    /// @brief The data of an animation started at time zero, at local time @p seconds.
    KRYS_NODISCARD AnimationEffectTiming::ResolutionData AtLocalTime(double seconds) noexcept
    {
      return {
        .TimelineTime = WebAnimationTime {Seconds {seconds}},
        .StartTime = WebAnimationTime {Seconds {0}},
        .LocalTime = WebAnimationTime {Seconds {seconds}},
        .PlaybackRate = 1,
      };
    }
  }

  TEST_CASE("AnimationEffectTiming: computed properties", "[Animation][AnimationEffectTiming]")
  {
    auto timing = MakeTiming(3);
    CHECK(timing.StartDelay == WebAnimationTime {Seconds {1}});
    CHECK(timing.EndDelay == WebAnimationTime {Seconds {0.5}});
    CHECK(timing.IterationDuration == WebAnimationTime {Seconds {2}});
    CHECK(timing.ActiveDuration == WebAnimationTime {Seconds {6}});
    CHECK(timing.EndTime == WebAnimationTime {Seconds {7.5}});

    SECTION("the playback rate doesn't change the active duration")
    {
      timing.UpdateComputedProperties(null, 2);
      CHECK(timing.ActiveDuration == WebAnimationTime {Seconds {6}});
      CHECK(timing.EndTime == WebAnimationTime {Seconds {7.5}});
    }
  }

  TEST_CASE("AnimationEffectTiming: phases", "[Animation][AnimationEffectTiming]")
  {
    auto timing = MakeTiming();

    CHECK(timing.GetBasicTiming({}).Phase == AnimationEffectPhase::Idle);
    CHECK(timing.GetBasicTiming(AtLocalTime(0.5)).Phase == AnimationEffectPhase::Before);
    CHECK(timing.GetBasicTiming(AtLocalTime(1.5)).Phase == AnimationEffectPhase::Active);
    CHECK(timing.GetBasicTiming(AtLocalTime(3.25)).Phase == AnimationEffectPhase::After);

    // The active interval excludes its end, unless asked to include it.
    CHECK(timing.GetBasicTiming(AtLocalTime(3)).Phase == AnimationEffectPhase::After);
    auto data = AtLocalTime(3);
    data.EndpointInclusiveActiveInterval = EndpointInclusiveActiveInterval::Yes;
    CHECK(timing.GetBasicTiming(data).Phase == AnimationEffectPhase::Active);
  }

  TEST_CASE("AnimationEffectTiming: progress", "[Animation][AnimationEffectTiming]")
  {
    SECTION("fill modes decide the progress outside the active interval")
    {
      CHECK_FALSE(MakeTiming().Resolve(AtLocalTime(0.5)).TransformedProgress);
      CHECK_FALSE(MakeTiming().Resolve(AtLocalTime(3.25)).TransformedProgress);
      CHECK(MakeTiming(1, PlaybackDirection::Normal, FillMode::Backwards)
              .Resolve(AtLocalTime(0.5))
              .TransformedProgress
            == 0);
      CHECK(MakeTiming(1, PlaybackDirection::Normal, FillMode::Forwards)
              .Resolve(AtLocalTime(3.25))
              .TransformedProgress
            == 1);
    }

    SECTION("iterations")
    {
      auto timing = MakeTiming(2.5);
      auto resolved = timing.Resolve(AtLocalTime(3.5));
      CHECK(resolved.CurrentIteration == 1);
      CHECK(resolved.TransformedProgress == Catch::Approx(0.25));
      CHECK(resolved.SimpleIterationProgress == Catch::Approx(0.25));
    }

    SECTION("directions")
    {
      CHECK(MakeTiming(2, PlaybackDirection::Reverse).Resolve(AtLocalTime(1.5)).TransformedProgress
            == Catch::Approx(0.75));
      CHECK(MakeTiming(2, PlaybackDirection::Alternate).Resolve(AtLocalTime(3.5)).TransformedProgress
            == Catch::Approx(0.75));
      CHECK(MakeTiming(2, PlaybackDirection::AlternateReverse).Resolve(AtLocalTime(1.5)).TransformedProgress
            == Catch::Approx(0.75));
    }

    SECTION("the timing function transforms the progress")
    {
      auto timing = MakeTiming();
      timing.TimingFunction = StepsTimingFunction::Create(2, StepsTimingFunction::StepPosition::End);
      CHECK(timing.Resolve(AtLocalTime(1.5)).TransformedProgress == 0);
      CHECK(timing.Resolve(AtLocalTime(2.5)).TransformedProgress == 0.5);
    }
  }
}
