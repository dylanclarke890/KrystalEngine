#include "Krystal.Booey/Animation/WebAnimation.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/Animation/AnimationPlaybackEvent.hpp"
#include "Krystal.Booey/Animation/DocumentTimeline.hpp"
#include "Krystal.Booey/Animation/KeyframeEffect.hpp"
#include "Krystal.Booey/Animation/KeyframeEffectStack.hpp"
#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Style/Scope.hpp"

namespace krys::boo::tests
{
  namespace
  {
    using namespace std::chrono_literals;

    /// @brief A page showing a document with a `#target` element, and an animation of an effect on it.
    struct AnimationFixture
    {
      Ref<Page> TestPage {Page::Create()};
      RefPtr<dom::HTMLDocument> Document;
      RefPtr<KeyframeEffect> Effect;
      RefPtr<WebAnimation> Animation;

      AnimationFixture() noexcept
      {
        auto &frame = TestPage->MainFrame();
        frame.CreateView({200, 100});
        Document = CreateRef<dom::HTMLDocument>();
        frame.SetDocument(ShareRefPtr<dom::Document>(Document.get()));
        auto parser = CreateRef<html::HTMLDocumentParser>(*Document);
        parser->InputStream().Append(u8"<!DOCTYPE html><div id=target></div>", IsEOF(true));
        parser->PumpTokenizer();
        Document->StyleScope().FlushPendingUpdate();
        TestPage->UpdateRendering();

        auto target = Document->GetElementById(u8"target");
        REQUIRE(target);
        Effect = KeyframeEffect::Create(*target, null);
        Effect->SetIterationDuration(Seconds {1});
        Animation = WebAnimation::Create(*Document, Effect.get());
      }

      ~AnimationFixture() noexcept
      {
        Animation->Cancel(WebAnimation::Silently::Yes);
        TestPage->MainFrame().SetDocument(nullptr);
      }

      /// @brief Runs the event loop @p delta after its last run, then the rendering update if one is
      /// scheduled.
      void Frame(MonotonicTime::duration delta = {}) noexcept
      {
        auto &loop = html::EventLoop::Current();
        loop.Run(loop.Now() + delta);
        if (TestPage->IsRenderingUpdateScheduled())
        {
          TestPage->UpdateRendering();
        }
      }

      /// @brief The animation's current time, in seconds.
      KRYS_NODISCARD double CurrentTime() const noexcept
      {
        auto currentTime = Animation->CurrentTime();
        REQUIRE(currentTime);
        REQUIRE(currentTime->Time());
        return currentTime->Time()->count();
      }
    };

    /// @brief Records the types of the events fired at a target.
    struct EventRecorder : dom::EventListenerCallback
    {
      List<std::string> Records;

