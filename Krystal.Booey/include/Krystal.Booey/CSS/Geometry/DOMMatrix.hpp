/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Geometry/DOMMatrixReadOnly.hpp"

namespace krys::boo
{
  /// @brief A 4 by 4 matrix (or a 2D one) that can be changed.
  /// @see https://drafts.csswg.org/geometry-1/#DOMMatrix
  class DOMMatrix : public DOMMatrixReadOnly
  {
    friend class DOMMatrixReadOnly;

  public:
#pragma region DOMMatrix - https://drafts.csswg.org/geometry-1/#dommatrix

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-dommatrixreadonly
    KRYS_NODISCARD static ExceptionOr<Ref<DOMMatrix>>
      Create(Maybe<Variant<CSSOMString, List<double>>> init = null) noexcept;

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrix-frommatrix
    KRYS_NODISCARD static ExceptionOr<Ref<DOMMatrix>> FromMatrix(DOMMatrixInit other = {}) noexcept;

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrix-fromfloat32array
    KRYS_NODISCARD static ExceptionOr<Ref<DOMMatrix>> FromFloat32Array(Span<const float> array32) noexcept;

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrix-fromfloat64array
    KRYS_NODISCARD static ExceptionOr<Ref<DOMMatrix>> FromFloat64Array(Span<const double> array64) noexcept;

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-a
    void SetA(double value) noexcept
    {
      _matrix.SetA(value);
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-b
    void SetB(double value) noexcept
    {
      _matrix.SetB(value);
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-c
    void SetC(double value) noexcept
    {
      _matrix.SetC(value);
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-d
    void SetD(double value) noexcept
    {
      _matrix.SetD(value);
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-e
    void SetE(double value) noexcept
    {
      _matrix.SetE(value);
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-f
    void SetF(double value) noexcept
    {
      _matrix.SetF(value);
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m11
    void SetM11(double value) noexcept
    {
      _matrix.SetM11(value);
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m12
    void SetM12(double value) noexcept
    {
      _matrix.SetM12(value);
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m13
    void SetM13(double value) noexcept
    {
      _matrix.SetM13(value);
      ClearIs2DUnless(value, 0);
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m14
    void SetM14(double value) noexcept
    {
      _matrix.SetM14(value);
      ClearIs2DUnless(value, 0);
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m21
    void SetM21(double value) noexcept
    {
      _matrix.SetM21(value);
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m22
    void SetM22(double value) noexcept
    {
      _matrix.SetM22(value);
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m23
    void SetM23(double value) noexcept
    {
      _matrix.SetM23(value);
      ClearIs2DUnless(value, 0);
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m24
    void SetM24(double value) noexcept
    {
      _matrix.SetM24(value);
      ClearIs2DUnless(value, 0);
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m31
    void SetM31(double value) noexcept
    {
      _matrix.SetM31(value);
      ClearIs2DUnless(value, 0);
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m32
    void SetM32(double value) noexcept
    {
      _matrix.SetM32(value);
      ClearIs2DUnless(value, 0);
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m33
    void SetM33(double value) noexcept
    {
      _matrix.SetM33(value);
      ClearIs2DUnless(value, 1);
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m34
    void SetM34(double value) noexcept
    {
      _matrix.SetM34(value);
      ClearIs2DUnless(value, 0);
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m41
    void SetM41(double value) noexcept
    {
      _matrix.SetM41(value);
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m42
    void SetM42(double value) noexcept
    {
      _matrix.SetM42(value);
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m43
    void SetM43(double value) noexcept
    {
      _matrix.SetM43(value);
      ClearIs2DUnless(value, 0);
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m44
    void SetM44(double value) noexcept
    {
      _matrix.SetM44(value);
      ClearIs2DUnless(value, 1);
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrix-multiplyself
    ExceptionOr<Ref<DOMMatrix>> MultiplySelf(DOMMatrixInit other = {}) noexcept;

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrix-premultiplyself
    ExceptionOr<Ref<DOMMatrix>> PreMultiplySelf(DOMMatrixInit other = {}) noexcept;

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrix-translateself
    Ref<DOMMatrix> TranslateSelf(double tx = 0, double ty = 0, double tz = 0) noexcept;

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrix-scaleself
    Ref<DOMMatrix> ScaleSelf(double scaleX = 1, Maybe<double> scaleY = null, double scaleZ = 1,
                             double originX = 0, double originY = 0, double originZ = 0) noexcept;

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrix-scale3dself
    Ref<DOMMatrix> Scale3dSelf(double scale = 1, double originX = 0, double originY = 0,
                               double originZ = 0) noexcept;

    /// @note Angles are in degrees.
    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrix-rotateself
    Ref<DOMMatrix> RotateSelf(double rotX = 0, Maybe<double> rotY = null, Maybe<double> rotZ = null) noexcept;

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrix-rotatefromvectorself
    Ref<DOMMatrix> RotateFromVectorSelf(double x = 0, double y = 0) noexcept;

    /// @note The angle is in degrees.
    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrix-rotateaxisangleself
    Ref<DOMMatrix> RotateAxisAngleSelf(double x = 0, double y = 0, double z = 0, double angle = 0) noexcept;

    /// @note The angle is in degrees.
    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrix-skewxself
    Ref<DOMMatrix> SkewXSelf(double sx = 0) noexcept;

    /// @note The angle is in degrees.
    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrix-skewyself
    Ref<DOMMatrix> SkewYSelf(double sy = 0) noexcept;

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrix-invertself
    Ref<DOMMatrix> InvertSelf() noexcept;

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrix-setmatrixvalue
    ExceptionOr<Ref<DOMMatrix>> SetMatrixValue(CSSOMString transformList) noexcept;

#pragma endregion

    /// @brief A matrix with @p matrix's elements, 2D if @p is2D says so.
    KRYS_NODISCARD static Ref<DOMMatrix> Create(const TransformationMatrix &matrix, Is2D is2D) noexcept
    {
      return AdoptRef(*new DOMMatrix(matrix, is2D));
    }

  private:
    DOMMatrix() noexcept = default;

    DOMMatrix(const TransformationMatrix &, Is2D) noexcept;

    /// @brief Clears is 2D when an element is set to @p value, unless it's @p identityValue (0 counting -0).
    void ClearIs2DUnless(double value, double identityValue) noexcept
    {
      if (value != identityValue)
      {
        _is2D = false;
      }
    }
  };

  static_assert(sizeof(DOMMatrix) == sizeof(DOMMatrixReadOnly));
}
