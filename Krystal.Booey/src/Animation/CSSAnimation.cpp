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

    switch (animation.GetFillMode())
    {
      case AnimationFillMode::None:      animationEffect->SetFill(FillMode::None); break;
      case AnimationFillMode::Backwards: animationEffect->SetFill(FillMode::Backwards); break;
      case AnimationFillMode::Forwards:  animationEffect->SetFill(FillMode::Forwards); break;
      case AnimationFillMode::Both:      animationEffect->SetFill(FillMode::Both); break;
    }

    switch (animation.GetDirection())
    {
      case AnimationDirection::Normal:    animationEffect->SetDirection(PlaybackDirection::Normal); break;
      case AnimationDirection::Alternate: animationEffect->SetDirection(PlaybackDirection::Alternate); break;
      case AnimationDirection::Reverse:   animationEffect->SetDirection(PlaybackDirection::Reverse); break;
      case AnimationDirection::AlternateReverse:
        animationEffect->SetDirection(PlaybackDirection::AlternateReverse);
        break;
    }

    if (auto iterations = animation.GetIterationCount().TryValue())
    {
      (void)animationEffect->SetIterations(iterations->Value);
    }
    else
    {
      (void)animationEffect->SetIterations(std::numeric_limits<double>::infinity());
    }

    animationEffect->SetDelay(Seconds {animation.GetDelay().Value});

    if (auto duration = animation.GetDuration().TryTime())
    {
      animationEffect->SetIterationDuration(Seconds {duration->Value});
    }
    else
    {
      animationEffect->SetIterationDuration(null);
    }

    if (auto *keyframeEffect = DynamicDowncast<KeyframeEffect>(animationEffect.get()))
    {
      keyframeEffect->SetComposite(animation.GetCompositeOperation());
    }

    // FIXME: The animation range (`animation-range-start`, `animation-range-end`), once there are scroll and
    // view timelines.

    EffectTimingDidChange();

    // Synchronize the play state
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
    if (!Effect())
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

  void CSSAnimation::KeyframesRuleDidChange() noexcept
  {
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
