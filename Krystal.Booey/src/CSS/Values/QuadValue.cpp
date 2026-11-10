/**
 * Copyright (C) 2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Values/QuadValue.hpp"

namespace krys::boo::css
{
  QuadValue::QuadValue(Quad quad) noexcept : Value(ValueType::Quad), _quad(krys::move(quad))
  {
  }

  // A quad value is a value and its quad: its coalescing bit is in the value's bit fields.
  static_assert(sizeof(QuadValue)
                == ((sizeof(Value) + alignof(Quad) - 1) / alignof(Quad) * alignof(Quad)) + sizeof(Quad));

  Ref<QuadValue> QuadValue::Create(Quad quad) noexcept
  {
    return AdoptRef(*new QuadValue(krys::move(quad)));
  }

  CSSOMString QuadValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    return _quad.CssText(context);
  }

  bool QuadValue::Equals(const QuadValue &other) const noexcept
  {
    return _quad.Equals(other._quad);
  }

  bool QuadValue::CanBeCoalesced() const noexcept
  {
    const Value &top = _quad.Top();
    return _coalesceIdenticalValues && top.Equals(_quad.Right()) && top.Equals(_quad.Left())
           && top.Equals(_quad.Bottom());
  }
}
