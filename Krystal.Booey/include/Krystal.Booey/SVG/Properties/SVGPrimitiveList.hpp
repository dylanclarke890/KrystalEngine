/*
 * Copyright (C) 2019 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/SVG/Properties/SVGList.hpp"

namespace krys::boo::svg
{
  /// @brief A list of plain values, such as the strings of an `SVGStringList`.
  template <typename TProperty>
  class SVGPrimitiveList : public SVGList<TProperty>
  {
  protected:
    using Base = SVGList<TProperty>;
    using Base::_items;
    using Base::Base;
    using Base::Size;

    KRYS_NODISCARD TProperty At(uint32 index) const noexcept override
    {
      krys_assert(index < Size());
      return _items.at(index);
    }

    TProperty InsertAt(uint32 index, TProperty &&newItem) noexcept override
    {
      krys_assert(index <= Size());
      _items.insert(_items.begin() + index, krys::move(newItem));
      return At(index);
    }

    TProperty ReplaceAt(uint32 index, TProperty &&newItem) noexcept override
    {
      krys_assert(index < Size());
      _items.at(index) = krys::move(newItem);
      return At(index);
    }

    TProperty RemoveAt(uint32 index) noexcept override
    {
      krys_assert(index < Size());
      TProperty item = At(index);
      _items.erase(_items.begin() + index);
      return item;
    }

    TProperty Append(TProperty &&newItem) noexcept override
    {
      _items.push_back(krys::move(newItem));
      return At(static_cast<uint32>(Size() - 1));
    }
  };
}
