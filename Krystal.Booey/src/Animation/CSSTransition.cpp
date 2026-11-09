/*
 * Copyright (C) 2018-2025 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Animation/CSSTransition.hpp"
#include "Krystal.Booey/Animation/AnimationEffect.hpp"
#include "Krystal.Booey/Animation/Events/CSSTransitionEvent.hpp"
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
