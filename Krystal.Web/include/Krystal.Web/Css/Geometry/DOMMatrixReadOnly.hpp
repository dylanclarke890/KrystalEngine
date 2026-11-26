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

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/Array.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Containers/Span.hpp"
#include "Krystal.Core/Geometry/TransformationMatrix.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include "Krystal.Web/Css/Geometry/DOMMatrixInit.hpp"
#include "Krystal.Web/Css/Types/CSSOMString.hpp"
#include "Krystal.Web/Dom/Types/ExceptionOr.hpp"

namespace krys::web
{
  class DOMMatrix;
  class DOMPoint;
  struct DOMPointInit;

  /// @brief A 4 by 4 matrix (or a 2D one) that can't be changed through it.
  /// @see https://drafts.csswg.org/geometry-1/#DOMMatrix
  class DOMMatrixReadOnly : public RefCounted<DOMMatrixReadOnly>
  {
  protected:
    TransformationMatrix _matrix;
    /// @see https://drafts.csswg.org/geometry-1/#matrix-is-2d
    bool _is2D {true};

  public:
    enum class Is2D : bool
    {
      No,
      Yes,
    };

#pragma region DOMMatrixReadOnly - https://drafts.csswg.org/geometry-1/#dommatrixreadonly

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-dommatrixreadonly
    KRYS_NODISCARD static ExceptionOr<Ref<DOMMatrixReadOnly>>
      Create(Maybe<Variant<CSSOMString, List<double>>> init = null) noexcept;

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-frommatrix
    KRYS_NODISCARD static ExceptionOr<Ref<DOMMatrixReadOnly>> FromMatrix(DOMMatrixInit other = {}) noexcept;

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-fromfloat32array
    KRYS_NODISCARD static ExceptionOr<Ref<DOMMatrixReadOnly>>
      FromFloat32Array(Span<const float> array32) noexcept;

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-fromfloat64array
    KRYS_NODISCARD static ExceptionOr<Ref<DOMMatrixReadOnly>>
      FromFloat64Array(Span<const double> array64) noexcept;

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-a
    KRYS_NODISCARD double A() const noexcept
    {
      return _matrix.A();
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-b
    KRYS_NODISCARD double B() const noexcept
    {
      return _matrix.B();
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-c
    KRYS_NODISCARD double C() const noexcept
    {
      return _matrix.C();
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-d
    KRYS_NODISCARD double D() const noexcept
    {
      return _matrix.D();
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-e
    KRYS_NODISCARD double E() const noexcept
    {
      return _matrix.E();
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-f
    KRYS_NODISCARD double F() const noexcept
    {
      return _matrix.F();
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m11
    KRYS_NODISCARD double M11() const noexcept
    {
      return _matrix.M11();
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m12
    KRYS_NODISCARD double M12() const noexcept
    {
      return _matrix.M12();
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m13
    KRYS_NODISCARD double M13() const noexcept
    {
      return _matrix.M13();
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m14
    KRYS_NODISCARD double M14() const noexcept
    {
      return _matrix.M14();
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m21
    KRYS_NODISCARD double M21() const noexcept
    {
      return _matrix.M21();
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m22
    KRYS_NODISCARD double M22() const noexcept
    {
      return _matrix.M22();
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m23
    KRYS_NODISCARD double M23() const noexcept
    {
      return _matrix.M23();
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m24
    KRYS_NODISCARD double M24() const noexcept
    {
      return _matrix.M24();
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m31
    KRYS_NODISCARD double M31() const noexcept
    {
      return _matrix.M31();
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m32
    KRYS_NODISCARD double M32() const noexcept
    {
      return _matrix.M32();
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m33
    KRYS_NODISCARD double M33() const noexcept
    {
      return _matrix.M33();
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m34
    KRYS_NODISCARD double M34() const noexcept
    {
      return _matrix.M34();
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m41
    KRYS_NODISCARD double M41() const noexcept
    {
      return _matrix.M41();
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m42
    KRYS_NODISCARD double M42() const noexcept
    {
      return _matrix.M42();
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m43
    KRYS_NODISCARD double M43() const noexcept
    {
      return _matrix.M43();
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-m44
    KRYS_NODISCARD double M44() const noexcept
    {
      return _matrix.M44();
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-is2d
    KRYS_NODISCARD bool GetIs2D() const noexcept
    {
      return _is2D;
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-isidentity
    KRYS_NODISCARD bool IsIdentity() const noexcept;

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-translate
    KRYS_NODISCARD Ref<DOMMatrix> Translate(double tx = 0, double ty = 0, double tz = 0) const noexcept;

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-scale
    KRYS_NODISCARD Ref<DOMMatrix> Scale(double scaleX = 1, Maybe<double> scaleY = null, double scaleZ = 1,
                                        double originX = 0, double originY = 0,
                                        double originZ = 0) const noexcept;

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-scalenonuniform
    KRYS_NODISCARD Ref<DOMMatrix> ScaleNonUniform(double scaleX = 1, double scaleY = 1) const noexcept;

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-scale3d
    KRYS_NODISCARD Ref<DOMMatrix> Scale3d(double scale = 1, double originX = 0, double originY = 0,
                                          double originZ = 0) const noexcept;

    /// @note Angles are in degrees.
    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-rotate
    KRYS_NODISCARD Ref<DOMMatrix> Rotate(double rotX = 0, Maybe<double> rotY = null,
                                         Maybe<double> rotZ = null) const noexcept;

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-rotatefromvector
    KRYS_NODISCARD Ref<DOMMatrix> RotateFromVector(double x = 0, double y = 0) const noexcept;

    /// @note The angle is in degrees.
    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-rotateaxisangle
    KRYS_NODISCARD Ref<DOMMatrix> RotateAxisAngle(double x = 0, double y = 0, double z = 0,
                                                  double angle = 0) const noexcept;

    /// @note The angle is in degrees.
    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-skewx
    KRYS_NODISCARD Ref<DOMMatrix> SkewX(double sx = 0) const noexcept;

    /// @note The angle is in degrees.
    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-skewy
    KRYS_NODISCARD Ref<DOMMatrix> SkewY(double sy = 0) const noexcept;

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-multiply
    KRYS_NODISCARD ExceptionOr<Ref<DOMMatrix>> Multiply(DOMMatrixInit other = {}) const noexcept;

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-flipx
    KRYS_NODISCARD Ref<DOMMatrix> FlipX() const noexcept;

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-flipy
    KRYS_NODISCARD Ref<DOMMatrix> FlipY() const noexcept;

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-inverse
    KRYS_NODISCARD Ref<DOMMatrix> Inverse() const noexcept;

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-transformpoint
    KRYS_NODISCARD Ref<DOMPoint> TransformPoint(const DOMPointInit &point) const noexcept;

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-tofloat32array
    KRYS_NODISCARD Array<float, 16> ToFloat32Array() const noexcept;

    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-tofloat64array
    KRYS_NODISCARD Array<double, 16> ToFloat64Array() const noexcept;

    /// @see https://drafts.csswg.org/geometry-1/#dommatrixreadonly-stringification-behavior
    KRYS_NODISCARD ExceptionOr<CSSOMString> ToString() const noexcept;

#pragma endregion

    /// @brief A matrix with @p matrix's elements, 2D if @p is2D says so.
    KRYS_NODISCARD static Ref<DOMMatrixReadOnly> Create(const TransformationMatrix &matrix,
                                                        Is2D is2D) noexcept
    {
      return AdoptRef(*new DOMMatrixReadOnly(matrix, is2D));
    }

    /// @see https://drafts.csswg.org/geometry-1/#matrix-validate-and-fixup-2d
    KRYS_NODISCARD static ExceptionOr<void> ValidateAndFixup(DOMMatrix2DInit &dict) noexcept;

    /// @see https://drafts.csswg.org/geometry-1/#matrix-validate-and-fixup
    KRYS_NODISCARD static ExceptionOr<void> ValidateAndFixup(DOMMatrixInit &dict) noexcept;

    KRYS_NODISCARD const TransformationMatrix &GetTransformationMatrix() const noexcept
    {
      return _matrix;
    }

    /// @brief A mutable matrix with this one's elements.
    KRYS_NODISCARD Ref<DOMMatrix> CloneAsDOMMatrix() const noexcept;

  protected:
    DOMMatrixReadOnly() noexcept = default;

    DOMMatrixReadOnly(const TransformationMatrix &, Is2D) noexcept;

    /// @see https://drafts.csswg.org/geometry-1/#parse-a-string-into-an-abstract-matrix
    struct AbstractMatrix
    {
      TransformationMatrix Matrix;
      bool Is2D {true};
    };

    /// @see https://drafts.csswg.org/geometry-1/#parse-a-string-into-an-abstract-matrix
    KRYS_NODISCARD static ExceptionOr<AbstractMatrix>
      ParseStringIntoAbstractMatrix(CSSOMString transformList) noexcept;

    /// @brief The DOMMatrixReadOnly or DOMMatrix constructor's steps, as @p T.
    /// @see https://drafts.csswg.org/geometry-1/#dom-dommatrixreadonly-dommatrixreadonly
    template <typename T>
    KRYS_NODISCARD static ExceptionOr<Ref<T>>
      CreateHelper(Maybe<Variant<CSSOMString, List<double>>> init) noexcept;

    /// @brief A 2D matrix of 6 values or a 3D matrix of 16, as @p T, and a TypeError for other lengths.
    /// @see https://drafts.csswg.org/geometry-1/#create-a-2d-matrix
    /// @see https://drafts.csswg.org/geometry-1/#create-a-3d-matrix
    template <typename T, typename TValue>
    KRYS_NODISCARD static ExceptionOr<Ref<T>> FromArrayHelper(Span<const TValue> init) noexcept;

    /// @see https://drafts.csswg.org/geometry-1/#create-a-dommatrixreadonly-from-the-dictionary
    template <typename T>
    KRYS_NODISCARD static ExceptionOr<Ref<T>> FromMatrixHelper(DOMMatrixInit other) noexcept;
  };

  template <typename T, typename TValue>
  inline ExceptionOr<Ref<T>> DOMMatrixReadOnly::FromArrayHelper(Span<const TValue> init) noexcept
  {
    if (init.size() == 6)
    {
      // To create a 2d matrix of type T, with a sequence init of 6 elements:
      // 1. Let matrix be a new instance of type.
      // 2. Set m11 element, m12 element, m21 element, m22 element, m41 element and m42 element to the values
      // of
      //    init in order starting with the first value.
      // 3. Set m13 element, m14 element, m23 element, m24 element, m31 element, m32 element, m34 element, and
      // m43
      //    element to 0.
      // 4. Set m33 element and m44 element to 1.
      // 5. Set is 2D to true.
      // 6. Return matrix
      return T::Create(TransformationMatrix {init[0], init[1], init[2], init[3], init[4], init[5]},
                       Is2D::Yes);
    }
    if (init.size() == 16)
    {
      // To create a 3d matrix of type T, with a sequence init of 16 elements:
      // 1. Let matrix be a new instance of type.
      // 2. Set m11 element to m44 element to the values of init in column-major order.
      // 3. Set is 2D to false.
      // 4. Return matrix
      return T::Create(TransformationMatrix {init[0], init[1], init[2], init[3], init[4], init[5], init[6],
                                             init[7], init[8], init[9], init[10], init[11], init[12],
                                             init[13], init[14], init[15]},
                       Is2D::No);
    }
    // Otherwise: Throw a TypeError exception.
    return Exception {ExceptionCode::TypeError};
  }

  template <typename T>
  inline ExceptionOr<Ref<T>> DOMMatrixReadOnly::FromMatrixHelper(DOMMatrixInit other) noexcept
  {
    // 1. Validate and fixup other.
    if (auto result = ValidateAndFixup(other); result.HasException())
    {
      return result.ReleaseException();
    }

    // 2. If the is2D dictionary member of other is true
    if (other.Is2D.value())
    {
      // Return the result of invoking create a 2d matrix of type DOMMatrixReadOnly or DOMMatrix as
      // appropriate, with a sequence of numbers, the values being the 6 elements m11, m12, m21, m22, m41 and
      // m42 of other in the given order.
      return T::Create(TransformationMatrix {other.M11.value(), other.M12.value(), other.M21.value(),
                                             other.M22.value(), other.M41.value(), other.M42.value()},
                       Is2D::Yes);
    }

    // Otherwise, Return the result of invoking create a 3d matrix of type DOMMatrixReadOnly or DOMMatrix as
    // appropriate, with a sequence of numbers, the values being the 16 elements m11, m12, m13, ..., m44 of
    // other in the given order.
    return T::Create(TransformationMatrix {other.M11.value(), other.M12.value(), other.M13, other.M14,
                                           other.M21.value(), other.M22.value(), other.M23, other.M24,
                                           other.M31, other.M32, other.M33, other.M34, other.M41.value(),
                                           other.M42.value(), other.M43, other.M44},
                     Is2D::No);
  }
}
