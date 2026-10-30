#include "Krystal.Booey/Animation/KeyframeInterpolation.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Animation/WebAnimationTime.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::tests
{
  namespace
  {
    struct TestKeyframe final : KeyframeInterpolation::Keyframe
    {
      double KeyOffset;
      bool Animates {true};
      Maybe<boo::CompositeOperation> Operation;

      explicit TestKeyframe(double offset, bool animates = true) noexcept
          : KeyOffset(offset), Animates(animates)
      {
      }

      KRYS_NODISCARD double Offset() const noexcept override
      {
        return KeyOffset;
      }
      KRYS_NODISCARD Maybe<boo::CompositeOperation> CompositeOperation() const noexcept override
      {
        return Operation;
      }
      KRYS_NODISCARD bool AnimatesProperty(KeyframeInterpolation::Property) const noexcept override
      {
        return Animates;
      }
    };

    struct TestInterpolation final : KeyframeInterpolation
    {
      List<UniquePtr<TestKeyframe>> Keyframes;
      RefPtr<TimingFunction> Timing;
      boo::CompositeOperation Operation {boo::CompositeOperation::Replace};

      KRYS_NODISCARD boo::CompositeOperation CompositeOperation() const noexcept override
      {
        return Operation;
      }
      KRYS_NODISCARD bool IsPropertyAdditiveOrCumulative(Property) const noexcept override
      {
        return true;
      }
      KRYS_NODISCARD const Keyframe &KeyframeAtIndex(size_t index) const noexcept override
      {
        return *Keyframes[index];
      }
      KRYS_NODISCARD size_t NumberOfKeyframes() const noexcept override
      {
        return Keyframes.size();
      }
      KRYS_NODISCARD const TimingFunction *TimingFunctionForKeyframe(const Keyframe &) const noexcept override
      {
        return Timing.get();
      }
    };

    const AnimatableCSSProperty Property {css::PropertyId::Opacity};
  }

  TEST_CASE("KeyframeInterpolation: interval endpoints", "[Animation][KeyframeInterpolation]")
  {
    TestInterpolation interpolation;
    interpolation.Keyframes.push_back(CreateUnique<TestKeyframe>(0.25));
    interpolation.Keyframes.push_back(CreateUnique<TestKeyframe>(0.5));
    interpolation.Keyframes.push_back(CreateUnique<TestKeyframe>(0.75, false));
    TestKeyframe start(0);
    TestKeyframe end(1);

    // The implicit 0% and 100% keyframes stand in where no keyframe has those offsets.
    auto interval = interpolation.InterpolationKeyframes(Property, 0.1, start, end);
    REQUIRE(interval.Endpoints.size() == 2);
    CHECK(interval.Endpoints[0] == &start);
    CHECK(interval.Endpoints[1]->Offset() == 0.25);
    CHECK(interval.HasImplicitZeroKeyframe);
    CHECK(interval.HasImplicitOneKeyframe);

    // Keyframes that don't animate the property are skipped.
    interval = interpolation.InterpolationKeyframes(Property, 0.6, start, end);
    REQUIRE(interval.Endpoints.size() == 2);
    CHECK(interval.Endpoints[0]->Offset() == 0.5);
    CHECK(interval.Endpoints[1] == &end);

    // Progress outside [0, 1] uses the first or last interval.
    interval = interpolation.InterpolationKeyframes(Property, -0.5, start, end);
    CHECK(interval.Endpoints[0] == &start);
    interval = interpolation.InterpolationKeyframes(Property, 1.5, start, end);
    CHECK(interval.Endpoints[1] == &end);

    // No keyframe animates the property: no interval.
    TestInterpolation empty;
    CHECK(empty.InterpolationKeyframes(Property, 0.5, start, end).Endpoints.empty());
  }

  TEST_CASE("KeyframeInterpolation: interval progress, timing functions and composition",
            "[Animation][KeyframeInterpolation]")
  {
    TestInterpolation interpolation;
    interpolation.Keyframes.push_back(CreateUnique<TestKeyframe>(0));
    interpolation.Keyframes.push_back(CreateUnique<TestKeyframe>(0.5));
    interpolation.Keyframes.push_back(CreateUnique<TestKeyframe>(1));
    interpolation.Keyframes[1]->Operation = CompositeOperation::Add;
    TestKeyframe start(0);
    TestKeyframe end(1);

    double progress = -1;
    List<const KeyframeInterpolation::Keyframe *> composed;
    auto interpolate = [&](double iterationProgress)
    {
      composed.clear();
      auto interval = interpolation.InterpolationKeyframes(Property, iterationProgress, start, end);
      interpolation.InterpolateKeyframes(
        Property, interval, iterationProgress, 0, Seconds {1}, TimingFunction::Before::No,
        [&](const KeyframeInterpolation::Keyframe &keyframe, CompositeOperation)
        { composed.push_back(&keyframe); }, [](const KeyframeInterpolation::Keyframe &) {},
        [&](double intervalProgress, double, IterationCompositeOperation) { progress = intervalProgress; },
        [] { return false; });
    };

    interpolate(0.75);
    CHECK(progress == Catch::Approx(0.5));
    // The keyframe with an add composite operation is composed with the underlying value.
    REQUIRE(composed.size() == 1);
    CHECK(composed[0]->Offset() == 0.5);

    // A timing function transforms the interval progress.
    interpolation.Timing =
      RefPtr<TimingFunction> {StepsTimingFunction::Create(2, StepsTimingFunction::StepPosition::End)};
    interpolate(0.6);
    CHECK(progress == Catch::Approx(0));
    interpolate(0.8);
    CHECK(progress == Catch::Approx(0.5));
  }

  TEST_CASE("WebAnimationTime: times and percentages", "[Animation][WebAnimationTime]")
  {
    WebAnimationTime time {Seconds {1.5}};
    CHECK(time.Time() == Seconds {1.5});
    CHECK_FALSE(time.Percentage());
    CHECK(WebAnimationTime::FromMilliseconds(500) + WebAnimationTime::FromMilliseconds(250)
          == Seconds {0.75});
    CHECK(time / WebAnimationTime {Seconds {0.5}} == 3);
    CHECK(time.MatchingZero().IsZero());
    CHECK(time.MatchingInfinity().IsInfinity());
    CHECK(time.ApproximatelyEqualTo(WebAnimationTime {Seconds {1.5000001}}));
    CHECK(time.ApproximatelyLessThan(WebAnimationTime {Seconds {1.6}}));
    CHECK(WebAnimationTime {Seconds {1}} < time);

    auto percentage = WebAnimationTime::FromPercentage(50);
    CHECK(percentage.Percentage() == 50);
    CHECK_FALSE(percentage.Time());
    CHECK(percentage.MatchingEpsilon().Percentage() == 0.000001);
    CHECK_FALSE(WebAnimationTime {}.IsValid());
  }
}
