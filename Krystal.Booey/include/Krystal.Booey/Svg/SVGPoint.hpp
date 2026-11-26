/*
 * Copyright (C) 2013 Samsung Electronics. All rights reserved.
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

#include "Krystal.Booey/Dom/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Svg/Properties/SVGPropertyTraits.hpp"
#include "Krystal.Booey/Svg/Properties/SVGValueProperty.hpp"
#include "Krystal.Booey/Svg/SVGMatrix.hpp"
#include "Krystal.Core/Geometry/Point.hpp"

namespace krys::boo
{
  /// @brief The deprecated alias of `DOMPoint` that SVG's interfaces still use.
  /// @see https://svgwg.org/svg2-draft/shapes.html#InterfaceSVGPointList
  class SVGPoint : public SVGValueProperty<PointF>
  {
    using Base = SVGValueProperty<PointF>;
    using Base::_value;
    using Base::Base;

  public:
    KRYS_NODISCARD static Ref<SVGPoint> Create(const PointF &value = {}) noexcept
    {
      return AdoptRef(*new SVGPoint(value));
    }

    template <typename T>
    KRYS_NODISCARD static ExceptionOr<Ref<SVGPoint>> Create(ExceptionOr<T> value) noexcept
    {
      if (value.HasException())
      {
        return value.ReleaseException();
      }
      return AdoptRef(*new SVGPoint(value.ReleaseValue()));
    }

    KRYS_NODISCARD Ref<SVGPoint> Clone() const noexcept
    {
      return SVGPoint::Create(_value);
    }

    KRYS_NODISCARD float X() noexcept
    {
      return _value.X();
    }

    ExceptionOr<void> SetX(float x) noexcept
    {
      if (IsReadOnly())
      {
        return Exception {ExceptionCode::NoModificationAllowedError};
      }

      _value.SetX(x);
      CommitChange();
      return {};
    }

    KRYS_NODISCARD float Y() noexcept
    {
      return _value.Y();
    }

    ExceptionOr<void> SetY(float y) noexcept
    {
      if (IsReadOnly())
      {
        return Exception {ExceptionCode::NoModificationAllowedError};
      }

      _value.SetY(y);
      CommitChange();
      return {};
    }

    KRYS_NODISCARD Ref<SVGPoint> MatrixTransform(SVGMatrix &matrix) const noexcept
    {
      auto newPoint = krys::MatrixTransform(_value, matrix.Value());
      return AdoptRef(*new SVGPoint(newPoint));
    }

  private:
    KRYS_NODISCARD DOMString ValueAsString() const noexcept override
    {
      return SVGPropertyTraits<PointF>::ToString(_value);
    }
  };
}
