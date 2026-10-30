#include "Krystal.Booey/Animation/CSSAnimation.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Animation/AnimationTestUtils.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/Animation/KeyframeEffect.hpp"
#include "Krystal.Booey/Animation/KeyframeEffectStack.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Style/Styleable.hpp"

namespace krys::boo::tests
{
  namespace
  {
    using namespace std::chrono_literals;

    /// @brief A page to run CSS animations on.
    struct CSSAnimationFixture : AnimationPageFixture
    {
      using AnimationPageFixture::AnimationPageFixture;

      /// @brief The CSS animations `#id`'s style created, in its `animation-name` order.
      KRYS_NODISCARD List<Ref<CSSAnimation>> Animations(dom::DOMStringView id) noexcept
      {
        auto &animations = ById(id).AnimationsCreatedByMarkup(null);
        return {animations.begin(), animations.end()};
      }

      KRYS_NODISCARD CSSAnimation &OnlyAnimation(dom::DOMStringView id) noexcept
      {
        auto &animations = ById(id).AnimationsCreatedByMarkup(null);
        REQUIRE(animations.size() == 1);
        return *animations.first();
      }
    };

    constexpr std::initializer_list<const char8_t *> AnimationEvents {
      u8"animationstart", u8"animationiteration", u8"animationend", u8"animationcancel"};

