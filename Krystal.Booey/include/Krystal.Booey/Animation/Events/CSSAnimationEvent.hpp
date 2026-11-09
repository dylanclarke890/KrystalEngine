/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
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
