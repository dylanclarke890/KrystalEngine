/*
 * Copyright (C) 2018-2019 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/SVG/Properties/SVGPropertyList.hpp"

namespace krys::boo::svg
{
  /// @brief A list of value properties (`SVGLength`, `SVGNumber`, `SVGPoint`, `SVGTransform`).
  template <typename TProperty>
  class SVGValuePropertyList : public SVGPropertyList<TProperty>
  {
  public:
    using Base = SVGPropertyList<TProperty>;
    using Base::ClearItems;
    using Base::Items;
    using Base::Size;

    SVGValuePropertyList &operator=(const SVGValuePropertyList &other) noexcept
    {
      ClearItems();
      for (const auto &item : other.Items())
      {
        Append(TProperty::Create(item->Value()));
      }
      return *this;
    }

    // This casting operator returns a List of the underlying ValueType,
    // for example List<float> from SVGNumberList.
    operator List<typename TProperty::ValueType>() const noexcept
    {
      List<typename TProperty::ValueType> values;
      for (const auto &item : Items())
      {
        values.push_back(item->Value());
      }
      return values;
    }

    void Resize(size_t newSize) noexcept
    {
      // Add new items.
      while (Size() < newSize)
      {
        Append(TProperty::Create());
      }

      // Remove existing items.
      while (Size() > newSize)
      {
        RemoveAt(static_cast<uint32>(Size() - 1));
      }
    }

  protected:
    using Base::Append;
    using Base::RemoveAt;

    // Base and default constructor.
    SVGValuePropertyList(SVGPropertyOwner *owner = nullptr,
                         SVGPropertyAccess access = SVGPropertyAccess::ReadWrite) noexcept
        : Base(owner, access)
    {
    }

    // Used by SVGAnimatedPropertyList when creating it animVal from baseVal.
    SVGValuePropertyList(const SVGValuePropertyList &other,
                         SVGPropertyAccess access = SVGPropertyAccess::ReadWrite) noexcept
        : Base(other.Owner(), access)
    {
      // Clone all items.
      for (const auto &item : other.Items())
      {
        Append(TProperty::Create(item->Value()));
      }
    }
  };
}
