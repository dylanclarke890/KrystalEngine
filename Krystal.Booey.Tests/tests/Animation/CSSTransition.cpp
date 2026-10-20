#include "Krystal.Booey/Animation/CSSTransition.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Animation/AnimationTestUtils.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/Animation/AnimationEffect.hpp"
#include "Krystal.Booey/Animation/CSSTransitionEvent.hpp"
#include "Krystal.Booey/Animation/DocumentTimeline.hpp"
#include "Krystal.Booey/Animation/KeyframeEffect.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Style/Styleable.hpp"

namespace krys::boo::tests
{
  namespace
  {
    using namespace std::chrono_literals;

    /// @brief A page to run transitions on.
    struct TransitionFixture : AnimationPageFixture
    {
      using AnimationPageFixture::AnimationPageFixture;

      KRYS_NODISCARD CSSTransition *RunningTransition(dom::DOMStringView id,
                                                      css::PropertyId property) noexcept
      {
        auto *transitions = ById(id).RunningTransitionsByProperty(null);
        if (!transitions)
        {
          return nullptr;
        }
        auto it = transitions->find(property);
        return it == transitions->end() ? nullptr : it->second.get();
      }
    };

    constexpr std::initializer_list<const char8_t *> TransitionEvents {
      u8"transitionrun", u8"transitionstart", u8"transitionend", u8"transitioncancel"};

