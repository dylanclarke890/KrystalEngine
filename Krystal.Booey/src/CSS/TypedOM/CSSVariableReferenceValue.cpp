/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER “AS IS” AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "Krystal.Booey/CSS/TypedOM/CSSVariableReferenceValue.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSUnparsedValue.hpp"

namespace krys::boo::css
{
  dom::ExceptionOr<Ref<CSSVariableReferenceValue>>
    CSSVariableReferenceValue::Create(CSSOMString variable, RefPtr<CSSUnparsedValue> fallback) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-cssvariablereferencevalue-cssvariablereferencevalue
    if (!variable.starts_with(u8"--"))
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    return AdoptRef(*new CSSVariableReferenceValue(krys::move(variable), krys::move(fallback)));
  }

  CSSVariableReferenceValue::CSSVariableReferenceValue(CSSOMString variable,
                                                       RefPtr<CSSUnparsedValue> fallback) noexcept
      : _variable(krys::move(variable)), _fallback(krys::move(fallback))
  {
  }

  CSSVariableReferenceValue::~CSSVariableReferenceValue() noexcept = default;

  dom::ExceptionOr<void> CSSVariableReferenceValue::SetVariable(CSSOMString variable) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-cssvariablereferencevalue-variable
    if (!variable.starts_with(u8"--"))
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    _variable = krys::move(variable);
    return {};
  }

  CSSOMString CSSVariableReferenceValue::ToString() const noexcept
  {
    StringBuilder builder;
    Serialise(builder);
    return krys::move(builder).ToString();
  }

  void CSSVariableReferenceValue::Serialise(StringBuilder &builder,
                                            EnumSet<SerialisationArguments> arguments) const noexcept
  {
    builder.Append(u8"var(", _variable);
    if (_fallback)
    {
      builder.Append(u8',');
      _fallback->Serialise(builder, arguments);
    }
    builder.Append(u8')');
  }
}
