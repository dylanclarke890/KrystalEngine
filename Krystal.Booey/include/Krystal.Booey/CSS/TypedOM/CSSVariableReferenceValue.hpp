/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/TypedOM/CSSStyleValue.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"

namespace krys::boo::css
{
  class CSSUnparsedValue;

  /// @brief A variable reference (`var()`) in an unparsed value.
  /// @see https://drafts.css-houdini.org/css-typed-om/#cssvariablereferencevalue
  class CSSVariableReferenceValue : public RefCounted<CSSVariableReferenceValue>
  {
    KRYS_NON_COPY_MOVABLE(CSSVariableReferenceValue);

  private:
    CSSOMString _variable;
    RefPtr<CSSUnparsedValue> _fallback;

  public:
    /// @return A `TypeError` if @p variable isn't a custom property name.
    /// @see
    /// https://drafts.css-houdini.org/css-typed-om/#dom-cssvariablereferencevalue-cssvariablereferencevalue
    KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSVariableReferenceValue>>
      Create(CSSOMString variable, RefPtr<CSSUnparsedValue> fallback = {}) noexcept;

    ~CSSVariableReferenceValue() noexcept;

    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-cssvariablereferencevalue-variable
    KRYS_NODISCARD const CSSOMString &Variable() const noexcept
    {
      return _variable;
    }
    /// @return A `TypeError`, leaving the variable unchanged, if @p variable isn't a custom property name.
    dom::ExceptionOr<void> SetVariable(CSSOMString variable) noexcept;
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-cssvariablereferencevalue-fallback
    KRYS_NODISCARD CSSUnparsedValue *Fallback() const noexcept
    {
      return _fallback.get();
    }

    KRYS_NODISCARD CSSOMString ToString() const noexcept;
    void Serialise(StringBuilder &builder, EnumSet<SerialisationArguments> arguments = {}) const noexcept;

  private:
    CSSVariableReferenceValue(CSSOMString variable, RefPtr<CSSUnparsedValue> fallback) noexcept;
  };
}
