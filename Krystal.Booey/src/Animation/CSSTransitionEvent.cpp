#include "Krystal.Booey/Animation/CSSTransitionEvent.hpp"

namespace krys::boo
{
  CSSTransitionEvent::CSSTransitionEvent(dom::DOMStringAtom type, WebAnimation *animation,
                                         Maybe<Seconds> scheduledTime, double elapsedTime,
                                         const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier,
                                         css::CSSOMString propertyName) noexcept
      : StyleOriginatedAnimationEvent(type, animation, scheduledTime, elapsedTime, pseudoElementIdentifier),
        _propertyName(krys::move(propertyName))
  {
  }

  CSSTransitionEvent::CSSTransitionEvent(dom::DOMStringAtom type,
                                         const CSSTransitionEventInit &eventInitDict) noexcept
      : StyleOriginatedAnimationEvent(type, eventInitDict, eventInitDict.ElapsedTime,
                                      eventInitDict.PseudoElement),
        _propertyName(eventInitDict.PropertyName)
  {
  }

  CSSTransitionEvent::~CSSTransitionEvent() noexcept = default;
}
