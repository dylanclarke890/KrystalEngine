/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Maths/Extras.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::style
{
  // This is used to identify style scopes that can affect an element.
  // Scopes are in tree-of-trees order. Styles from earlier scopes win over later ones (modulo !important).
  enum class ScopeOrdinal : int8
  {
    ContainingHostLimit = Limits<int8>::min(),
    // ::part rules and author-exposed UA pseudo classes from the host tree scope. Values less than
    // ContainingHost indicate enclosing scopes.
    ContainingHost = -1,
    // Normal rules in the same tree where the element is.
    Element = 0,
    // ::slotted rules in the parent's shadow tree. Values greater than FirstSlot indicate subsequent slots in
    // the chain.
    FirstSlot = 1,
    SlotLimit = Limits<int8>::max() - 1,
    // :host rules in element's own shadow tree.
    Shadow = Limits<int8>::max(),
  };

  constexpr ScopeOrdinal &operator++(ScopeOrdinal &ordinal) noexcept
  {
    krys_debug_assert(ordinal < ScopeOrdinal::SlotLimit);
    return ordinal = static_cast<ScopeOrdinal>(ToUnderlying(ordinal) + 1);
  }

  constexpr ScopeOrdinal &operator--(ScopeOrdinal &ordinal) noexcept
  {
    krys_debug_assert(ordinal > ScopeOrdinal::ContainingHostLimit);
    return ordinal = static_cast<ScopeOrdinal>(ToUnderlying(ordinal) - 1);
  }
}
