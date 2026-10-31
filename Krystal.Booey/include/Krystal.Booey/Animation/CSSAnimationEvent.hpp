#pragma once

#include "Krystal.Booey/Animation/StyleOriginatedAnimationEvent.hpp"

namespace krys::boo
{
  /// @see https://drafts.csswg.org/css-animations-1/#dictdef-animationeventinit
  struct CSSAnimationEventInit : public dom::EventInit
  {
    css::CSSOMString AnimationName;
    double ElapsedTime {0};
    css::CSSOMString PseudoElement;
  };

  /// @brief Fired at the owning element of a CSS animation as it starts, iterates, ends or is canceled.
  /// @see https://drafts.csswg.org/css-animations-1/#interface-animationevent
  class CSSAnimationEvent final : public StyleOriginatedAnimationEvent
  {
  private:
    css::CSSOMString _animationName;

  public:
    /// @brief The dictionary its constructor takes.
    using dictionary_type = CSSAnimationEventInit;

    CSSAnimationEvent(dom::DOMStringAtom type, WebAnimation *animation, Maybe<Seconds> scheduledTime,
                      double elapsedTime,
                      const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier,
                      css::CSSOMString animationName) noexcept;

    /// @see https://drafts.csswg.org/css-animations-1/#dom-animationevent-animationevent
    explicit CSSAnimationEvent(dom::DOMStringAtom type,
                               const CSSAnimationEventInit &eventInitDict = {}) noexcept;

    ~CSSAnimationEvent() noexcept override;

    KRYS_NODISCARD bool IsCSSAnimationEvent() const noexcept final
    {
      return true;
    }

    /// @see https://drafts.csswg.org/css-animations-1/#dom-animationevent-animationname
    KRYS_NODISCARD const css::CSSOMString &AnimationName() const noexcept
    {
      return _animationName;
    }
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_ANIMATION_EVENT_BASE(krys::boo::CSSAnimationEvent, IsCSSAnimationEvent())
