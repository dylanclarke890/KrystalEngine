/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Animation/Events/AnimationEventBase.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Style/PseudoElementIdentifier.hpp"

namespace krys::boo
{
  /// @brief An event about a CSS animation or a CSS transition, fired at its owning element.
  class StyleOriginatedAnimationEvent : public AnimationEventBase
  {
  private:
    double _elapsedTime;
    css::CSSOMString _pseudoElement;
    Maybe<style::PseudoElementIdentifier> _pseudoElementIdentifier;

  protected:
    StyleOriginatedAnimationEvent(
      dom::DOMStringAtom type, WebAnimation *animation, Maybe<Seconds> scheduledTime, double elapsedTime,
      const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept;
    StyleOriginatedAnimationEvent(dom::DOMStringAtom type, const dom::EventInit &eventInitDict,
                                  double elapsedTime, const css::CSSOMString &pseudoElement) noexcept;

  public:
    ~StyleOriginatedAnimationEvent() noexcept override;

    /// @brief In seconds.
    KRYS_NODISCARD double ElapsedTime() const noexcept
    {
      return _elapsedTime;
    }

    KRYS_NODISCARD const css::CSSOMString &PseudoElement() noexcept;

    KRYS_NODISCARD const Maybe<style::PseudoElementIdentifier> &PseudoElementIdentifier() const noexcept
    {
      return _pseudoElementIdentifier;
    }
  };
}
