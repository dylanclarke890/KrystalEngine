#include "Krystal.Booey/Animation/CSSAnimation.hpp"
#include "Krystal.Booey/Animation/CSSAnimationEvent.hpp"
#include "Krystal.Booey/Animation/DocumentTimeline.hpp"
#include "Krystal.Booey/Animation/KeyframeEffect.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/Styleable.hpp"
#include <limits>

namespace krys::boo
{
  Ref<CSSAnimation> CSSAnimation::Create(const style::Styleable &owningElement,
                                         style::Animation &&backingStyleAnimation,
                                         const RenderStyle *oldStyle, const RenderStyle &newStyle,
                                         const style::ResolutionContext &resolutionContext) noexcept
  {
    // CSSAnimation should only ever be created with non-"none" animation names.
    auto name = backingStyleAnimation.GetName().TryKeyframesName();
    krys_assert(name);
    Ref<CSSAnimation> result =
      AdoptRef(*new CSSAnimation(owningElement, krys::move(*name), krys::move(backingStyleAnimation)));
    result->Initialize(oldStyle, newStyle, resolutionContext);
    return result;
  }

  CSSAnimation::CSSAnimation(const style::Styleable &styleable, style::ScopedName &&animationName,
                             style::Animation &&backingStyleAnimation) noexcept
      : StyleOriginatedAnimation(styleable), _animationName(krys::move(animationName)),
        _backingStyleAnimation(krys::move(backingStyleAnimation))
  {
  }

  CSSAnimation::~CSSAnimation() noexcept = default;

  void CSSAnimation::SetBackingStyleAnimation(const style::Animation &backingStyleAnimation) noexcept
  {
    _backingStyleAnimation = backingStyleAnimation;
    SyncPropertiesWithBackingAnimation();
  }

  void CSSAnimation::SyncPropertiesWithBackingAnimation() noexcept
  {
    StyleOriginatedAnimation::SyncPropertiesWithBackingAnimation();

    // If we have been disassociated from our original owning element, we should no longer sync any of the
    // `animation-*` CSS properties.
    if (!OwningElement())
    {
      return;
    }

    if (!Effect())
    {
      return;
    }

    SuspendEffectInvalidation();

    // https://drafts.csswg.org/css-animations-2/#animation-timeline
    // When multiple animation-* properties are set simultaneously, animation-timeline is updated first, so
    // e.g. a change to animation-play-state applies to the simultaneously-applied timeline specified in
    // animation-timeline.
    SyncStyleOriginatedTimeline();

    const auto &animation = _backingStyleAnimation;
    Ref animationEffect = ShareRef(*Effect());

    if (!_overriddenProperties.Contains(Property::FillMode))
    {
      switch (animation.GetFillMode())
      {
        case AnimationFillMode::None:      animationEffect->SetFill(FillMode::None); break;
        case AnimationFillMode::Backwards: animationEffect->SetFill(FillMode::Backwards); break;
        case AnimationFillMode::Forwards:  animationEffect->SetFill(FillMode::Forwards); break;
        case AnimationFillMode::Both:      animationEffect->SetFill(FillMode::Both); break;
      }
    }

    if (!_overriddenProperties.Contains(Property::Direction))
    {
      switch (animation.GetDirection())
      {
        case AnimationDirection::Normal: animationEffect->SetDirection(PlaybackDirection::Normal); break;
        case AnimationDirection::Alternate:
          animationEffect->SetDirection(PlaybackDirection::Alternate);
          break;
        case AnimationDirection::Reverse: animationEffect->SetDirection(PlaybackDirection::Reverse); break;
        case AnimationDirection::AlternateReverse:
          animationEffect->SetDirection(PlaybackDirection::AlternateReverse);
          break;
      }
    }

    if (!_overriddenProperties.Contains(Property::IterationCount))
    {
      if (auto iterations = animation.GetIterationCount().TryValue())
      {
        (void)animationEffect->SetIterations(iterations->Value);
      }
      else
      {
        (void)animationEffect->SetIterations(std::numeric_limits<double>::infinity());
      }
    }

    if (!_overriddenProperties.Contains(Property::Delay))
    {
      animationEffect->SetDelay(Seconds {animation.GetDelay().Value});
    }

    if (!_overriddenProperties.Contains(Property::Duration))
    {
      if (auto duration = animation.GetDuration().TryTime())
      {
        animationEffect->SetIterationDuration(Seconds {duration->Value});
      }
      else
      {
        animationEffect->SetIterationDuration(null);
      }
    }

    if (!_overriddenProperties.Contains(Property::CompositeOperation))
    {
      if (auto *keyframeEffect = DynamicDowncast<KeyframeEffect>(animationEffect.get()))
      {
        keyframeEffect->SetComposite(animation.GetCompositeOperation());
      }
    }

    // FIXME: The animation range (`animation-range-start`, `animation-range-end`), once there are scroll and
    // view timelines.

    EffectTimingDidChange();

    // Synchronize the play state
    if (!_overriddenProperties.Contains(Property::PlayState))
    {
      auto styleOriginatedPlayState = animation.GetPlayState();
      if (_lastStyleOriginatedPlayState != styleOriginatedPlayState)
      {
        if (styleOriginatedPlayState == AnimationPlayState::Running
            && GetPlayState() == WebAnimation::PlayState::Paused)
        {
          (void)Play();
        }
        else if (styleOriginatedPlayState == AnimationPlayState::Paused
                 && GetPlayState() == WebAnimation::PlayState::Running)
        {
          (void)Pause();
        }
      }
      _lastStyleOriginatedPlayState = styleOriginatedPlayState;
    }

    UnsuspendEffectInvalidation();
  }

