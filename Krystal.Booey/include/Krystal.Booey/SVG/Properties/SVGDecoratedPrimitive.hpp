/*
 * Copyright (C) 2019-2024 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/SVG/Properties/SVGDecoratedProperty.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyTraits.hpp"

namespace krys::boo::svg
{
  /// @brief A decorated property kept as a `TProperty` and converted to and from `TDecoration` by a cast.
  template <typename TDecoration, typename TProperty>
  class SVGDecoratedPrimitive : public SVGDecoratedProperty<TDecoration>
  {
    using Base = SVGDecoratedProperty<TDecoration>;

  protected:
    TProperty _value;

  public:
    SVGDecoratedPrimitive(const TProperty &value) noexcept : _value(value)
    {
    }

  protected:
    using Base::Base;

    void SetValueInternal(const TDecoration &value) noexcept override
    {
      _value = static_cast<TProperty>(value);
    }

    KRYS_NODISCARD TDecoration ValueInternal() const noexcept override
    {
      return static_cast<TDecoration>(_value);
    }

    KRYS_NODISCARD dom::DOMString ValueAsString() const noexcept override
    {
      return SVGPropertyTraits<TProperty>::ToString(_value);
    }
  };
}
