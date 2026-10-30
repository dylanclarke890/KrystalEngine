#include "Krystal.Booey/Animation/AnimationEffect.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Animation/AnimationTestUtils.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/Animation/KeyframeEffect.hpp"
#include "Krystal.Booey/Animation/WebAnimation.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSUnitValue.hpp"
#include <cmath>
#include <limits>

namespace krys::boo::tests
{
  namespace
  {
    KRYS_NODISCARD std::string Narrow(const dom::DOMString &string)
    {
      return std::string(string.begin(), string.end());
    }

    /// @brief A page with `#box`, and a keyframe effect on it with no keyframes.
    struct EffectFixture : AnimationPageFixture
    {
      Ref<KeyframeEffect> Effect;

      EffectFixture() noexcept
          : AnimationPageFixture(u8"<!DOCTYPE html><div id=box></div>"),
            Effect(KeyframeEffect::Create(ById(u8"box"), null))
      {
      }
    };
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "AnimationEffect: getTiming() and updateTiming()",
                   "[Animation][AnimationEffect]")
  {
    EffectFixture fixture;
    auto &effect = *fixture.Effect;

    SECTION("the initial timing")
    {
      auto timing = effect.GetBindingsTiming();
      CHECK(std::get<dom::DOMString>(timing.Duration) == u8"auto");
      CHECK(timing.Delay == 0);
      CHECK(timing.EndDelay == 0);
      CHECK(timing.IterationStart == 0);
      CHECK(timing.Iterations == 1);
      CHECK(Narrow(timing.Easing) == "linear");
      CHECK(timing.Fill == FillMode::Auto);
      CHECK(timing.Direction == PlaybackDirection::Normal);
    }

    SECTION("the members given are updated")
    {
      REQUIRE_FALSE(effect
                      .BindingsUpdateTiming(*fixture.Document,
                                            OptionalEffectTiming {
                                              .Duration = Variant<double, dom::DOMString> {2'000.0},
                                              .Iterations = 3,
                                              .Delay = 500,
                                              .Easing = dom::DOMString(u8"ease-in"),
                                              .Fill = FillMode::Both,
                                              .Direction = PlaybackDirection::Alternate,
                                            })
                      .HasException());
      auto timing = effect.GetBindingsTiming();
      CHECK(std::get<double>(timing.Duration) == 2'000);
      CHECK(timing.Iterations == 3);
      CHECK(timing.Delay == 500);
      CHECK(timing.EndDelay == 0);
      CHECK(Narrow(timing.Easing) == "ease-in");
      CHECK(timing.Fill == FillMode::Both);
      CHECK(timing.Direction == PlaybackDirection::Alternate);
      CHECK(effect.SpecifiedIterationDuration() == Seconds {2});

      // `auto` makes the duration unspecified again.
      REQUIRE_FALSE(effect
                      .UpdateTiming(*fixture.Document,
                                    OptionalEffectTiming {
                                      .Duration = Variant<double, dom::DOMString> {dom::DOMString(u8"auto")}})
                      .HasException());
      CHECK_FALSE(effect.SpecifiedIterationDuration());
      CHECK(effect.Iterations() == 3);

      // Nothing to update.
      CHECK_FALSE(effect.UpdateTiming(*fixture.Document, null).HasException());
    }

    SECTION("invalid values throw a TypeError and change nothing")
    {
      auto expectTypeError = [&](OptionalEffectTiming &&timing)
      {
        auto result = effect.UpdateTiming(*fixture.Document, timing);
        REQUIRE(result.HasException());
        CHECK(result.GetException().Code() == dom::ExceptionCode::TypeError);
      };
      expectTypeError({.IterationStart = -1});
      expectTypeError({.Iterations = -1});
      expectTypeError({.Iterations = std::numeric_limits<double>::quiet_NaN()});
      expectTypeError({.Duration = Variant<double, dom::DOMString> {-1.0}});
      expectTypeError({.Duration = Variant<double, dom::DOMString> {dom::DOMString(u8"lemon")}});
      expectTypeError({.Iterations = 2, .Easing = dom::DOMString(u8"bounce")});
      CHECK(effect.Iterations() == 1);
      CHECK(Narrow(effect.GetBindingsTiming().Easing) == "linear");

      CHECK(effect.SetIterations(-2).HasException());
      CHECK(effect.SetIterationStart(-2).HasException());
    }

    SECTION("getComputedTiming()")
    {
      REQUIRE_FALSE(effect
                      .UpdateTiming(*fixture.Document,
                                    OptionalEffectTiming {
                                      .Duration = Variant<double, dom::DOMString> {1'000.0},
                                      .Easing = dom::DOMString(u8"steps(4)"),
                                    })
                      .HasException());
      auto computed = effect.GetBindingsComputedTiming();
      CHECK(std::get<double>(computed.Duration) == 1'000);
      // `auto` fill computes to `none`.
      CHECK(computed.Fill == FillMode::None);
      CHECK(Narrow(computed.Easing) == "steps(4)");
      // Without an animation, the effect is idle.
      CHECK(computed.Phase == AnimationEffectPhase::Idle);
      CHECK_FALSE(computed.LocalTime);
      CHECK_FALSE(computed.Progress);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "AnimationEffect: computed timing of a playing animation",
                   "[Animation][AnimationEffect]")
  {
    using namespace std::chrono_literals;

    EffectFixture fixture;
    REQUIRE_FALSE(
      fixture.Effect
        ->UpdateTiming(*fixture.Document,
                       OptionalEffectTiming {.Duration = Variant<double, dom::DOMString> {1'000.0}})
        .HasException());
    auto animation = WebAnimation::Create(*fixture.Document, fixture.Effect.get());
    REQUIRE_FALSE(animation->Play().HasException());
    fixture.StartAnimations();
    fixture.Frame(250ms);

    auto computed = fixture.Effect->GetBindingsComputedTiming();
    CHECK(computed.Phase == AnimationEffectPhase::Active);
    REQUIRE(computed.Progress);
    CHECK(*computed.Progress > 0.2);
    CHECK(*computed.Progress < 0.3);
    CHECK(computed.CurrentIteration == 0.0);
    animation->Cancel(WebAnimation::Silently::Yes);
  }
}
