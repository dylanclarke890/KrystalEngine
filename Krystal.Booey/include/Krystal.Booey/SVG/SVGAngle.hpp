/*
 * Copyright (C) 2016-2019 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/SVG/Properties/SVGValueProperty.hpp"
#include "Krystal.Booey/SVG/SVGAngleValue.hpp"

namespace krys::boo::svg
{
  /// @see https://svgwg.org/svg2-draft/types.html#InterfaceSVGAngle
  class SVGAngle : public SVGValueProperty<SVGAngleValue>
  {
    using Base = SVGValueProperty<SVGAngleValue>;
    using Base::_value;
    using Base::Base;

  public:
    KRYS_NODISCARD static Ref<SVGAngle> Create(const SVGAngleValue &value = {}) noexcept
    {
      return AdoptRef(*new SVGAngle(value));
    }

    KRYS_NODISCARD static Ref<SVGAngle> Create(SVGPropertyOwner *owner, SVGPropertyAccess access,
                                               const SVGAngleValue &value = {}) noexcept
    {
      return AdoptRef(*new SVGAngle(owner, access, value));
    }

    template <typename T>
    KRYS_NODISCARD static dom::ExceptionOr<Ref<SVGAngle>> Create(dom::ExceptionOr<T> &&value) noexcept
    {
      if (value.HasException())
      {
        return value.ReleaseException();
      }
      return AdoptRef(*new SVGAngle(value.ReleaseValue()));
    }

    KRYS_NODISCARD SVGAngleValue::Type UnitType() const noexcept
    {
      // Per spec https://svgwg.org/svg2-draft/types.html#__svg__SVGAngle__SVG_ANGLETYPE_UNKNOWN
      if (_value.UnitType() > SVGAngleValue::Type::SVG_ANGLETYPE_GRAD)
      {
        return SVGAngleValue::Type::SVG_ANGLETYPE_UNKNOWN;
      }
      return _value.UnitType();
    }

    dom::ExceptionOr<void> SetValueForBindings(float value) noexcept
    {
      if (IsReadOnly())
      {
        return dom::Exception {dom::ExceptionCode::NoModificationAllowedError};
      }

      _value.SetValue(value);
      CommitChange();
      return {};
    }

    KRYS_NODISCARD float ValueForBindings() const noexcept
    {
      return _value.Value();
    }

    dom::ExceptionOr<void> SetValueInSpecifiedUnits(float valueInSpecifiedUnits) noexcept
    {
      if (IsReadOnly())
      {
        return dom::Exception {dom::ExceptionCode::NoModificationAllowedError};
      }

      _value.SetValueInSpecifiedUnits(valueInSpecifiedUnits);
      CommitChange();
      return {};
    }

    KRYS_NODISCARD float ValueInSpecifiedUnits() const noexcept
    {
      return _value.ValueInSpecifiedUnits();
    }

    dom::ExceptionOr<void> SetValueAsString(dom::DOMStringView value) noexcept
    {
      if (IsReadOnly())
      {
        return dom::Exception {dom::ExceptionCode::NoModificationAllowedError};
      }

      auto result = _value.SetValueAsString(value);
      if (result.HasException())
      {
        return result;
      }

      CommitChange();
      return result;
    }

    KRYS_NODISCARD dom::DOMString ValueAsString() const noexcept override
    {
      return _value.ValueAsString();
    }

    dom::ExceptionOr<void> NewValueSpecifiedUnits(uint16 unitType, float valueInSpecifiedUnits) noexcept
    {
      if (IsReadOnly())
      {
        return dom::Exception {dom::ExceptionCode::NoModificationAllowedError};
      }

      auto result = _value.NewValueSpecifiedUnits(unitType, valueInSpecifiedUnits);
      if (result.HasException())
      {
        return result;
      }

      CommitChange();
      return result;
    }

    dom::ExceptionOr<void> ConvertToSpecifiedUnits(uint16 unitType) noexcept
    {
      if (IsReadOnly())
      {
        return dom::Exception {dom::ExceptionCode::NoModificationAllowedError};
      }

      auto result = _value.ConvertToSpecifiedUnits(unitType);
      if (result.HasException())
      {
        return result;
      }

      CommitChange();
      return result;
    }
  };
}