    constexpr utf8_stringview FadingBox =
      u8"<!DOCTYPE html><style>#box{opacity:0;transition:opacity 1s linear}</style><div id=box></div>";
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "CSSTransition: a style change starts a transition",
                   "[Animation][CSSTransition]")
  {
    TransitionFixture fixture {utf8_string(FadingBox)};
    CHECK(fixture.Opacity(u8"box") == 0.0f);
    CHECK_FALSE(fixture.RunningTransition(u8"box", css::PropertyId::Opacity));

    fixture.SetStyle(u8"box", u8"opacity:1");
    fixture.Frame();

    auto *transition = fixture.RunningTransition(u8"box", css::PropertyId::Opacity);
    REQUIRE(transition);
    CHECK(transition->Property() == AnimatableCSSProperty {css::PropertyId::Opacity});
    CHECK(transition->TransitionProperty() == css::CSSOMStringAtom {u8"opacity"});
    CHECK(transition->OwningElement() == style::Styleable::FromElement(fixture.ById(u8"box")));
    CHECK(transition->Timeline() == &fixture.Document->Timeline());
    REQUIRE(transition->GetKeyframeEffect());
    CHECK(transition->GetKeyframeEffect()->AnimationType() == WebAnimationType::CSSTransition);
    CHECK(transition->GetKeyframeEffect()->Fill() == FillMode::Backwards);
    CHECK(transition->GetKeyframeEffect()->IterationDuration() == WebAnimationTime {Seconds {1}});

    // The transition runs from the before-change style.
    CHECK(fixture.Opacity(u8"box") == 0.0f);

    fixture.StartAnimations();
    CHECK(transition->GetPlayState() == WebAnimation::PlayState::Running);
    CHECK(transition->StartTime());
    CHECK(fixture.Opacity(u8"box") == 0.0f);

    fixture.Frame(250ms);
    CHECK(fixture.Opacity(u8"box") == Catch::Approx(0.25f));

    fixture.Frame(500ms);
    CHECK(fixture.Opacity(u8"box") == Catch::Approx(0.75f));

    // Once it ends, the element has the after-change style and the transition is no longer running.
    fixture.Frame(500ms);
    fixture.Frame();
    CHECK(fixture.Opacity(u8"box") == 1.0f);
    CHECK(transition->GetPlayState() == WebAnimation::PlayState::Finished);
    CHECK_FALSE(fixture.RunningTransition(u8"box", css::PropertyId::Opacity));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "CSSTransition: transition events", "[Animation][CSSTransition]")
  {
    TransitionFixture fixture {utf8_string(FadingBox)};
    auto recorder = CreateRef<AnimationEventRecorder>();
    recorder->Listen(fixture.ById(u8"box"), TransitionEvents);

    fixture.SetStyle(u8"box", u8"opacity:1");
    fixture.Frame();
    fixture.StartAnimations();
    CHECK(recorder->Records == List<std::string> {"transitionrun opacity", "transitionstart opacity"});

    fixture.Frame(1s);
    fixture.Frame();
    CHECK(recorder->Records
          == List<std::string> {"transitionrun opacity", "transitionstart opacity", "transitionend opacity"});
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "CSSTransition: a delayed transition",
                   "[Animation][CSSTransition]")
  {
    TransitionFixture fixture {
      u8"<!DOCTYPE html><style>#box{opacity:0;transition:opacity 1s linear 500ms}</style><div id=box></div>"};
    auto recorder = CreateRef<AnimationEventRecorder>();
    recorder->Listen(fixture.ById(u8"box"), TransitionEvents);

    fixture.SetStyle(u8"box", u8"opacity:1");
    fixture.Frame();
    fixture.StartAnimations();
    CHECK(recorder->Records == List<std::string> {"transitionrun opacity"});
    CHECK(fixture.Opacity(u8"box") == 0.0f);

    fixture.Frame(250ms);
    CHECK(fixture.Opacity(u8"box") == 0.0f);

    fixture.Frame(500ms);
    CHECK(fixture.Opacity(u8"box") == Catch::Approx(0.25f));
    CHECK(recorder->Records == List<std::string> {"transitionrun opacity", "transitionstart opacity"});
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "CSSTransition: changing the end value of a running transition",
                   "[Animation][CSSTransition]")
  {
    TransitionFixture fixture {utf8_string(FadingBox)};
    fixture.SetStyle(u8"box", u8"opacity:1");
    fixture.Frame();
    fixture.StartAnimations();
    fixture.Frame(500ms);
    REQUIRE(fixture.Opacity(u8"box") == Catch::Approx(0.5f));
    auto *firstTransition = fixture.RunningTransition(u8"box", css::PropertyId::Opacity);
    REQUIRE(firstTransition);
    Ref protectedFirstTransition = ShareRef(*firstTransition);

    SECTION("reversing it shortens the reversed transition")
    {
      fixture.SetStyle(u8"box", u8"opacity:0");
      fixture.Frame();

      auto *reversed = fixture.RunningTransition(u8"box", css::PropertyId::Opacity);
      REQUIRE(reversed);
      CHECK(reversed != firstTransition);
      CHECK(protectedFirstTransition->GetPlayState() == WebAnimation::PlayState::Idle);
      CHECK(reversed->ReversingShorteningFactor() == Catch::Approx(0.5));
      CHECK(reversed->GetKeyframeEffect()->IterationDuration() == WebAnimationTime {Seconds {0.5}});

      // It starts from the current value of the running transition.
      fixture.StartAnimations();
      CHECK(fixture.Opacity(u8"box") == Catch::Approx(0.5f));
      fixture.Frame(250ms);
      CHECK(fixture.Opacity(u8"box") == Catch::Approx(0.25f));
    }

    SECTION("going elsewhere starts a full transition from the current value")
    {
      fixture.SetStyle(u8"box", u8"opacity:0.75");
      fixture.Frame();

      auto *replacement = fixture.RunningTransition(u8"box", css::PropertyId::Opacity);
      REQUIRE(replacement);
      CHECK(replacement->ReversingShorteningFactor() == 1);
      CHECK(replacement->GetKeyframeEffect()->IterationDuration() == WebAnimationTime {Seconds {1}});

      fixture.StartAnimations();
      CHECK(fixture.Opacity(u8"box") == Catch::Approx(0.5f));
      fixture.Frame(500ms);
      CHECK(fixture.Opacity(u8"box") == Catch::Approx(0.625f));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "CSSTransition: canceling transitions",
                   "[Animation][CSSTransition]")
  {
    TransitionFixture fixture {utf8_string(FadingBox)};
    auto recorder = CreateRef<AnimationEventRecorder>();
    recorder->Listen(fixture.ById(u8"box"), TransitionEvents);
    fixture.SetStyle(u8"box", u8"opacity:1");
    fixture.Frame();
    fixture.StartAnimations();
    fixture.Frame(250ms);
    REQUIRE(fixture.RunningTransition(u8"box", css::PropertyId::Opacity));

    SECTION("by removing the matching transition-property")
    {
      fixture.SetStyle(u8"box", u8"opacity:1;transition:none");
      fixture.Frame();
      CHECK_FALSE(fixture.RunningTransition(u8"box", css::PropertyId::Opacity));
      CHECK(fixture.Opacity(u8"box") == 1.0f);
    }

    SECTION("by display:none")
    {
      fixture.SetStyle(u8"box", u8"opacity:1;display:none");
      fixture.Frame();
      CHECK_FALSE(fixture.RunningTransition(u8"box", css::PropertyId::Opacity));
    }

    fixture.Frame();
    CHECK(
      recorder->Records
      == List<std::string> {"transitionrun opacity", "transitionstart opacity", "transitioncancel opacity"});
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "CSSTransition: which changes start transitions",
                   "[Animation][CSSTransition]")
  {
    TransitionFixture fixture {
      u8"<!DOCTYPE html><style>#box{opacity:0;width:10px;transition:width 1s}</style>"
      u8"<div id=box></div>"};

    SECTION("only properties with a matching transition-property")
    {
      fixture.SetStyle(u8"box", u8"opacity:1;width:20px");
      fixture.Frame();
      CHECK(fixture.RunningTransition(u8"box", css::PropertyId::Width));
      CHECK_FALSE(fixture.RunningTransition(u8"box", css::PropertyId::Opacity));
    }

    SECTION("not a zero combined duration")
    {
      fixture.SetStyle(u8"box", u8"width:20px;transition:width 0s");
      fixture.Frame();
      CHECK_FALSE(fixture.RunningTransition(u8"box", css::PropertyId::Width));
    }

    SECTION("not discrete properties, unless transition-behavior allows them")
    {
      fixture.SetStyle(u8"box", u8"visibility:hidden;transition:visibility 1s");
      fixture.Frame();
      CHECK(fixture.RunningTransition(u8"box", css::PropertyId::Visibility));

      fixture.SetStyle(u8"box", u8"float:left;transition:float 1s");
      fixture.Frame();
      CHECK_FALSE(fixture.RunningTransition(u8"box", css::PropertyId::Float));

      fixture.SetStyle(u8"box", u8"float:right;transition:float 1s allow-discrete");
      fixture.Frame();
      CHECK(fixture.RunningTransition(u8"box", css::PropertyId::Float));
    }

    SECTION("shorthands transition their longhands")
    {
      fixture.SetStyle(u8"box", u8"margin:5px;transition:margin 1s");
      fixture.Frame();
      CHECK(fixture.RunningTransition(u8"box", css::PropertyId::MarginTop));
      CHECK(fixture.RunningTransition(u8"box", css::PropertyId::MarginLeft));
    }

    SECTION("all")
    {
      fixture.SetStyle(u8"box", u8"opacity:1;width:20px;transition:all 1s");
      fixture.Frame();
      CHECK(fixture.RunningTransition(u8"box", css::PropertyId::Width));
      CHECK(fixture.RunningTransition(u8"box", css::PropertyId::Opacity));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "CSSTransition: @starting-style", "[Animation][CSSTransition]")
  {
    TransitionFixture fixture {u8"<!DOCTYPE html><style>#box{opacity:1;transition:opacity 1s linear}"
                               u8"@starting-style{#box{opacity:0}}</style><div id=container></div>"};

    auto box = fixture.Document->CreateElement(u8"div");
    REQUIRE_FALSE(box.HasException());
    REQUIRE_FALSE(box.Value()->SetAttribute(u8"id", u8"box").HasException());
    REQUIRE_FALSE(fixture.ById(u8"container").AppendChild(*box.Value()).HasException());
    fixture.Frame();

    // An element without a before-change style transitions from its starting style.
    REQUIRE(fixture.RunningTransition(u8"box", css::PropertyId::Opacity));
    fixture.StartAnimations();
    fixture.Frame(500ms);
    CHECK(fixture.Opacity(u8"box") == Catch::Approx(0.5f));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "CSSTransition: removing the element cancels its transitions",
                   "[Animation][CSSTransition]")
  {
    TransitionFixture fixture {utf8_string(FadingBox)};
    fixture.SetStyle(u8"box", u8"opacity:1");
    fixture.Frame();
    Ref transition = ShareRef(*fixture.RunningTransition(u8"box", css::PropertyId::Opacity));
    Ref box = ShareRef(fixture.ById(u8"box"));

    box->Remove();
    CHECK(transition->GetPlayState() == WebAnimation::PlayState::Idle);
    CHECK_FALSE(transition->OwningElement());
    auto *runningTransitions = box->RunningTransitionsByProperty(null);
    CHECK((!runningTransitions || !runningTransitions->contains(css::PropertyId::Opacity)));
  }
}
