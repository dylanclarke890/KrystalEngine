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

#include "Krystal.Booey/SVG/Properties/SVGProperty.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::svg
{
  /// @brief An SVG property holding a value of `TProperty`.
  template <typename TProperty>
  class SVGValueProperty : public SVGProperty
  {
  protected:
    TProperty _value;

  public:
    using ValueType = TProperty;

    KRYS_NODISCARD static Ref<SVGValueProperty> Create() noexcept
    {
      return AdoptRef(*new SVGValueProperty());
    }

    // Getter/Setter for the value.
    KRYS_NODISCARD const TProperty &Value() const noexcept
    {
      return _value;
    }

    void SetValue(const TProperty &value) noexcept
    {
      _value = value;
    }

    // Used by the SVGAnimatedPropertyAnimator to pass _value to SVGAnimationFunction.
    KRYS_NODISCARD TProperty &Value() noexcept
    {
      return _value;
    }

  protected:
    // Create an initialized property, e.g creating an item to be appended in an SVGList.
    SVGValueProperty(const TProperty &value) noexcept : _value(value)
    {
    }

    // Needed when value should not be copied, e.g. SVGTransformValue.
    SVGValueProperty(TProperty &&value) noexcept : _value(krys::move(value))
    {
    }

    // Base and default constructor.
    SVGValueProperty(SVGPropertyOwner *owner = nullptr,
                     SVGPropertyAccess access = SVGPropertyAccess::ReadWrite) noexcept
        : SVGProperty(owner, access)
    {
    }

    // Create an initialized and attached property.
    SVGValueProperty(SVGPropertyOwner *owner, SVGPropertyAccess access, const TProperty &value) noexcept
        : SVGProperty(owner, access), _value(value)
    {
    }
  };
}
