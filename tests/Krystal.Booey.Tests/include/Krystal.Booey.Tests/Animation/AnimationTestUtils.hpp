#pragma once

#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Animation/Events/AnimationEventBase.hpp"
#include "Krystal.Booey/Animation/Events/CSSAnimationEvent.hpp"
#include "Krystal.Booey/Animation/Events/CSSTransitionEvent.hpp"
#include "Krystal.Booey/Animation/KeyframeEffect.hpp"
#include "Krystal.Booey/Animation/WebAnimation.hpp"
#include "Krystal.Booey/Dom/Events/EventListener.hpp"
#include "Krystal.Booey/Dom/Nodes/Element.hpp"
#include "Krystal.Booey/Dom/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/Html/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/Html/Parser/DocumentParser.hpp"
#include "Krystal.Booey/Page/Frame.hpp"
#include "Krystal.Booey/Page/FrameView.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Style/Scope/Scope.hpp"
#include <string>

namespace krys::boo::tests
{
  /// @brief A page with a 200x100 view, and @p markup parsed into its main frame's document, to run
  /// animations on.
  struct AnimationPageFixture
  {
    Ref<boo::Frame> TestFrame {boo::Frame::Create()};
    RefPtr<HTMLDocument> Document;

    explicit AnimationPageFixture(utf8_string markup) noexcept
    {
      auto &frame = *TestFrame;
      frame.CreateView({200, 100});
      Document = CreateRef<HTMLDocument>();
      frame.SetDocument(ShareRefPtr<boo::Document>(Document.get()));
      auto parser = CreateRef<html::DocumentParser>(*Document);
      parser->InputStream().Append(krys::move(markup), IsEOF::Yes);
      parser->PumpTokenizer();
      Document->StyleScope().FlushPendingUpdate();
      TestFrame->UpdateRendering();
    }

    ~AnimationPageFixture() noexcept
    {
      TestFrame->SetDocument(nullptr);
    }

    KRYS_NODISCARD Element &ById(DOMStringView id) const noexcept
    {
      auto element = Document->GetElementById(id);
      REQUIRE(element);
      return *element;
    }

    void SetStyle(DOMStringView id, utf8_stringview style) noexcept
    {
      REQUIRE_FALSE(ById(id).SetAttribute(u8"style", DOMString {style}).HasException());
    }

    /// @brief Runs the event loop @p delta after its last run, then the rendering update if one is scheduled.
    void Frame(MonotonicTime::duration delta = {}) const noexcept
    {
      auto &loop = html::EventLoop::Current();
      loop.Run(loop.Now() + delta);
      if (TestFrame->IsRenderingUpdateScheduled())
      {
        TestFrame->UpdateRendering();
      }
    }

    /// @brief Runs frames until the animations created by a style change are ready: their start time is the
    /// time of the frame after the change.
    void StartAnimations() noexcept
    {
      Frame();
      Frame();
    }

    KRYS_NODISCARD float Opacity(DOMStringView id) noexcept
    {
      auto *renderer = ById(id).Renderer();
      REQUIRE(renderer);
      return renderer->Style().Opacity().Value.Value;
    }
  };

  /// @brief Records the animation events fired at an element: their type, then the transition's property or
  /// the animation's name.
  struct AnimationEventRecorder : EventListenerCallback
  {
    List<std::string> Records;

    void HandleEvent(Event &event) noexcept override
    {
      auto type = event.Type().View();
      std::string record(type.begin(), type.end());
      auto *animationEventBase = DynamicDowncast<AnimationEventBase>(event);
      if (auto *transitionEvent = DynamicDowncast<CSSTransitionEvent>(animationEventBase))
      {
        const auto &property = transitionEvent->PropertyName();
        record += ' ';
        record.append(property.begin(), property.end());
      }
      else if (auto *animationEvent = DynamicDowncast<CSSAnimationEvent>(animationEventBase))
      {
        const auto &name = animationEvent->AnimationName();
        record += ' ';
        record.append(name.begin(), name.end());
      }
      Records.push_back(krys::move(record));
    }

    void Listen(Element &element, std::initializer_list<const char8_t *> types) noexcept
    {
      for (auto type : types)
      {
        element.AddEventListener(DOMStringAtom(type), ShareRef<EventListenerCallback>(*this), false);
      }
    }
  };

  /// @brief Keyframes that fade opacity in from 0 to 1.
  KRYS_NODISCARD inline KeyframesInput FadeKeyframes() noexcept
  {
    InputKeyframe from;
    from.Properties = {{u8"opacity", u8"0"}};
    InputKeyframe to;
    to.Properties = {{u8"opacity", u8"1"}};
    return List<InputKeyframe> {krys::move(from), krys::move(to)};
  }

  /// @brief The percentage @p time holds, which it must.
  KRYS_NODISCARD inline double Percentage(const Maybe<WebAnimationTime> &time) noexcept
  {
    REQUIRE(time);
    REQUIRE(time->Percentage());
    return *time->Percentage();
  }

  /// @brief Scrolls `#scroller` to @p top and @p left, and runs a rendering update so its timelines catch up.
  inline void ScrollTo(AnimationPageFixture &fixture, int32 top, int32 left = 0) noexcept
  {
    auto &scroller = fixture.ById(u8"scroller");
    scroller.SetScrollTop(top);
    scroller.SetScrollLeft(left);
    fixture.TestFrame->UpdateRendering();
  }

  /// @brief The vertical scroll position of @p fixture's frame view.
  KRYS_NODISCARD inline int32 ViewY(AnimationPageFixture &fixture) noexcept
  {
    return fixture.TestFrame->View()->GetScrollPosition().Y();
  }

  /// @brief Runs two frames, 1.6 seconds apart, so that smooth scroll animations finish.
  inline void FinishScrollAnimations(AnimationPageFixture &fixture) noexcept
  {
    fixture.Frame();
    fixture.Frame(std::chrono::milliseconds(1'600));
  }
}
