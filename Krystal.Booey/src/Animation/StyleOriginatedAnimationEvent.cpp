#include "Krystal.Booey/Animation/StyleOriginatedAnimationEvent.hpp"
#include "Krystal.Booey/Animation/WebAnimationUtilities.hpp"
#include "Krystal.Booey/DOM/Node.hpp"

namespace krys::boo
{
  StyleOriginatedAnimationEvent::StyleOriginatedAnimationEvent(
    dom::DOMStringAtom type, WebAnimation *animation, Maybe<Seconds> scheduledTime, double elapsedTime,
    const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept
      : AnimationEventBase(type, animation, scheduledTime), _elapsedTime(elapsedTime),
        _pseudoElementIdentifier(pseudoElementIdentifier)
  {
  }

  StyleOriginatedAnimationEvent::StyleOriginatedAnimationEvent(dom::DOMStringAtom type,
                                                               const dom::EventInit &eventInitDict,
                                                               double elapsedTime,
                                                               const css::CSSOMString &pseudoElement) noexcept
      : AnimationEventBase(type, eventInitDict), _elapsedTime(elapsedTime), _pseudoElement(pseudoElement)
  {
    RefPtr<dom::Node> node = ShareRefPtr(DynamicDowncast<dom::Node>(Target().get()));
    auto [parsed, pseudoElementIdentifier] =
      PseudoElementIdentifierFromString(_pseudoElement, node ? &node->NodeDocument() : nullptr);
    _pseudoElementIdentifier = parsed ? pseudoElementIdentifier : null;
  }

  StyleOriginatedAnimationEvent::~StyleOriginatedAnimationEvent() noexcept = default;

  const css::CSSOMString &StyleOriginatedAnimationEvent::PseudoElement() noexcept
  {
    if (_pseudoElementIdentifier && _pseudoElement.empty())
    {
      _pseudoElement = PseudoElementIdentifierAsString(_pseudoElementIdentifier);
    }
    return _pseudoElement;
  }
}
