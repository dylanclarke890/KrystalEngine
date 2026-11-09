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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/Selectors/SelectorEnums.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  /// @brief The result of looking up a name written after a single colon: a pseudo-class, or one of the
  /// pseudo-elements that can also be written with a single colon (`:before`, `:after`, `:first-line`,
  /// `:first-letter`).
  struct PseudoClassOrCompatibilityPseudoElement
  {
    Maybe<SelectorPseudoClass> PseudoClass;
    Maybe<SelectorPseudoElement> CompatibilityPseudoElement;
  };

  /// @brief Looks up the name of a pseudo-class or single-colon pseudo-element, ignoring ASCII case.
  /// @param name The name without the leading colon.
  /// @return Both members are empty if the name is unknown.
  KRYS_NODISCARD PseudoClassOrCompatibilityPseudoElement
    FindPseudoClassAndCompatibilityElementName(CSSOMStringView name) noexcept;

  /// @brief Looks up the name of a pseudo-element, ignoring ASCII case.
  /// @param name The name without the leading colons.
  /// @return `SelectorPseudoElement::UserAgentPart` for the parts of a user agent shadow tree, or nothing if
  /// the name is unknown.
  KRYS_NODISCARD Maybe<SelectorPseudoElement> FindPseudoElementName(CSSOMStringView name) noexcept;
}