      void HandleEvent(dom::Event &event) noexcept override
      {
        auto type = event.Type().View();
        Records.emplace_back(type.begin(), type.end());
      }
    };
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "WebAnimation: an animation on the document timeline",
                   "[Animation][WebAnimation]")
  {
    AnimationFixture fixture;
    auto &animation = *fixture.Animation;

    CHECK(animation.Timeline() == &fixture.Document->Timeline());
    CHECK(animation.Effect() == fixture.Effect.get());
    CHECK(fixture.Effect->Animation() == &animation);
    CHECK(animation.GetPlayState() == WebAnimation::PlayState::Idle);
    CHECK_FALSE(animation.CurrentTime());
    CHECK_FALSE(animation.IsRelevant());
    CHECK(animation.GlobalPosition() > 0);

    // The document timeline's time is the host's time.
    auto timelineTime = fixture.Document->Timeline().CurrentTime();
    REQUIRE(timelineTime);
    CHECK(timelineTime->Time()->count() == Catch::Approx(Seconds {html::EventLoop::Current().Now()}.count()));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "WebAnimation: playing", "[Animation][WebAnimation]")
  {
    AnimationFixture fixture;
    auto &animation = *fixture.Animation;

    REQUIRE_FALSE(animation.Play().HasException());
    CHECK(animation.Pending());
    CHECK(animation.GetPlayState() == WebAnimation::PlayState::Running);
    CHECK(animation.CurrentTime() == WebAnimationTime {Seconds {0}});
    CHECK(animation.IsRelevant());
    CHECK(fixture.TestPage->IsRenderingUpdateScheduled());

    // It becomes ready, with the time of the frame after it was played as its start time.
    fixture.Frame();
    fixture.Frame();
    CHECK_FALSE(animation.Pending());
    CHECK(animation.StartTime());
    CHECK(fixture.CurrentTime() == Catch::Approx(0));

    fixture.Frame(250ms);
    CHECK(fixture.CurrentTime() == Catch::Approx(0.25));
    CHECK(fixture.Effect->GetComputedTiming().Progress == Catch::Approx(0.25));
    CHECK(fixture.Effect->GetComputedTiming().Phase == AnimationEffectPhase::Active);

    SECTION("pausing holds its current time")
    {
      // The pause takes effect once the animation is ready, with its current time then.
      REQUIRE_FALSE(animation.Pause().HasException());
      CHECK(animation.GetPlayState() == WebAnimation::PlayState::Paused);
      fixture.Frame();
      fixture.Frame();
      CHECK_FALSE(animation.Pending());
      CHECK_FALSE(animation.StartTime());
      fixture.Frame(250ms);
      CHECK(fixture.CurrentTime() == Catch::Approx(0.25));

      REQUIRE_FALSE(animation.Play().HasException());
      fixture.Frame();
      fixture.Frame();
      fixture.Frame(250ms);
      CHECK(fixture.CurrentTime() == Catch::Approx(0.5));
    }

    SECTION("the playback rate scales its current time")
    {
      animation.SetPlaybackRate(2);
      fixture.Frame(125ms);
      CHECK(fixture.CurrentTime() == Catch::Approx(0.5));
    }

    SECTION("seeking")
    {
      REQUIRE_FALSE(animation.SetCurrentTime(WebAnimationTime {Seconds {0.75}}).HasException());
      CHECK(fixture.CurrentTime() == Catch::Approx(0.75));
      fixture.Frame(125ms);
      CHECK(fixture.CurrentTime() == Catch::Approx(0.875));
    }

    SECTION("reversing")
    {
      REQUIRE_FALSE(animation.Reverse().HasException());
      fixture.Frame();
      fixture.Frame();
      CHECK(animation.PlaybackRate() == -1);
      fixture.Frame(125ms);
      CHECK(fixture.CurrentTime() == Catch::Approx(0.125));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "WebAnimation: finishing and canceling",
                   "[Animation][WebAnimation]")
  {
    AnimationFixture fixture;
    auto &animation = *fixture.Animation;
    auto recorder = CreateRef<EventRecorder>();
    for (auto type : {u8"finish", u8"cancel"})
    {
      animation.AddEventListener(dom::DOMStringAtom(type), ShareRef<dom::EventListenerCallback>(*recorder),
                                 false);
    }

    REQUIRE_FALSE(animation.Play().HasException());
    fixture.Frame();
    fixture.Frame();

    SECTION("it finishes at its end time")
    {
      fixture.Frame(1s);
      CHECK(animation.GetPlayState() == WebAnimation::PlayState::Finished);
      CHECK(fixture.CurrentTime() == Catch::Approx(1));
      CHECK_FALSE(animation.IsRelevant());
      fixture.Frame();
      CHECK(recorder->Records == List<std::string> {"finish"});
    }

    SECTION("finish() jumps to its end")
    {
      REQUIRE_FALSE(animation.Finish().HasException());
      CHECK(animation.GetPlayState() == WebAnimation::PlayState::Finished);
      CHECK(fixture.CurrentTime() == Catch::Approx(1));
      fixture.Frame();
      CHECK(recorder->Records == List<std::string> {"finish"});
    }

    SECTION("cancel() makes it idle")
    {
      animation.Cancel();
      CHECK(animation.GetPlayState() == WebAnimation::PlayState::Idle);
      CHECK_FALSE(animation.CurrentTime());
      CHECK_FALSE(animation.StartTime());
      fixture.Frame();
      CHECK(recorder->Records == List<std::string> {"cancel"});
    }

    SECTION("an infinite animation can't be finished")
    {
      REQUIRE_FALSE(fixture.Effect->SetIterations(std::numeric_limits<double>::infinity()).HasException());
      CHECK(animation.Finish().HasException());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "WebAnimation: effect stacks", "[Animation][WebAnimation]")
  {
    AnimationFixture fixture;
    auto &animation = *fixture.Animation;
    auto &target = *fixture.Document->GetElementById(u8"target");

    // Only relevant animations' effects are in their target's effect stack.
    CHECK_FALSE(target.HasKeyframeEffects(null));
    REQUIRE_FALSE(animation.Play().HasException());
    CHECK(target.HasKeyframeEffects(null));
    auto *stack = target.GetKeyframeEffectStack(null);
    REQUIRE(stack);
    REQUIRE(stack->SortedEffects().size() == 1);
    CHECK(stack->SortedEffects()[0].get() == fixture.Effect.get());

    // A second animation sorts after the first, by its position in the global animation list.
    auto secondEffect = KeyframeEffect::Create(target, null);
    secondEffect->SetIterationDuration(Seconds {1});
    auto second = WebAnimation::Create(*fixture.Document, secondEffect.get());
    REQUIRE_FALSE(second->Play().HasException());
    REQUIRE(stack->SortedEffects().size() == 2);
    CHECK(stack->SortedEffects()[1].get() == secondEffect.get());

    animation.Cancel(WebAnimation::Silently::Yes);
    CHECK(stack->SortedEffects().size() == 1);
    second->Cancel(WebAnimation::Silently::Yes);
    CHECK_FALSE(target.HasKeyframeEffects(null));
  }
}