    /// @brief `#box` with the `fade` animation (opacity 0 to 1), with @p animation as its `animation`.
    KRYS_NODISCARD utf8_string FadingBox(utf8_stringview animation) noexcept
    {
      utf8_string markup =
        u8"<!DOCTYPE html><style>@keyframes fade{from{opacity:0}to{opacity:1}}#box{animation:";
      markup += animation;
      markup += u8"}</style><div id=box></div>";
      return markup;
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "CSSAnimation: an animation from @keyframes",
                   "[Animation][CSSAnimation]")
  {
    CSSAnimationFixture fixture {FadingBox(u8"fade 1s linear")};
    auto &animation = fixture.OnlyAnimation(u8"box");
    CHECK(animation.AnimationName() == css::CSSOMStringAtom {u8"fade"});
    CHECK(animation.OwningElement() == style::Styleable::FromElement(fixture.ById(u8"box")));
    REQUIRE(animation.GetKeyframeEffect());
    CHECK(animation.GetKeyframeEffect()->AnimationType() == WebAnimationType::CSSAnimation);
    CHECK(animation.GetKeyframeEffect()->GetBlendingKeyframes().Size() == 2);
    CHECK(animation.GetKeyframeEffect()->IterationDuration() == WebAnimationTime {Seconds {1}});
    CHECK(animation.Pending());

    fixture.StartAnimations();
    CHECK(animation.GetPlayState() == WebAnimation::PlayState::Running);
    CHECK(fixture.Opacity(u8"box") == 0.0f);

    fixture.Frame(250ms);
    CHECK(fixture.Opacity(u8"box") == Catch::Approx(0.25f));
    fixture.Frame(500ms);
    CHECK(fixture.Opacity(u8"box") == Catch::Approx(0.75f));

    // Without a fill mode, the element goes back to its own style.
    fixture.Frame(500ms);
    fixture.Frame();
    CHECK(animation.GetPlayState() == WebAnimation::PlayState::Finished);
    CHECK(fixture.Opacity(u8"box") == 1.0f);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "CSSAnimation: animation events", "[Animation][CSSAnimation]")
  {
    // The animation starts in the rendering update after its style is resolved, so the listener is added
    // first.
    CSSAnimationFixture fixture {FadingBox(u8"none")};
    auto recorder = CreateRef<AnimationEventRecorder>();
    recorder->Listen(fixture.ById(u8"box"), AnimationEvents);
    fixture.SetStyle(u8"box", u8"animation:fade 1s linear 2");
    fixture.Frame();

    fixture.StartAnimations();
    CHECK(recorder->Records == List<std::string> {"animationstart fade"});

    fixture.Frame(1'250ms);
    fixture.Frame();
    CHECK(recorder->Records == List<std::string> {"animationstart fade", "animationiteration fade"});

    fixture.Frame(1s);
    fixture.Frame();
    CHECK(recorder->Records
          == List<std::string> {"animationstart fade", "animationiteration fade", "animationend fade"});
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "CSSAnimation: timing properties", "[Animation][CSSAnimation]")
  {
    SECTION("iteration count and direction")
    {
      CSSAnimationFixture fixture {FadingBox(u8"fade 1s linear 2 alternate")};
      fixture.StartAnimations();
      fixture.Frame(1'250ms);
      CHECK(fixture.Opacity(u8"box") == Catch::Approx(0.75f));
    }

    SECTION("infinite iterations")
    {
      CSSAnimationFixture fixture {FadingBox(u8"fade 1s linear infinite")};
      CHECK(std::isinf(fixture.OnlyAnimation(u8"box").Effect()->Iterations()));
      fixture.StartAnimations();
      fixture.Frame(5'250ms);
      CHECK(fixture.Opacity(u8"box") == Catch::Approx(0.25f));
    }

    SECTION("a delay, with backwards fill")
    {
      CSSAnimationFixture fixture {FadingBox(u8"fade 1s linear 500ms backwards")};
      fixture.SetStyle(u8"box", u8"opacity:0.5");
      fixture.StartAnimations();
      CHECK(fixture.Opacity(u8"box") == 0.0f);
      fixture.Frame(750ms);
      CHECK(fixture.Opacity(u8"box") == Catch::Approx(0.25f));
    }

    SECTION("forwards fill")
    {
      CSSAnimationFixture fixture {u8"<!DOCTYPE html><style>@keyframes fade{from{opacity:1}to{opacity:0.5}}"
                                   u8"#box{animation:fade 1s linear forwards}</style><div id=box></div>"};
      fixture.StartAnimations();
      fixture.Frame(2s);
      CHECK(fixture.Opacity(u8"box") == 0.5f);
    }

    SECTION("steps()")
    {
      CSSAnimationFixture fixture {FadingBox(u8"fade 1s steps(4)")};
      fixture.StartAnimations();
      fixture.Frame(300ms);
      CHECK(fixture.Opacity(u8"box") == Catch::Approx(0.25f));
      fixture.Frame(400ms);
      CHECK(fixture.Opacity(u8"box") == Catch::Approx(0.5f));
    }

    SECTION("a keyframe's timing function")
    {
      CSSAnimationFixture fixture {
        u8"<!DOCTYPE html><style>@keyframes fade{from{opacity:0;animation-timing-function:steps(2)}"
        u8"to{opacity:1}}#box{animation:fade 1s linear}</style><div id=box></div>"};
      fixture.StartAnimations();
      fixture.Frame(400ms);
      CHECK(fixture.Opacity(u8"box") == 0.0f);
      fixture.Frame(200ms);
      CHECK(fixture.Opacity(u8"box") == Catch::Approx(0.5f));
    }

    SECTION("an implicit keyframe from the element's style")
    {
      CSSAnimationFixture fixture {u8"<!DOCTYPE html><style>@keyframes fade{to{opacity:1}}"
                                   u8"#box{opacity:0.5;animation:fade 1s linear}</style><div id=box></div>"};
      fixture.StartAnimations();
      fixture.Frame(500ms);
      CHECK(fixture.Opacity(u8"box") == Catch::Approx(0.75f));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "CSSAnimation: the play state", "[Animation][CSSAnimation]")
  {
    CSSAnimationFixture fixture {FadingBox(u8"fade 1s linear paused")};
    auto &animation = fixture.OnlyAnimation(u8"box");
    fixture.StartAnimations();
    CHECK(animation.GetPlayState() == WebAnimation::PlayState::Paused);
    fixture.Frame(500ms);
    CHECK(fixture.Opacity(u8"box") == 0.0f);

    fixture.SetStyle(u8"box", u8"animation-play-state:running");
    fixture.Frame();
    CHECK(&fixture.OnlyAnimation(u8"box") == &animation);
    fixture.StartAnimations();
    fixture.Frame(250ms);
    CHECK(fixture.Opacity(u8"box") == Catch::Approx(0.25f));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "CSSAnimation: changing animation-name",
                   "[Animation][CSSAnimation]")
  {
    CSSAnimationFixture fixture {u8"<!DOCTYPE html><style>@keyframes fade{from{opacity:0}to{opacity:1}}"
                                 u8"@keyframes dim{from{opacity:1}to{opacity:0}}#box{animation:fade 1s "
                                 u8"linear}</style><div id=box></div>"};
    auto recorder = CreateRef<AnimationEventRecorder>();
    recorder->Listen(fixture.ById(u8"box"), AnimationEvents);
    Ref fade = ShareRef(fixture.OnlyAnimation(u8"box"));
    fixture.StartAnimations();
    fixture.Frame(250ms);

    SECTION("to another name cancels the old animation and creates one")
    {
      fixture.SetStyle(u8"box", u8"animation-name:dim");
      fixture.Frame();
      auto &dim = fixture.OnlyAnimation(u8"box");
      CHECK(dim.AnimationName() == css::CSSOMStringAtom {u8"dim"});
      CHECK(fade->GetPlayState() == WebAnimation::PlayState::Idle);
      CHECK_FALSE(fade->OwningElement());
      fixture.StartAnimations();
      CHECK(recorder->Records == List<std::string> {"animationcancel fade", "animationstart dim"});
      fixture.Frame(250ms);
      CHECK(fixture.Opacity(u8"box") == Catch::Approx(0.75f));
    }

    SECTION("adding a name keeps the existing animation")
    {
      fixture.SetStyle(u8"box", u8"animation:dim 1s linear, fade 1s linear");
      fixture.Frame();
      // The collection is filled from the end of animation-name.
      auto animations = fixture.Animations(u8"box");
      REQUIRE(animations.size() == 2);
      CHECK(animations[0].get() == fade.get());
      CHECK(animations[1]->AnimationName() == css::CSSOMStringAtom {u8"dim"});

      // The animation later in animation-name is applied last: it wins.
      fixture.StartAnimations();
      CHECK(fixture.Opacity(u8"box") == Catch::Approx(0.25f));
    }

    SECTION("to none cancels it")
    {
      fixture.SetStyle(u8"box", u8"animation:none");
      fixture.Frame();
      CHECK(fixture.Animations(u8"box").empty());
      CHECK(fade->GetPlayState() == WebAnimation::PlayState::Idle);
      CHECK(fixture.Opacity(u8"box") == 1.0f);
    }

    SECTION("display:none cancels it")
    {
      fixture.SetStyle(u8"box", u8"display:none");
      fixture.Frame();
      CHECK(fixture.Animations(u8"box").empty());
      CHECK(fade->GetPlayState() == WebAnimation::PlayState::Idle);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "CSSAnimation: @keyframes rules changing",
                   "[Animation][CSSAnimation]")
  {
    CSSAnimationFixture fixture {u8"<!DOCTYPE html><style id=sheet></style><style>#box{animation:fade 1s "
                                 u8"linear}</style><div id=box></div>"};

    // A name without a rule creates no animation...
    CHECK(fixture.Animations(u8"box").empty());
    CHECK(fixture.ById(u8"box").GetKeyframeEffectStack(null)->ContainsInvalidCSSAnimationName(
      css::CSSOMString {u8"fade"}));

    // ...until the rule is added.
    auto &sheet = fixture.ById(u8"sheet");
    REQUIRE_FALSE(
      sheet.AppendChild(*fixture.Document->CreateTextNode(u8"@keyframes fade{from{opacity:0}to{opacity:1}}"))
        .HasException());
    fixture.Frame();
    auto &animation = fixture.OnlyAnimation(u8"box");
    fixture.StartAnimations();
    fixture.Frame(500ms);
    CHECK(fixture.Opacity(u8"box") == Catch::Approx(0.5f));

    // Changing the rule changes the running animation's keyframes.
    REQUIRE_FALSE(sheet.TextContent(u8"@keyframes fade{from{opacity:1}to{opacity:0}}").HasException());
    fixture.Frame();
    CHECK(&fixture.OnlyAnimation(u8"box") == &animation);
    CHECK(fixture.Opacity(u8"box") == Catch::Approx(0.5f));
    fixture.Frame(250ms);
    CHECK(fixture.Opacity(u8"box") == Catch::Approx(0.25f));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "CSSAnimation: transitions and animations together",
                   "[Animation][CSSAnimation]")
  {
    CSSAnimationFixture fixture {
      u8"<!DOCTYPE html><style>@keyframes grow{from{width:10px}to{width:20px}}"
      u8"#box{width:10px;opacity:0;animation:grow 1s linear;transition:opacity 1s linear}</style>"
      u8"<div id=box></div>"};
    fixture.StartAnimations();

    // A transition on another property runs alongside the animation; composite order puts it first.
    fixture.SetStyle(u8"box", u8"opacity:1");
    fixture.Frame();
    auto *stack = fixture.ById(u8"box").GetKeyframeEffectStack(null);
    REQUIRE(stack);
    REQUIRE(stack->SortedEffects().size() == 2);
    CHECK(stack->SortedEffects()[0]->Animation()->IsCSSTransition());
    CHECK(stack->SortedEffects()[1]->Animation()->IsCSSAnimation());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "CSSAnimation: properties C++ overrides stop following the style",
                   "[Animation][CSSAnimation]")
  {
    CSSAnimationFixture fixture {FadingBox(u8"fade 1s linear")};
    auto &animation = fixture.OnlyAnimation(u8"box");
    fixture.StartAnimations();

    SECTION("pausing")
    {
      REQUIRE_FALSE(static_cast<WebAnimation &>(animation).BindingsPause().HasException());
      fixture.Frame();
      CHECK(animation.GetPlayState() == WebAnimation::PlayState::Paused);
      // animation-play-state no longer plays or pauses it.
      fixture.SetStyle(u8"box", u8"animation-play-state: paused");
      fixture.Frame();
      fixture.SetStyle(u8"box", u8"animation-play-state: running");
      fixture.Frame();
      CHECK(animation.GetPlayState() == WebAnimation::PlayState::Paused);
    }

    SECTION("updateTiming()")
    {
      auto &effect = *animation.GetKeyframeEffect();
      REQUIRE_FALSE(
        effect
          .BindingsUpdateTiming(*fixture.Document,
                                OptionalEffectTiming {.Duration = Variant<double, dom::DOMString> {4'000.0}})
          .HasException());
      fixture.SetStyle(u8"box", u8"animation-duration: 2s");
      fixture.Frame();
      CHECK(effect.SpecifiedIterationDuration() == Seconds {4});
      // Properties it didn't give still follow the style.
      fixture.SetStyle(u8"box", u8"animation-duration: 2s; animation-iteration-count: 3");
      fixture.Frame();
      CHECK(effect.Iterations() == 3);
    }

    SECTION("setKeyframes()")
    {
      auto &effect = *animation.GetKeyframeEffect();
      InputKeyframe keyframe;
      keyframe.Properties = {{u8"opacity", u8"0.5"}};
      REQUIRE_FALSE(effect.SetBindingsKeyframes(*fixture.Document, List<InputKeyframe> {keyframe, keyframe})
                      .HasException());
      fixture.Frame();
      CHECK(fixture.Opacity(u8"box") == Catch::Approx(0.5f));
    }
  }
}
