#include "Krystal.Booey/Animation/CSSTransition.hpp"
#include "Krystal.Booey/Animation/AnimationEffect.hpp"
#include "Krystal.Booey/Animation/CSSTransitionEvent.hpp"
#include "Krystal.Booey/Animation/WebAnimationUtilities.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/Styleable.hpp"

namespace krys::boo
{
  Ref<CSSTransition> CSSTransition::Create(const style::Styleable &owningElement,
                                           const AnimatableCSSProperty &property, Seconds generationTime,
                                           const style::Transition &backingStyleTransition,
                                           const RenderStyle &oldStyle, const RenderStyle &newStyle,
                                           Seconds delay, Seconds duration,
                                           const RenderStyle &reversingAdjustedStartStyle,
                                           double reversingShorteningFactor) noexcept
  {
    Ref<CSSTransition> result =
      AdoptRef(*new CSSTransition(owningElement, property, generationTime, backingStyleTransition, newStyle,
                                  reversingAdjustedStartStyle, reversingShorteningFactor));
    result->Initialize(&oldStyle, newStyle, {nullptr});
    result->SetTimingProperties(delay, duration);
    return result;
  }

  CSSTransition::CSSTransition(const style::Styleable &styleable, const AnimatableCSSProperty &property,
                               Seconds generationTime, const style::Transition &backingStyleTransition,
                               const RenderStyle &targetStyle, const RenderStyle &reversingAdjustedStartStyle,
                               double reversingShorteningFactor) noexcept
      : StyleOriginatedAnimation(styleable), _property(property), _generationTime(generationTime),
        _targetStyle(RenderStyle::ClonePtr(targetStyle)),
        _reversingAdjustedStartStyle(RenderStyle::ClonePtr(reversingAdjustedStartStyle)),
        _reversingShorteningFactor(reversingShorteningFactor), _backingStyleTransition(backingStyleTransition)
  {
  }

  CSSTransition::~CSSTransition() noexcept = default;

  void CSSTransition::AnimationDidFinish() noexcept
  {
    StyleOriginatedAnimation::AnimationDidFinish();

    if (auto owningElement = OwningElement())
    {
      owningElement->RemoveStyleOriginatedAnimationFromListsForOwningElement(*this);
    }
  }

  void CSSTransition::SetTimingProperties(Seconds delay, Seconds duration) noexcept
  {
    SuspendEffectInvalidation();

    // This method is only called from CSSTransition::Create() where we're guaranteed to have an effect.
    Ref animationEffect = ShareRef(*Effect());

    // In order for CSS Transitions to be seeked backwards, they need to have their fill mode set to backwards
    // such that the original CSS value applied prior to the transition is used for a negative current time.
    animationEffect->SetFill(FillMode::Backwards);
    animationEffect->SetDelay(delay);
    animationEffect->SetIterationDuration(duration);
    animationEffect->SetTimingFunction(ShareRefPtr(_backingStyleTransition.GetTimingFunction().Value.get()));
    EffectTimingDidChange();

    UnsuspendEffectInvalidation();
  }

  Ref<StyleOriginatedAnimationEvent>
    CSSTransition::CreateEvent(dom::DOMStringAtom eventType, Maybe<Seconds> scheduledTime, double elapsedTime,
                               const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept
  {
    return CreateRef<CSSTransitionEvent>(eventType, this, scheduledTime, elapsedTime, pseudoElementIdentifier,
                                         css::CSSOMString {TransitionProperty().View()});
  }

  css::CSSOMStringAtom CSSTransition::TransitionProperty() const noexcept
  {
    return AnimatablePropertyAsString(_property);
  }

  AnimationPlayState CSSTransition::BackingAnimationPlayState() const noexcept
  {
    return AnimationPlayState::Running;
  }

  TimingFunction *CSSTransition::BackingAnimationTimingFunction() const noexcept
  {
    return _backingStyleTransition.GetTimingFunction().Value.get();
  }
}
