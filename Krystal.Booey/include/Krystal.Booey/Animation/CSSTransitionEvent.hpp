#pragma once

#include "Krystal.Booey/Animation/StyleOriginatedAnimationEvent.hpp"

namespace krys::boo
{
  /// @see https://drafts.csswg.org/css-transitions-1/#dictdef-transitioneventinit
  struct CSSTransitionEventInit : public dom::EventInit
  {
    css::CSSOMString PropertyName;
    double ElapsedTime {0};
    css::CSSOMString PseudoElement;
  };

  /// @brief Fired at the owning element of a CSS transition as it runs, starts, ends or is canceled.
  /// @see https://drafts.csswg.org/css-transitions-1/#interface-transitionevent
  class CSSTransitionEvent final : public StyleOriginatedAnimationEvent
  {
  private:
    css::CSSOMString _propertyName;

  public:
    /// @brief The dictionary its constructor takes.
    using dictionary_type = CSSTransitionEventInit;

    CSSTransitionEvent(dom::DOMStringAtom type, WebAnimation *animation, Maybe<Seconds> scheduledTime,
                       double elapsedTime,
                       const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier,
                       css::CSSOMString propertyName) noexcept;

    /// @see https://drafts.csswg.org/css-transitions-1/#dom-transitionevent-transitionevent
    explicit CSSTransitionEvent(dom::DOMStringAtom type,
                                const CSSTransitionEventInit &eventInitDict = {}) noexcept;

    ~CSSTransitionEvent() noexcept override;

    KRYS_NODISCARD bool IsCSSTransitionEvent() const noexcept final
    {
      return true;
    }

    /// @see https://drafts.csswg.org/css-transitions-1/#dom-transitionevent-propertyname
    KRYS_NODISCARD const css::CSSOMString &PropertyName() const noexcept
    {
      return _propertyName;
    }
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_ANIMATION_EVENT_BASE(krys::boo::CSSTransitionEvent, IsCSSTransitionEvent())
