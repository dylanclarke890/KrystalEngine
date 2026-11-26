#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Web.Tests/Animation/AnimationTestUtils.hpp"
#include "Krystal.Web.Tests/Css/ValueTestUtils.hpp"
#include "Krystal.Web/Animation/CSSAnimation.hpp"
#include "Krystal.Web/Animation/KeyframeAnimationOptions.hpp"
#include "Krystal.Web/Animation/KeyframeEffect.hpp"
#include "Krystal.Web/Animation/Timelines/DocumentTimeline.hpp"
#include "Krystal.Web/Animation/WebAnimation.hpp"
#include "Krystal.Web/Css/TypedOM/CSSNumericFactory.hpp"
#include "Krystal.Web/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Web/Dom/Types/DOMHighResTimeStamp.hpp"

namespace krys::web::tests
{
  namespace
  {
    using namespace std::chrono_literals;

    KRYS_NODISCARD InputKeyframe Opacity(KeyframeOffset offset, DOMString value) noexcept
    {
      InputKeyframe keyframe;
      keyframe.Offset = krys::move(offset);
      keyframe.Properties = {{u8"opacity", krys::move(value)}};
      return keyframe;
    }

    KRYS_NODISCARD Ref<WebAnimation>
      Animate(Element &element, KeyframesInput &&keyframes,
              Maybe<Variant<double, KeyframeAnimationOptions>> &&options = 1'000.0)
    {
      auto result = element.Animate(krys::move(keyframes), krys::move(options));
      REQUIRE(result.HasValue());
      return result.ReleaseValue();
    }

    /// @brief Records the outcomes of `WhenReady` and `WhenFinished`: what was waited on and the animation's
    /// ID, or the exception's name.
    struct AnimationWaitRecorder
    {
      List<std::string> Records;

      KRYS_NODISCARD WebAnimation::ReadyCallback Settled(const std::string &what) noexcept
      {
        return [this, what](WebAnimation &animation)
        {
          Records.push_back(what + " " + std::string(animation.Id().begin(), animation.Id().end()));
        };
      }

      KRYS_NODISCARD WebAnimation::ErrorCallback Failed(const std::string &what) noexcept
      {
        return [this, what](const Exception &exception)
        {
          Records.push_back(what + " "
                            + (exception.Code() == ExceptionCode::AbortError ? "AbortError" : "rejected"));
        };
      }

      void WhenReady(WebAnimation &animation) noexcept
      {
        animation.WhenReady(Settled("ready"), Failed("ready"));
      }

      void WhenFinished(WebAnimation &animation) noexcept
      {
        animation.WhenFinished(Settled("finished"), Failed("finished"));
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
    auto animation = Animate(box, FadeKeyframes(), options);
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
      auto unattached = Animate(box, FadeKeyframes(), noTimeline);
      CHECK_FALSE(unattached->Timeline());
      unattached->Cancel(WebAnimation::Silently::Yes);
    }

    SECTION("invalid keyframes")
    {
      auto result = box.Animate(List<InputKeyframe> {Opacity(2.0, u8"0")});
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == ExceptionCode::TypeError);
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
    auto first = Animate(outer, FadeKeyframes());
    auto second = Animate(inner, FadeKeyframes());

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
      auto inShadow = Animate(*shadowChild, FadeKeyframes());
      auto shadowAnimations = shadowRoot.Value()->GetAnimations();
      REQUIRE(shadowAnimations.size() == 1);
      CHECK(shadowAnimations[0] == inShadow.get());
      // The document's animations leave out those in shadow trees.
      CHECK(fixture.Document->GetAnimations().size() == 2);
      inShadow->Cancel(WebAnimation::Silently::Yes);
    }

    second->Cancel(WebAnimation::Silently::Yes);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Animatable: waiting for the animation to be ready and finished",
                   "[Animation][Animatable]")
  {
    AnimationPageFixture fixture {u8"<!DOCTYPE html><div id=box></div>"};
    KeyframeAnimationOptions options;
    options.Id = u8"a";
    options.Duration = 500.0;
    auto animation = Animate(fixture.ById(u8"box"), FadeKeyframes(), options);

    AnimationWaitRecorder recorder;
    recorder.WhenReady(*animation);
    recorder.WhenFinished(*animation);
    CHECK(animation->Pending());
    CHECK(recorder.Records.empty());

    // The ready callback runs once the animation is ready, the finished one once it has finished.
    fixture.StartAnimations();
    CHECK(recorder.Records == List<std::string> {"ready a"});
    fixture.Frame(600ms);
    fixture.Frame();
    CHECK(recorder.Records == List<std::string> {"ready a", "finished a"});

    // A finished animation calls back straight away.
    recorder.WhenFinished(*animation);
    fixture.Frame();
    CHECK(recorder.Records == List<std::string> {"ready a", "finished a", "finished a"});

    // Playing again waits on a new finished promise; canceling rejects the pending ready promise, then it.
    REQUIRE_FALSE(animation->Play().HasException());
    recorder.Records.clear();
    recorder.WhenReady(*animation);
    recorder.WhenFinished(*animation);
    animation->Cancel();
    fixture.Frame();
    CHECK(recorder.Records == List<std::string> {"ready AbortError", "finished AbortError"});
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Animatable: times from C++ are validated",
                   "[Animation][Animatable]")
  {
    AnimationPageFixture fixture {u8"<!DOCTYPE html><div id=box></div>"};
    auto animation = Animate(fixture.ById(u8"box"), FadeKeyframes());
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
    auto animation = Animate(box, FadeKeyframes(), options);
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
      auto onPseudo = Animate(box, FadeKeyframes(), pseudoOptions);
      CHECK(onPseudo->CommitStyles().GetException().Code() == ExceptionCode::NoModificationAllowedError);
      onPseudo->Cancel(WebAnimation::Silently::Yes);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Animatable: a document timeline with an origin time",
                   "[Animation][Animatable]")
  {
    AnimationPageFixture fixture {u8"<!DOCTYPE html><div id=box></div>"};
    auto timeline =
      DocumentTimeline::Create(*fixture.Document, DocumentTimelineOptions {.OriginTime = 100ms});
    auto defaultTime = fixture.Document->Timeline().CurrentTime();
    auto time = timeline->CurrentTime();
    REQUIRE(defaultTime);
    REQUIRE(time);
    CHECK(time->Time()->count() == Catch::Approx(defaultTime->Time()->count() - 0.1));
    timeline->DetachFromDocument();
  }

  TEST_CASE("Animatable: DOMHighResTimeStamp is a duration of fractional milliseconds",
            "[Animation][Animatable]")
  {
    CHECK(SameType<DOMHighResTimeStamp, std::chrono::duration<double, std::milli>>);
  }
}
