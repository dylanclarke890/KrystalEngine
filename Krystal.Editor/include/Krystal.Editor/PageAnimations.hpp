#pragma once

#include "Krystal.Booey/Animation/KeyframeAnimationOptions.hpp"
#include "Krystal.Booey/Animation/KeyframeEffect.hpp"
#include "Krystal.Booey/Animation/WebAnimation.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/EventListenerHandle.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include <format>

namespace krys::editor
{
  /// @brief The editor page's animation, made from C++: it moves `#anim-ball` along its track, `#anim-play`,
  /// `#anim-pause` and `#anim-reverse` play, pause and reverse it, `#anim-seek` (a range from 0 to 100) seeks
  /// it, and `#anim-time` shows its time and play state on each animation frame. A page without the ball is
  /// left alone. (The page's scroll-driven effects are its own CSS.)
  class PageAnimations
  {
    KRYS_NON_COPY_MOVABLE(PageAnimations);

  public:
    /// @brief How long the ball takes to cross its track, in milliseconds.
    static constexpr double Duration = 2'000;

  private:
    WeakPtr<boo::dom::Document> _document;
    RefPtr<boo::WebAnimation> _animation;
    Array<boo::dom::EventListenerHandle, 4> _listeners;
    int32 _frameRequest {0};

  public:
    explicit PageAnimations(boo::dom::Document &document) noexcept
        : _document(WeakPtr<boo::dom::Document>::WithRef(document))
    {
      auto ball = document.GetElementById(u8"anim-ball");
      if (!ball)
      {
        return;
      }

      // A page restored from the back/forward cache keeps its animation.
      for (auto &existing : ball->GetAnimations())
      {
        if (existing->Id() == u8"ball")
        {
          _animation = existing;
        }
      }
      if (!_animation && !CreateAnimation(*ball))
      {
        return;
      }

      auto listen = [&](utf8_stringview id, boo::dom::DOMStringAtom type,
                        Func<void(boo::dom::Event &)> &&callback) -> boo::dom::EventListenerHandle
      {
        if (auto element = document.GetElementById(id))
        {
          return element->AddEventListener(type, krys::move(callback));
        }
        return {};
      };
      _listeners[0] =
        listen(u8"anim-play", u8"click", [this](boo::dom::Event &) { (void)_animation->Play(); });
      _listeners[1] =
        listen(u8"anim-pause", u8"click", [this](boo::dom::Event &) { (void)_animation->Pause(); });
      _listeners[2] =
        listen(u8"anim-reverse", u8"click", [this](boo::dom::Event &) { (void)_animation->Reverse(); });
      _listeners[3] = listen(u8"anim-seek", u8"input",
                             [this](boo::dom::Event &)
                             {
                               if (auto seek = _document ? _document->GetElementById(u8"anim-seek") : nullptr)
                               {
                                 if (auto *input = DynamicDowncast<boo::html::HTMLInputElement>(seek.get()))
                                 {
                                   Seek(input->ValueAsNumber() / 100);
                                 }
                               }
                             });

      ShowTime();
      RequestFrame();
    }

    ~PageAnimations() noexcept
    {
      if (_frameRequest && _document)
      {
        if (auto *window = _document->Window())
        {
          window->CancelAnimationFrame(_frameRequest);
        }
      }
    }

    /// @brief The ball's animation; null on a page without it.
    KRYS_NODISCARD boo::WebAnimation *Animation() const noexcept
    {
      return _animation.get();
    }

    /// @brief Seeks the animation to @p progress (0 to 1) of its duration.
    void Seek(double progress) noexcept
    {
      if (!_animation)
      {
        return;
      }
      (void)_animation->SetBindingsCurrentTime(boo::WebAnimationTime::FromMilliseconds(progress * Duration));
      ShowTime();
    }

  private:
    /// @brief Animates @p ball from its track's start to its end, paused at the start.
    KRYS_NODISCARD bool CreateAnimation(boo::dom::Element &ball) noexcept
    {
      // The ball's margin is a share of the track's width, less its own.
      boo::InputKeyframe start;
      start.Properties = {{u8"marginLeft", u8"0%"}};
      boo::InputKeyframe end;
      end.Properties = {{u8"marginLeft", u8"calc(100% - 40px)"}};
      boo::KeyframeAnimationOptions options;
      options.Id = u8"ball";
      options.Duration = Duration;
      options.Easing = u8"ease-in-out";
      options.Fill = boo::FillMode::Both;
      auto animation = ball.Animate(List<boo::InputKeyframe> {krys::move(start), krys::move(end)}, options);
      if (animation.HasException())
      {
        return false;
      }
      _animation = animation.ReleaseValue();
      // It waits at the start for the play button.
      (void)_animation->Pause();
      return true;
    }

    void RequestFrame() noexcept
    {
      auto *window = _document ? _document->Window() : nullptr;
      if (!window)
      {
        return;
      }
      _frameRequest = window->RequestAnimationFrame(
        [this](double)
        {
          _frameRequest = 0;
          ShowTime();
          RequestFrame();
        });
    }

    KRYS_NODISCARD static const char *PlayStateName(boo::WebAnimation::PlayState playState) noexcept
    {
      switch (playState)
      {
        case boo::WebAnimation::PlayState::Idle:     return "idle";
        case boo::WebAnimation::PlayState::Running:  return "running";
        case boo::WebAnimation::PlayState::Paused:   return "paused";
        case boo::WebAnimation::PlayState::Finished: return "finished";
      }
      return "";
    }

    /// @brief Shows the animation's time and play state, and moves the seek range with it.
    void ShowTime() noexcept
    {
      if (!_animation || !_document)
      {
        return;
      }
      auto currentTime = _animation->CurrentTime();
      auto seconds = currentTime && currentTime->Time() ? currentTime->Time()->count() : 0.0;
      auto text = std::format("{:.2f} s of {:.0f} s, {}", seconds, Duration / 1'000,
                              PlayStateName(_animation->GetPlayState()));
      if (auto label = _document->GetElementById(u8"anim-time"))
      {
        (void)label->TextContent(
          boo::dom::DOMString(reinterpret_cast<const char8_t *>(text.data()), text.size()));
      }
      if (auto seek = _document->GetElementById(u8"anim-seek"))
      {
        if (auto *input = DynamicDowncast<boo::html::HTMLInputElement>(seek.get()))
        {
          (void)input->SetValueAsNumber(seconds * 1'000 / Duration * 100);
        }
      }
    }
  };
}
