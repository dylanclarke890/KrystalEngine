#pragma once

#include "Krystal.Booey/Animation/StyleOriginatedAnimation.hpp"
#include "Krystal.Booey/Style/Values/Transitions/Transition.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  /// @brief A transition created by an element's `transition` properties when a property's value changes.
  /// @see https://drafts.csswg.org/css-transitions-2/#csstransition
  class CSSTransition final : public StyleOriginatedAnimation
  {
  private:
    AnimatableCSSProperty _property;
    Seconds _generationTime;
    UniquePtr<RenderStyle> _targetStyle;
    UniquePtr<RenderStyle> _reversingAdjustedStartStyle;
    double _reversingShorteningFactor;
    style::Transition _backingStyleTransition;

    CSSTransition(const style::Styleable &styleable, const AnimatableCSSProperty &property,
                  Seconds generationTime, const style::Transition &backingStyleTransition,
                  const RenderStyle &targetStyle, const RenderStyle &reversingAdjustedStartStyle,
                  double reversingShorteningFactor) noexcept;

  public:
    ~CSSTransition() noexcept override;

    KRYS_NODISCARD static Ref<CSSTransition>
      Create(const style::Styleable &owningElement, const AnimatableCSSProperty &property,
             Seconds generationTime, const style::Transition &backingStyleTransition,
             const RenderStyle &oldStyle, const RenderStyle &newStyle, Seconds delay, Seconds duration,
             const RenderStyle &reversingAdjustedStartStyle, double reversingShorteningFactor) noexcept;

    /// @see https://drafts.csswg.org/css-transitions-2/#dom-csstransition-transitionproperty
    KRYS_NODISCARD css::CSSOMStringAtom TransitionProperty() const noexcept;
    KRYS_NODISCARD AnimatableCSSProperty Property() const noexcept
    {
      return _property;
    }
    /// @brief When the transition was generated, in the host's time.
    /// @see https://drafts.csswg.org/css-transitions-2/#transition-generation
    KRYS_NODISCARD Seconds GenerationTime() const noexcept
    {
      return _generationTime;
    }
    KRYS_NODISCARD const RenderStyle &TargetStyle() const noexcept
    {
      return *_targetStyle;
    }
    /// @see https://drafts.csswg.org/css-transitions-1/#reversing-adjusted-start-value
    KRYS_NODISCARD const RenderStyle &ReversingAdjustedStartStyle() const noexcept
    {
      return *_reversingAdjustedStartStyle;
    }
    /// @see https://drafts.csswg.org/css-transitions-1/#reversing-shortening-factor
    KRYS_NODISCARD double ReversingShorteningFactor() const noexcept
    {
      return _reversingShorteningFactor;
    }
    KRYS_NODISCARD const style::Transition &BackingStyleTransition() const noexcept
    {
      return _backingStyleTransition;
    }

  private:
    void SetTimingProperties(Seconds delay, Seconds duration) noexcept;
    KRYS_NODISCARD Ref<StyleOriginatedAnimationEvent>
      CreateEvent(dom::DOMStringAtom eventType, Maybe<Seconds> scheduledTime, double elapsedTime,
                  const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept final;
    void AnimationDidFinish() noexcept final;
    KRYS_NODISCARD bool IsCSSTransition() const noexcept final
    {
      return true;
    }
    KRYS_NODISCARD AnimationPlayState BackingAnimationPlayState() const noexcept final;
    KRYS_NODISCARD boo::TimingFunction *BackingAnimationTimingFunction() const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_WEB_ANIMATION(krys::boo::CSSTransition, IsCSSTransition())
