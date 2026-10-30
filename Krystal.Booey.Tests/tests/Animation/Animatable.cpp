#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Animation/AnimationTestUtils.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/Animation/CSSAnimation.hpp"
#include "Krystal.Booey/Animation/DocumentTimeline.hpp"
#include "Krystal.Booey/Animation/KeyframeEffect.hpp"
#include "Krystal.Booey/Animation/WebAnimation.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSNumericFactory.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Types/DOMHighResTimeStamp.hpp"
#include <type_traits>

namespace krys::boo::tests
{
  namespace
  {
    using namespace std::chrono_literals;

    KRYS_NODISCARD InputKeyframe Opacity(KeyframeOffset offset, dom::DOMString value) noexcept
    {
      InputKeyframe keyframe;
      keyframe.Offset = krys::move(offset);
      keyframe.Properties = {{u8"opacity", krys::move(value)}};
      return keyframe;
    }

    KRYS_NODISCARD KeyframesInput Fade() noexcept
    {
      return List<InputKeyframe> {Opacity(nullptr, u8"0"), Opacity(nullptr, u8"1")};
    }

    KRYS_NODISCARD Ref<WebAnimation>
      Animate(dom::Element &element, KeyframesInput &&keyframes,
              Maybe<Variant<double, KeyframeAnimationOptions>> &&options = 1'000.0)
    {
      auto result = element.Animate(krys::move(keyframes), krys::move(options));
      REQUIRE(result.HasValue());
      return result.ReleaseValue();
    }

    /// @brief Records the outcomes of promises: the animation's ID when fulfilled, the exception's name when
    /// rejected.
    struct PromiseRecorder
    {
      List<std::string> Records;

      void Watch(AnimationPromise &promise) noexcept
      {
        promise.Then(
          [this](WebAnimation &animation)
          { Records.push_back("fulfilled " + std::string(animation.Id().begin(), animation.Id().end())); },
          [this](const dom::Exception &exception)
          {
            Records.push_back(exception.Code() == dom::ExceptionCode::AbortError ? "AbortError" : "rejected");
          });
      }
    };
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Animatable: animate()", "[Animation][Animatable]")
  {
    AnimationPageFixture fixture {u8"<!DOCTYPE html><div id=box></div>"};
    auto &box = fixture.ById(u8"box");

    KeyframeAnimationOptions options;
    options.Id = u8"fade";
    options.Duration = 1'000.0;
    auto animation = Animate(box, Fade(), options);
    CHECK(animation->Id() == u8"fade");
    CHECK(animation->Timeline() == &fixture.Document->Timeline());
    CHECK(animation->GetPlayState() == WebAnimation::PlayState::Running);
    CHECK(animation->Pending());
    REQUIRE(animation->GetKeyframeEffect());
    CHECK(animation->GetKeyframeEffect()->Target() == &box);

    fixture.StartAnimations();
    CHECK_FALSE(animation->Pending());
    fixture.Frame(500ms);
    CHECK(fixture.Opacity(u8"box") == Catch::Approx(0.5f));

    SECTION("a null timeline")
    {
      KeyframeAnimationOptions noTimeline;
      noTimeline.Timeline = RefPtr<AnimationTimeline> {};
      auto unattached = Animate(box, Fade(), noTimeline);
      CHECK_FALSE(unattached->Timeline());
      unattached->Cancel(WebAnimation::Silently::Yes);
    }

    SECTION("invalid keyframes")
    {
      auto result = box.Animate(List<InputKeyframe> {Opacity(2.0, u8"0")});
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == dom::ExceptionCode::TypeError);
    }

    animation->Cancel(WebAnimation::Silently::Yes);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Animatable: getAnimations()", "[Animation][Animatable]")
  {
    AnimationPageFixture fixture {
      u8"<!DOCTYPE html><style>@keyframes spin{to{opacity:0}}#inner{animation:spin 1s}"
      u8"</style><div id=outer><div id=inner></div></div><div id=host></div>"};
    auto &outer = fixture.ById(u8"outer");
    auto &inner = fixture.ById(u8"inner");
    auto first = Animate(outer, Fade());
    auto second = Animate(inner, Fade());

    // The CSS animation comes first in composite order, then the script animations in the order they were
    // made.
    auto all = fixture.Document->GetAnimations();
    REQUIRE(all.size() == 3);
    CHECK(Is<CSSAnimation>(*all[0]));
    CHECK(all[1] == first.get());
    CHECK(all[2] == second.get());

    auto onOuter = outer.GetAnimations();
    REQUIRE(onOuter.size() == 1);
    CHECK(onOuter[0] == first.get());
    CHECK(outer.GetAnimations(GetAnimationsOptions {.Subtree = true}).size() == 3);
    CHECK(inner.GetAnimations().size() == 2);

    // A finished animation isn't relevant, unless it fills.
    REQUIRE_FALSE(first->Finish().HasException());
    CHECK(outer.GetAnimations().empty());

    SECTION("shadow trees")
    {
      auto &host = fixture.ById(u8"host");
      auto shadowRoot = host.AttachShadow({});
      REQUIRE(shadowRoot.HasValue());
      auto shadowChild = fixture.Document->CreateElement(u8"div").ReleaseValue();
      REQUIRE_FALSE(shadowRoot.Value()->AppendChild(*shadowChild).HasException());
      auto inShadow = Animate(*shadowChild, Fade());
      auto shadowAnimations = shadowRoot.Value()->GetAnimations();
      REQUIRE(shadowAnimations.size() == 1);
      CHECK(shadowAnimations[0] == inShadow.get());
      // The document's animations leave out those in shadow trees.
      CHECK(fixture.Document->GetAnimations().size() == 2);
      inShadow->Cancel(WebAnimation::Silently::Yes);
    }

    second->Cancel(WebAnimation::Silently::Yes);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Animatable: the ready and finished promises",
                   "[Animation][Animatable]")
  {
    AnimationPageFixture fixture {u8"<!DOCTYPE html><div id=box></div>"};
    KeyframeAnimationOptions options;
    options.Id = u8"a";
    options.Duration = 500.0;
    auto animation = Animate(fixture.ById(u8"box"), Fade(), options);

    PromiseRecorder recorder;
    Ref ready = ShareRef(animation->Ready());
    recorder.Watch(*ready);
    recorder.Watch(animation->Finished());
    CHECK(ready->GetStatus() == AnimationPromise::Status::Pending);

    // The ready promise resolves once the animation is ready, the finished promise once it has finished.
    fixture.StartAnimations();
    CHECK(ready->GetStatus() == AnimationPromise::Status::Fulfilled);
    CHECK(recorder.Records == List<std::string> {"fulfilled a"});
    fixture.Frame(600ms);
    fixture.Frame();
    CHECK(recorder.Records == List<std::string> {"fulfilled a", "fulfilled a"});

    // Playing again replaces the finished promise; canceling rejects it.
    Ref finished = ShareRef(animation->Finished());
    REQUIRE_FALSE(animation->Play().HasException());
    CHECK(&animation->Finished() != finished.get());
    CHECK(finished->GetStatus() == AnimationPromise::Status::Fulfilled);
    recorder.Watch(animation->Finished());
    animation->Cancel();
    fixture.Frame();
    CHECK(recorder.Records == List<std::string> {"fulfilled a", "fulfilled a", "AbortError"});
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Animatable: times from C++ are validated",
                   "[Animation][Animatable]")
  {
    AnimationPageFixture fixture {u8"<!DOCTYPE html><div id=box></div>"};
    auto animation = Animate(fixture.ById(u8"box"), Fade());
    fixture.StartAnimations();

    // A monotonic timeline takes times, not percentages.
    CHECK(animation->SetBindingsCurrentTime(WebAnimationTime::FromPercentage(50)).HasException());
    CHECK(animation->SetBindingsStartTime(WebAnimationTime::FromPercentage(50)).HasException());
    REQUIRE_FALSE(animation->SetBindingsCurrentTime(WebAnimationTime::FromMilliseconds(250)).HasException());
    REQUIRE(animation->BindingsCurrentTime());
    CHECK(animation->BindingsCurrentTime()->Time()->count() == Catch::Approx(0.25));
    // An unresolved time can't replace a resolved one.
    CHECK(animation->SetBindingsCurrentTime(null).HasException());

    animation->Cancel(WebAnimation::Silently::Yes);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Animatable: commitStyles()", "[Animation][Animatable]")
  {
    AnimationPageFixture fixture {u8"<!DOCTYPE html><div id=box style='color: red'></div>"};
    auto &box = fixture.ById(u8"box");
    KeyframeAnimationOptions options;
    options.Duration = 1'000.0;
    options.Fill = FillMode::Forwards;
    auto animation = Animate(box, Fade(), options);
    fixture.StartAnimations();
    REQUIRE_FALSE(animation->Finish().HasException());
    fixture.Frame();

    REQUIRE_FALSE(animation->CommitStyles().HasException());
    auto style = box.GetAttribute(u8"style");
    REQUIRE(style);
    CHECK(std::string(style->begin(), style->end()) == "color: red; opacity: 1;");

    // Without the animation, the committed opacity stays.
    animation->Cancel();
    fixture.Frame();
    CHECK(fixture.Opacity(u8"box") == 1.0f);

    SECTION("a pseudo-element can't have a style attribute")
    {
      KeyframeAnimationOptions pseudoOptions;
      pseudoOptions.PseudoElement = u8"::before";
      auto onPseudo = Animate(box, Fade(), pseudoOptions);
      CHECK(onPseudo->CommitStyles().GetException().Code() == dom::ExceptionCode::NoModificationAllowedError);
      onPseudo->Cancel(WebAnimation::Silently::Yes);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Animatable: a document timeline with an origin time",
                   "[Animation][Animatable]")
  {
    AnimationPageFixture fixture {u8"<!DOCTYPE html><div id=box></div>"};
    auto timeline = DocumentTimeline::Create(*fixture.Document, DocumentTimelineOptions {.OriginTime = 100});
    auto defaultTime = fixture.Document->Timeline().CurrentTime();
    auto time = timeline->CurrentTime();
    REQUIRE(defaultTime);
    REQUIRE(time);
    CHECK(time->Time()->count() == Catch::Approx(defaultTime->Time()->count() - 0.1));
    timeline->DetachFromDocument();
  }

  TEST_CASE("Animatable: DOMHighResTimeStamp is a double", "[Animation][Animatable]")
  {
    CHECK(std::is_floating_point_v<dom::DOMHighResTimeStamp>);
  }
}
