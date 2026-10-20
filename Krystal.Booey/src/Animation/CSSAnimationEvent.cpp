#include "Krystal.Booey/Animation/CSSAnimationEvent.hpp"

namespace krys::boo
{
  CSSAnimationEvent::CSSAnimationEvent(dom::DOMStringAtom type, WebAnimation *animation,
                                       Maybe<Seconds> scheduledTime, double elapsedTime,
                                       const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier,
                                       css::CSSOMString animationName) noexcept
      : StyleOriginatedAnimationEvent(type, animation, scheduledTime, elapsedTime, pseudoElementIdentifier),
        _animationName(krys::move(animationName))
  {
  }

  CSSAnimationEvent::CSSAnimationEvent(dom::DOMStringAtom type,
                                       const CSSAnimationEventInit &eventInitDict) noexcept
      : StyleOriginatedAnimationEvent(type, eventInitDict, eventInitDict.ElapsedTime,
                                      eventInitDict.PseudoElement),
        _animationName(eventInitDict.AnimationName)
  {
  }

  CSSAnimationEvent::~CSSAnimationEvent() noexcept = default;
}
