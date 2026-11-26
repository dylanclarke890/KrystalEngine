/*
 * Copyright (C) 2014 Adobe Systems Incorporated. All rights reserved.
 * Copyright (C) 2016 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Dom/Types/ExceptionOr.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Geometry/Point3.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"

namespace krys::boo
{
  class DOMMatrixReadOnly;
  class DOMPoint;
  struct DOMMatrixInit;

  /// @see https://drafts.csswg.org/geometry-1/#dictdef-dompointinit
  struct DOMPointInit
  {
    double X {0};
    double Y {0};
    double Z {0};
    double W {1};
  };

  /// @brief A point whose coordinates can't be changed through it.
  /// @see https://drafts.csswg.org/geometry-1/#DOMPoint
  class DOMPointReadOnly : public RefCounted<DOMPointReadOnly>
  {
  protected:
    // Any of these can be NaN or Inf.
    double _x;
    double _y;
    double _z;
    double _w;

  public:
#pragma region DOMPointReadOnly - https://drafts.csswg.org/geometry-1/#dompointreadonly

    /// @see https://drafts.csswg.org/geometry-1/#dom-dompointreadonly-dompointreadonly
    KRYS_NODISCARD static Ref<DOMPointReadOnly> Create(double x = 0, double y = 0, double z = 0,
                                                       double w = 1) noexcept;

    /// @see https://drafts.csswg.org/geometry-1/#dom-dompointreadonly-frompoint
    KRYS_NODISCARD static Ref<DOMPointReadOnly> FromPoint(const DOMPointInit &other) noexcept;

    /// @see https://drafts.csswg.org/geometry-1/#dom-dompointreadonly-x
    KRYS_NODISCARD double X() const noexcept
    {
      return _x;
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dompointreadonly-y
    KRYS_NODISCARD double Y() const noexcept
    {
      return _y;
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dompointreadonly-z
    KRYS_NODISCARD double Z() const noexcept
    {
      return _z;
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dompointreadonly-w
    KRYS_NODISCARD double W() const noexcept
    {
      return _w;
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dompointreadonly-matrixtransform
    KRYS_NODISCARD ExceptionOr<Ref<DOMPoint>> MatrixTransform(DOMMatrixInit matrix) const noexcept;

#pragma endregion

    /// @brief A point at @p point, with a w perspective of 1.
    KRYS_NODISCARD static Ref<DOMPointReadOnly> FromPointF(const Point3F &point) noexcept
    {
      return Create(point.X(), point.Y(), point.Z(), 1);
    }

  protected:
    DOMPointReadOnly(double x, double y, double z, double w) noexcept : _x(x), _y(y), _z(z), _w(w)
    {
    }
  };

  /// @brief The point that @p matrix moves @p point to.
  /// @see https://drafts.csswg.org/geometry-1/#transform-a-point-with-a-matrix
  KRYS_NODISCARD Ref<DOMPoint> TransformPointWithMatrix(const DOMPointInit &point,
                                                        const DOMMatrixReadOnly &matrix) noexcept;
}
