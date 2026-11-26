/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 * Copyright (C) 2018-2024 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#pragma once

#include "Krystal.Booey/Dom/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Svg/Properties/SVGPropertyTraits.hpp"
#include "Krystal.Booey/Svg/Properties/SVGValueProperty.hpp"
#include "Krystal.Core/Geometry/Rect.hpp"

namespace krys::boo
{
  /// @brief The deprecated alias of `DOMRect` that SVG's interfaces still use.
  /// @see https://svgwg.org/svg2-draft/types.html#InterfaceSVGAnimatedRect
  class SVGRect : public SVGValueProperty<RectF>
  {
    using Base = SVGValueProperty<RectF>;
    using Base::_value;
    using Base::Base;

  public:
    KRYS_NODISCARD static Ref<SVGRect> Create(const RectF &value = {}) noexcept
    {
      return AdoptRef(*new SVGRect(value));
    }

    KRYS_NODISCARD static Ref<SVGRect> Create(SVGPropertyOwner *owner, SVGPropertyAccess access,
                                              const RectF &value = {}) noexcept
    {
      return AdoptRef(*new SVGRect(owner, access, value));
    }

    template <typename T>
    KRYS_NODISCARD static ExceptionOr<Ref<SVGRect>> Create(ExceptionOr<T> value) noexcept
    {
      if (value.HasException())
      {
        return value.ReleaseException();
      }
      return AdoptRef(*new SVGRect(value.ReleaseValue()));
    }

    KRYS_NODISCARD float X() noexcept
    {
      return _value.X();
    }

    ExceptionOr<void> SetX(float xValue) noexcept
    {
      if (IsReadOnly())
      {
        return Exception {ExceptionCode::NoModificationAllowedError};
      }

      _value.SetX(xValue);
      CommitChange();
      return {};
    }

    KRYS_NODISCARD float Y() noexcept
    {
      return _value.Y();
    }

    ExceptionOr<void> SetY(float yValue) noexcept
    {
      if (IsReadOnly())
      {
        return Exception {ExceptionCode::NoModificationAllowedError};
      }

      _value.SetY(yValue);
      CommitChange();
      return {};
    }

    KRYS_NODISCARD float Width() noexcept
    {
      return _value.Width();
    }

    ExceptionOr<void> SetWidth(float widthValue) noexcept
    {
      if (IsReadOnly())
      {
        return Exception {ExceptionCode::NoModificationAllowedError};
      }

      _value.SetWidth(widthValue);
      CommitChange();
      return {};
    }

    KRYS_NODISCARD float Height() noexcept
    {
      return _value.Height();
    }

    ExceptionOr<void> SetHeight(float heightValue) noexcept
    {
      if (IsReadOnly())
      {
        return Exception {ExceptionCode::NoModificationAllowedError};
      }

      _value.SetHeight(heightValue);
      CommitChange();
      return {};
    }

    KRYS_NODISCARD DOMString ValueAsString() const noexcept override
    {
      return SVGPropertyTraits<RectF>::ToString(_value);
    }
  };
}
