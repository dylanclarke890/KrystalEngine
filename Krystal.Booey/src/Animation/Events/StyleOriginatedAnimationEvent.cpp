/*
 * Copyright (C) 2022-2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Animation/Events/StyleOriginatedAnimationEvent.hpp"
#include "Krystal.Booey/Animation/WebAnimationUtilities.hpp"
#include "Krystal.Booey/DOM/Nodes/Node.hpp"

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