  AnimationPlayState CSSAnimation::BackingAnimationPlayState() const noexcept
  {
    return _backingStyleAnimation.GetPlayState();
  }

  TimingFunction *CSSAnimation::BackingAnimationTimingFunction() const noexcept
  {
    return _backingStyleAnimation.GetTimingFunction().Value.get();
  }

  void CSSAnimation::SyncStyleOriginatedTimeline() noexcept
  {
    if (_overriddenProperties.Contains(Property::Timeline) || !Effect())
    {
      return;
    }

    SuspendEffectInvalidation();

    krys_debug_assert(OwningElement());
    Ref document = ShareRef(OwningElement()->Element.NodeDocument());

    const auto &timeline = _backingStyleAnimation.GetTimeline();
    if (timeline.IsAuto())
    {
      SetTimeline(ShareRefPtr<AnimationTimeline>(document->ExistingTimeline()));
    }
    else
    {
      // FIXME: Named timelines and the scroll() and view() functions, once there are scroll and view
      // timelines; until then, such an animation has no timeline, as one naming a timeline that doesn't
      // exist.
      SetTimeline(nullptr);
    }

    UnsuspendEffectInvalidation();
  }

  AnimationTimeline *CSSAnimation::BindingsTimeline() const noexcept
  {
    FlushPendingStyleChanges();
    return StyleOriginatedAnimation::BindingsTimeline();
  }

  void CSSAnimation::SetBindingsTimeline(RefPtr<AnimationTimeline> &&timeline) noexcept
  {
    _overriddenProperties.Add(Property::Timeline);
    StyleOriginatedAnimation::SetBindingsTimeline(krys::move(timeline));
  }

  dom::ExceptionOr<void> CSSAnimation::BindingsPlay() noexcept
  {
    // https://drafts.csswg.org/css-animations-2/#animations

    // After a successful call to play() or pause() on a CSSAnimation, any subsequent change to the
    // animation-play-state will no longer cause the CSSAnimation to be played or paused.

    auto retVal = StyleOriginatedAnimation::BindingsPlay();
    if (!retVal.HasException())
    {
      _overriddenProperties.Add(Property::PlayState);
    }
    return retVal;
  }

  dom::ExceptionOr<void> CSSAnimation::BindingsPause() noexcept
  {
    // https://drafts.csswg.org/css-animations-2/#animations

    // After a successful call to play() or pause() on a CSSAnimation, any subsequent change to the
    // animation-play-state will no longer cause the CSSAnimation to be played or paused.

    auto retVal = StyleOriginatedAnimation::BindingsPause();
    if (!retVal.HasException())
    {
      _overriddenProperties.Add(Property::PlayState);
    }
    return retVal;
  }

  void CSSAnimation::SetBindingsEffect(RefPtr<AnimationEffect> &&newEffect) noexcept
  {
    // https://drafts.csswg.org/css-animations-2/#animations

    // After successfully setting the effect of a CSSAnimation to null or some AnimationEffect other than the
    // original KeyframeEffect, all subsequent changes to animation properties other than animation-name or
    // animation-play-state will not be reflected in that animation. Similarly, any change to matching
    // @keyframes rules will not be reflected in that animation. However, if the last matching @keyframes rule
    // is removed the animation must still be canceled.

    RefPtr previousEffect = ShareRefPtr(Effect());
    StyleOriginatedAnimation::SetBindingsEffect(krys::move(newEffect));
    if (Effect() != previousEffect.get())
    {
      _overriddenProperties.Add(Property::Duration);
      _overriddenProperties.Add(Property::TimingFunction);
      _overriddenProperties.Add(Property::IterationCount);
      _overriddenProperties.Add(Property::Direction);
      _overriddenProperties.Add(Property::Delay);
      _overriddenProperties.Add(Property::FillMode);
      _overriddenProperties.Add(Property::Keyframes);
      _overriddenProperties.Add(Property::CompositeOperation);
    }
  }

