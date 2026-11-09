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

#include "Krystal.Booey/SVG/Properties/SVGDecoratedPrimitive.hpp"
#include "Krystal.Core/Concepts.hpp"

namespace krys::boo::svg
{
  /// @brief An enumeration the DOM sees as an integer, refusing values outside the enumeration and reporting
  /// those the DOM doesn't expose as 0 (unknown).
  template <typename TDecoration, typename TEnum>
  class SVGDecoratedEnumeration : public SVGDecoratedPrimitive<TDecoration, TEnum>
  {
    using Base = SVGDecoratedPrimitive<TDecoration, TEnum>;
    using Base::_value;
    using Base::Base;

    static constexpr TDecoration OutOfRangeEnumValue = 0;

  public:
    KRYS_NODISCARD static auto Create(const TEnum &value) noexcept
    {
      static_assert(Integral<TDecoration>, "TDecoration of an enumeration should be integral.");
      return AdoptRef(*new SVGDecoratedEnumeration(value));
    }

  private:
    bool SetValue(const TDecoration &value) noexcept override
    {
      if (!value || value > SVGPropertyTraits<TEnum>::HighestEnumValue())
      {
        return false;
      }
      Base::SetValueInternal(value);
      return true;
    }

    KRYS_NODISCARD TDecoration Value() const noexcept override
    {
      if (Base::Value() > SVGPropertyTraits<TEnum>::HighestEnumValue())
      {
        return OutOfRangeEnumValue;
      }
      return Base::Value();
    }

    KRYS_NODISCARD Ref<SVGDecoratedProperty<TDecoration>> Clone() noexcept override
    {
      return Create(_value);
    }
  };
}
