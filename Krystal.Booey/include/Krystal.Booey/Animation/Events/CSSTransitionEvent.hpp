/*
 * Copyright (C) 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2013 Intel Corporation. All rights reserved.
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

#pragma once

#include "Krystal.Booey/Animation/Events/StyleOriginatedAnimationEvent.hpp"

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
