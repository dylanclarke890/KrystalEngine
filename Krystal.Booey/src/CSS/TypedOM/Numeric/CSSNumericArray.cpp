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

#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSNumericArray.hpp"

namespace krys::boo::css
{
  Ref<CSSNumericArray> CSSNumericArray::Create(List<CSSNumberish> &&numberishes) noexcept
  {
    List<Ref<CSSNumericValue>> values;
    values.reserve(numberishes.size());
    for (auto &numberish : numberishes)
    {
      values.push_back(CSSNumericValue::RectifyNumberish(krys::move(numberish)));
    }
    return AdoptRef(*new CSSNumericArray(krys::move(values)));
  }

  Ref<CSSNumericArray> CSSNumericArray::Create(List<Ref<CSSNumericValue>> &&values) noexcept
  {
    return AdoptRef(*new CSSNumericArray(krys::move(values)));
  }

  CSSNumericArray::CSSNumericArray(List<Ref<CSSNumericValue>> &&values) noexcept : _array(krys::move(values))
  {
  }

  RefPtr<CSSNumericValue> CSSNumericArray::Item(size_t index) noexcept
  {
    if (index >= _array.size())
    {
      return nullptr;
    }
    return _array[index];
  }

  void CSSNumericArray::ForEach(const Func<void(const CSSNumericValue &, bool first)> &function) noexcept
  {
    for (size_t i = 0; i < _array.size(); ++i)
    {
      function(*_array[i], !i);
    }
  }
}