  dom::ExceptionOr<void> CSSAnimation::SetBindingsStartTime(const Maybe<WebAnimationTime> &startTime) noexcept
  {
    // https://drafts.csswg.org/css-animations-2/#animations

    // After a successful call to reverse() on a CSSAnimation or after successfully setting the startTime on a
    // CSSAnimation, if, as a result of that call the play state of the CSSAnimation changes to or from the
    // paused play state, any subsequent change to the animation-play-state will no longer cause the
    // CSSAnimation to be played or paused.

    auto previousPlayState = GetPlayState();
    auto result = StyleOriginatedAnimation::SetBindingsStartTime(startTime);
    if (result.HasException())
    {
      return result.ReleaseException();
    }
    auto currentPlayState = GetPlayState();
    if (currentPlayState != previousPlayState
        && (currentPlayState == PlayState::Paused || previousPlayState == PlayState::Paused))
    {
      _overriddenProperties.Add(Property::PlayState);
    }

    return {};
  }

  dom::ExceptionOr<void> CSSAnimation::BindingsReverse() noexcept
  {
    // https://drafts.csswg.org/css-animations-2/#animations

    // After a successful call to reverse() on a CSSAnimation or after successfully setting the startTime on a
    // CSSAnimation, if, as a result of that call the play state of the CSSAnimation changes to or from the
    // paused play state, any subsequent change to the animation-play-state will no longer cause the
    // CSSAnimation to be played or paused.

    auto previousPlayState = GetPlayState();
    auto retVal = StyleOriginatedAnimation::BindingsReverse();
    if (!retVal.HasException())
    {
      auto currentPlayState = GetPlayState();
      if (currentPlayState != previousPlayState
          && (currentPlayState == PlayState::Paused || previousPlayState == PlayState::Paused))
      {
        _overriddenProperties.Add(Property::PlayState);
      }
    }
    return retVal;
  }

  void CSSAnimation::EffectTimingWasUpdatedUsingBindings(const OptionalEffectTiming &timing) noexcept
  {
    // https://drafts.csswg.org/css-animations-2/#animations

    // After a successful call to updateTiming() on the KeyframeEffect associated with a CSSAnimation, for
    // each property included in the timing parameter, any subsequent change to a corresponding animation
    // property will not be reflected in that animation.

    if (timing.Duration)
    {
      _overriddenProperties.Add(Property::Duration);
    }

    if (timing.Iterations)
    {
      _overriddenProperties.Add(Property::IterationCount);
    }

    if (timing.Delay)
    {
      _overriddenProperties.Add(Property::Delay);
    }

    if (timing.Easing)
    {
      _overriddenProperties.Add(Property::TimingFunction);
    }

    if (timing.Fill)
    {
      _overriddenProperties.Add(Property::FillMode);
    }

    if (timing.Direction)
    {
      _overriddenProperties.Add(Property::Direction);
    }
  }

  void CSSAnimation::EffectKeyframesWereSetUsingBindings() noexcept
  {
    // https://drafts.csswg.org/css-animations-2/#animations

    // After a successful call to setKeyframes() on the KeyframeEffect associated with a CSSAnimation, any
    // subsequent change to matching @keyframes rules or the resolved value of the animation-timing-function
    // property for the target element will not be reflected in that animation.
    _overriddenProperties.Add(Property::Keyframes);
    _overriddenProperties.Add(Property::TimingFunction);
  }

  void CSSAnimation::EffectCompositeOperationWasSetUsingBindings() noexcept
  {
    _overriddenProperties.Add(Property::CompositeOperation);
  }

  void CSSAnimation::KeyframesRuleDidChange() noexcept
  {
    if (_overriddenProperties.Contains(Property::Keyframes))
    {
      return;
    }

    RefPtr keyframeEffect = ShareRefPtr(GetKeyframeEffect());
    if (!keyframeEffect)
    {
      return;
    }

    auto owningElement = OwningElement();
    if (!owningElement)
    {
      return;
    }

    keyframeEffect->RecomputeKeyframesAtNextOpportunity();
    owningElement->KeyframesRuleDidChange();
  }

  void CSSAnimation::UpdateKeyframesIfNeeded(const RenderStyle *oldStyle, const RenderStyle &newStyle,
                                             const style::ResolutionContext &resolutionContext) noexcept
  {
    RefPtr keyframeEffect = ShareRefPtr(GetKeyframeEffect());
    if (!keyframeEffect)
    {
      return;
    }

    if (keyframeEffect->GetBlendingKeyframes().IsEmpty())
    {
      keyframeEffect->ComputeStyleOriginatedAnimationBlendingKeyframes(oldStyle, newStyle, resolutionContext);
    }
  }

  Ref<StyleOriginatedAnimationEvent>
    CSSAnimation::CreateEvent(dom::DOMStringAtom eventType, Maybe<Seconds> scheduledTime, double elapsedTime,
                              const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept
  {
    return CreateRef<CSSAnimationEvent>(eventType, this, scheduledTime, elapsedTime, pseudoElementIdentifier,
                                        css::CSSOMString {_animationName.Name.View()});
  }
}
