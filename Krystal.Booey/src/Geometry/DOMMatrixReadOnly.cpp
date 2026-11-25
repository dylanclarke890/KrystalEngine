/*
 * Copyright (C) 2017-2019 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Geometry/DOMMatrixReadOnly.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Transform.hpp"
#include "Krystal.Booey/Geometry/DOMMatrix.hpp"
#include "Krystal.Booey/Geometry/DOMPoint.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Transform.hpp"
#include "Krystal.Core/Move.hpp"
#include "Krystal.Core/Strings/StringBuilder.hpp"
#include "Krystal.Core/Visitor.hpp"
#include <cmath>

namespace krys::boo
{
  namespace
  {
    /// @see https://tc39.es/ecma262/#sec-samevaluezero
    KRYS_NODISCARD bool SameValueZero(double a, double b) noexcept
    {
      if (std::isnan(a) && std::isnan(b))
      {
        return true;
      }
      return a == b;
    }
  }

  DOMMatrixReadOnly::DOMMatrixReadOnly(const TransformationMatrix &matrix, Is2D is2D) noexcept
      : _matrix(matrix), _is2D(is2D == Is2D::Yes)
  {
    krys_debug_assert(!_is2D || _matrix.IsAffine());
  }

#pragma region DOMMatrixReadOnly - https://drafts.csswg.org/geometry-1/#dommatrixreadonly

  ExceptionOr<Ref<DOMMatrixReadOnly>>
    DOMMatrixReadOnly::Create(Maybe<Variant<CSSOMString, List<double>>> init) noexcept
  {
    return CreateHelper<DOMMatrixReadOnly>(krys::move(init));
  }

  ExceptionOr<Ref<DOMMatrixReadOnly>> DOMMatrixReadOnly::FromMatrix(DOMMatrixInit other) noexcept
  {
    return FromMatrixHelper<DOMMatrixReadOnly>(krys::move(other));
  }

  ExceptionOr<Ref<DOMMatrixReadOnly>> DOMMatrixReadOnly::FromFloat32Array(Span<const float> array32) noexcept
  {
    // If array32 has 6 elements, return the result of invoking create a 2d matrix of type DOMMatrixReadOnly
    // or DOMMatrix as appropriate, with a sequence of numbers taking the values from array32 in the provided
    // order. If array32 has 16 elements, return the result of invoking create a 3d matrix of type
    // DOMMatrixReadOnly or DOMMatrix as appropriate, with a sequence of numbers taking the values from
    // array32 in the provided order. Otherwise, throw a TypeError exception.
    return FromArrayHelper<DOMMatrixReadOnly>(array32);
  }

  ExceptionOr<Ref<DOMMatrixReadOnly>> DOMMatrixReadOnly::FromFloat64Array(Span<const double> array64) noexcept
  {
    // As fromFloat32Array(array32), with array64.
    return FromArrayHelper<DOMMatrixReadOnly>(array64);
  }

  bool DOMMatrixReadOnly::IsIdentity() const noexcept
  {
    // True if m12 element, m13 element, m14 element, m21 element, m23 element, m24 element, m31 element, m32
    // element, m34 element, m41 element, m42 element, m43 element are 0 or -0 and m11 element, m22 element,
    // m33 element, m44 element are 1. Otherwise false.
    return _matrix.IsIdentity();
  }

  Ref<DOMMatrix> DOMMatrixReadOnly::Translate(double tx, double ty, double tz) const noexcept
  {
    // 1. Let result be the resulting matrix initialized to the values of the current matrix.
    auto result = CloneAsDOMMatrix();
    // 2. Perform a translateSelf() transformation on result with the arguments tx, ty, tz.
    result->TranslateSelf(tx, ty, tz);
    // 3. Return result.
    return result;
  }

  Ref<DOMMatrix> DOMMatrixReadOnly::Scale(double scaleX, Maybe<double> scaleY, double scaleZ, double originX,
                                          double originY, double originZ) const noexcept
  {
    // 1. If scaleY is missing, set scaleY to the value of scaleX.
    if (!scaleY)
    {
      scaleY = scaleX;
    }
    // 2. Let result be the resulting matrix initialized to the values of the current matrix.
    auto result = CloneAsDOMMatrix();
    // 3. Perform a scaleSelf() transformation on result with the arguments scaleX, scaleY, scaleZ, originX,
    //    originY, originZ.
    result->ScaleSelf(scaleX, scaleY, scaleZ, originX, originY, originZ);
    // 4. Return result.
    return result;
  }

  Ref<DOMMatrix> DOMMatrixReadOnly::ScaleNonUniform(double scaleX, double scaleY) const noexcept
  {
    // 1. Let result be the resulting matrix initialized to the values of the current matrix.
    auto result = CloneAsDOMMatrix();
    // 2. Perform a scaleSelf() transformation on result with the arguments scaleX, scaleY, 1, 0, 0, 0.
    result->ScaleSelf(scaleX, scaleY, 1, 0, 0, 0);
    // 3. Return result.
    return result;
  }

  Ref<DOMMatrix> DOMMatrixReadOnly::Scale3d(double scale, double originX, double originY,
                                            double originZ) const noexcept
  {
    // 1. Let result be the resulting matrix initialized to the values of the current matrix.
    auto result = CloneAsDOMMatrix();
    // 2. Perform a scale3dSelf() transformation on result with the arguments scale, originX, originY,
    // originZ.
    result->Scale3dSelf(scale, originX, originY, originZ);
    // 3. Return result.
    return result;
  }

  Ref<DOMMatrix> DOMMatrixReadOnly::Rotate(double rotX, Maybe<double> rotY, Maybe<double> rotZ) const noexcept
  {
    // 1. Let result be the resulting matrix initialized to the values of the current matrix.
    auto result = CloneAsDOMMatrix();
    // 2. Perform a rotateSelf() transformation on result with the arguments rotX, rotY, rotZ.
    result->RotateSelf(rotX, rotY, rotZ);
    // 3. Return result.
    return result;
  }

  Ref<DOMMatrix> DOMMatrixReadOnly::RotateFromVector(double x, double y) const noexcept
  {
    // 1. Let result be the resulting matrix initialized to the values of the current matrix.
    auto result = CloneAsDOMMatrix();
    // 2. Perform a rotateFromVectorSelf() transformation on result with the arguments x, y.
    result->RotateFromVectorSelf(x, y);
    // 3. Return result.
    return result;
  }

  Ref<DOMMatrix> DOMMatrixReadOnly::RotateAxisAngle(double x, double y, double z, double angle) const noexcept
  {
    // 1. Let result be the resulting matrix initialized to the values of the current matrix.
    auto result = CloneAsDOMMatrix();
    // 2. Perform a rotateAxisAngleSelf() transformation on result with the arguments x, y, z, angle.
    result->RotateAxisAngleSelf(x, y, z, angle);
    // 3. Return result.
    return result;
  }

  Ref<DOMMatrix> DOMMatrixReadOnly::SkewX(double sx) const noexcept
  {
    // 1. Let result be the resulting matrix initialized to the values of the current matrix.
    auto result = CloneAsDOMMatrix();
    // 2. Perform a skewXSelf() transformation on result with the argument sx.
    result->SkewXSelf(sx);
    // 3. Return result.
    return result;
  }

  Ref<DOMMatrix> DOMMatrixReadOnly::SkewY(double sy) const noexcept
  {
    // 1. Let result be the resulting matrix initialized to the values of the current matrix.
    auto result = CloneAsDOMMatrix();
    // 2. Perform a skewYSelf() transformation on result with the argument sy.
    result->SkewYSelf(sy);
    // 3. Return result.
    return result;
  }

  ExceptionOr<Ref<DOMMatrix>> DOMMatrixReadOnly::Multiply(DOMMatrixInit other) const noexcept
  {
    // 1. Let result be the resulting matrix initialized to the values of the current matrix.
    auto result = CloneAsDOMMatrix();
    // 2. Perform a multiplySelf() transformation on result with the argument other.
    if (auto multiplied = result->MultiplySelf(krys::move(other)); multiplied.HasException())
    {
      return multiplied.ReleaseException();
    }
    // 3. Return result.
    return result;
  }

  Ref<DOMMatrix> DOMMatrixReadOnly::FlipX() const noexcept
  {
    // 1. Let result be the resulting matrix initialized to the values of the current matrix.
    auto result = CloneAsDOMMatrix();
    // 2. Post-multiply result with new DOMMatrix([-1, 0, 0, 1, 0, 0]).
    result->_matrix.FlipX();
    // 3. Return result.
    return result;
  }

  Ref<DOMMatrix> DOMMatrixReadOnly::FlipY() const noexcept
  {
    // 1. Let result be the resulting matrix initialized to the values of the current matrix.
    auto result = CloneAsDOMMatrix();
    // 2. Post-multiply result with new DOMMatrix([1, 0, 0, -1, 0, 0]).
    result->_matrix.FlipY();
    // 3. Return result.
    return result;
  }

  Ref<DOMMatrix> DOMMatrixReadOnly::Inverse() const noexcept
  {
    // 1. Let result be the resulting matrix initialized to the values of the current matrix.
    auto result = CloneAsDOMMatrix();
    // 2. Perform a invertSelf() transformation on result.
    result->InvertSelf();
    // 3. Return result.
    return result;
  }

  Ref<DOMPoint> DOMMatrixReadOnly::TransformPoint(const DOMPointInit &point) const noexcept
  {
    // Let pointObject be the result of invoking create a DOMPoint from the dictionary point. Return the
    // result of invoking transform a point with a matrix, given pointObject and the current matrix. The
    // passed argument does not get modified.
    return TransformPointWithMatrix(point, *this);
  }

  Array<float, 16> DOMMatrixReadOnly::ToFloat32Array() const noexcept
  {
    // Returns the serialized 16 elements m11 to m44 of the current matrix in column-major order.
    const auto array64 = ToFloat64Array();
    Array<float, 16> array32;
    for (size_t i = 0; i < array64.size(); ++i)
    {
      array32[i] = static_cast<float>(array64[i]);
    }
    return array32;
  }

  Array<double, 16> DOMMatrixReadOnly::ToFloat64Array() const noexcept
  {
    // Returns the serialized 16 elements m11 to m44 of the current matrix in column-major order.
    return {_matrix.M11(), _matrix.M12(), _matrix.M13(), _matrix.M14(), _matrix.M21(), _matrix.M22(),
            _matrix.M23(), _matrix.M24(), _matrix.M31(), _matrix.M32(), _matrix.M33(), _matrix.M34(),
            _matrix.M41(), _matrix.M42(), _matrix.M43(), _matrix.M44()};
  }

  ExceptionOr<CSSOMString> DOMMatrixReadOnly::ToString() const noexcept
  {
    using namespace std::string_view_literals;

    // 1. If one or more of m11 element through m44 element are a non-finite value, then throw an
    //    "InvalidStateError" DOMException.
    if (!_matrix.ContainsOnlyFiniteValues())
    {
      return Exception {ExceptionCode::InvalidStateError, u8"Matrix contains non-finite values"};
    }

    // 2. Let string be the empty string.
    StringBuilder string;

    // 3. If is 2D is true, then:
    if (_is2D)
    {
      // 3a.–3m. Append "matrix(", then ! ToString of the m11, m12, m21, m22, m41 and m42 elements separated
      // by
      //         ", ", then ")".
      string.Append(u8"matrix("sv, _matrix.A(), u8", "sv, _matrix.B(), u8", "sv, _matrix.C(), u8", "sv,
                    _matrix.D(), u8", "sv, _matrix.E(), u8", "sv, _matrix.F(), u8')');
    }
    // 4. Otherwise:
    else
    {
      // 4a.–4ag. Append "matrix3d(", then ! ToString of the m11 through m44 elements separated by ", ", then
      // ")".
      string.Append(u8"matrix3d("sv, _matrix.M11(), u8", "sv, _matrix.M12(), u8", "sv, _matrix.M13(),
                    u8", "sv, _matrix.M14(), u8", "sv, _matrix.M21(), u8", "sv, _matrix.M22(), u8", "sv,
                    _matrix.M23(), u8", "sv, _matrix.M24(), u8", "sv, _matrix.M31(), u8", "sv, _matrix.M32(),
                    u8", "sv, _matrix.M33(), u8", "sv, _matrix.M34(), u8", "sv, _matrix.M41(), u8", "sv,
                    _matrix.M42(), u8", "sv, _matrix.M43(), u8", "sv, _matrix.M44(), u8')');
    }

    // 5. Return string.
    return krys::move(string).ToString();
  }

#pragma endregion

  ExceptionOr<void> DOMMatrixReadOnly::ValidateAndFixup(DOMMatrix2DInit &dict) noexcept
  {
    // 1. If if at least one of the following conditions are true for dict, then throw a TypeError exception
    // and
    //    abort these steps.
    //    - a and m11 are both present and SameValueZero(a, m11) is false.
    if (dict.A && dict.M11 && !SameValueZero(*dict.A, *dict.M11))
    {
      return Exception {ExceptionCode::TypeError, u8"init.a and init.m11 do not match"};
    }
    // - b and m12 are both present and SameValueZero(b, m12) is false.
    if (dict.B && dict.M12 && !SameValueZero(*dict.B, *dict.M12))
    {
      return Exception {ExceptionCode::TypeError, u8"init.b and init.m12 do not match"};
    }
    // - c and m21 are both present and SameValueZero(c, m21) is false.
    if (dict.C && dict.M21 && !SameValueZero(*dict.C, *dict.M21))
    {
      return Exception {ExceptionCode::TypeError, u8"init.c and init.m21 do not match"};
    }
    // - d and m22 are both present and SameValueZero(d, m22) is false.
    if (dict.D && dict.M22 && !SameValueZero(*dict.D, *dict.M22))
    {
      return Exception {ExceptionCode::TypeError, u8"init.d and init.m22 do not match"};
    }
    // - e and m41 are both present and SameValueZero(e, m41) is false.
    if (dict.E && dict.M41 && !SameValueZero(*dict.E, *dict.M41))
    {
      return Exception {ExceptionCode::TypeError, u8"init.e and init.m41 do not match"};
    }
    // - f and m42 are both present and SameValueZero(f, m42) is false.
    if (dict.F && dict.M42 && !SameValueZero(*dict.F, *dict.M42))
    {
      return Exception {ExceptionCode::TypeError, u8"init.f and init.m42 do not match"};
    }

    // 2. If m11 is not present then set it to the value of member a, or value 1 if a is also not present.
    if (!dict.M11)
    {
      dict.M11 = dict.A.value_or(1);
    }
    // 3. If m12 is not present then set it to the value of member b, or value 0 if b is also not present.
    if (!dict.M12)
    {
      dict.M12 = dict.B.value_or(0);
    }
    // 4. If m21 is not present then set it to the value of member c, or value 0 if c is also not present.
    if (!dict.M21)
    {
      dict.M21 = dict.C.value_or(0);
    }
    // 5. If m22 is not present then set it to the value of member d, or value 1 if d is also not present.
    if (!dict.M22)
    {
      dict.M22 = dict.D.value_or(1);
    }
    // 6. If m41 is not present then set it to the value of member e, or value 0 if e is also not present.
    if (!dict.M41)
    {
      dict.M41 = dict.E.value_or(0);
    }
    // 7. If m42 is not present then set it to the value of member f, or value 0 if f is also not present.
    if (!dict.M42)
    {
      dict.M42 = dict.F.value_or(0);
    }

    return {};
  }

  ExceptionOr<void> DOMMatrixReadOnly::ValidateAndFixup(DOMMatrixInit &dict) noexcept
  {
    // 1. Validate and fixup (2D) dict.
    if (auto result = ValidateAndFixup(static_cast<DOMMatrix2DInit &>(dict)); result.HasException())
    {
      return result.ReleaseException();
    }

    // The 3D members are always present, as they have defaults. A value other than 0 or -0 is true, NaN
    // included.
    const bool has3DValues = dict.M13 || dict.M14 || dict.M23 || dict.M24 || dict.M31 || dict.M32 || dict.M34
                             || dict.M43 || dict.M33 != 1 || dict.M44 != 1;

    // 2. If is2D is true and: at least one of m13, m14, m23, m24, m31, m32, m34, m43 are present with a value
    //    other than 0 or -0, or at least one of m33, m44 are present with a value other than 1, then throw a
    //    TypeError exception and abort these steps.
    if (dict.Is2D.value_or(false) && has3DValues)
    {
      return Exception {ExceptionCode::TypeError, u8"A 2D matrix can't have 3D values"};
    }

    // 3. If is2D is not present and at least one of m13, m14, m23, m24, m31, m32, m34, m43 are present with a
    //    value other than 0 or -0, or at least one of m33, m44 are present with a value other than 1, set
    //    is2D to false.
    if (!dict.Is2D && has3DValues)
    {
      dict.Is2D = false;
    }

    // 4. If is2D is still not present, set it to true.
    if (!dict.Is2D)
    {
      dict.Is2D = true;
    }

    return {};
  }

  Ref<DOMMatrix> DOMMatrixReadOnly::CloneAsDOMMatrix() const noexcept
  {
    return DOMMatrix::Create(_matrix, _is2D ? Is2D::Yes : Is2D::No);
  }

  ExceptionOr<DOMMatrixReadOnly::AbstractMatrix>
    DOMMatrixReadOnly::ParseStringIntoAbstractMatrix(CSSOMString transformList) noexcept
  {
    // 1. If transformList is the empty string, set it to the string "matrix(1, 0, 0, 1, 0, 0)".
    if (transformList.empty())
    {
      transformList = u8"matrix(1, 0, 0, 1, 0, 0)";
    }

    // 2. Parse transformList into parsedValue given the grammar for the CSS transform property. The result
    // will be
    //    a <transform-list>, the keyword none, or failure. If parsedValue is failure, or any
    //    <transform-function> has <length> values without absolute length units, or any keyword other than
    //    none is used, then return failure.
    auto parsedValue = css::parse_property::ParseTransformRaw(
      transformList, css::ParserContext(css::ParserMode::HTMLStandard));
    if (!parsedValue)
    {
      return Exception {ExceptionCode::SyntaxError};
    }
    if (auto [isWidthDependent, isHeightDependent] = parsedValue->ComputeSizeDependencies();
        isWidthDependent || isHeightDependent)
    {
      return Exception {ExceptionCode::SyntaxError};
    }

    // 3. If parsedValue is none, set parsedValue to a <transform-list> containing a single identity matrix.
    if (parsedValue->IsNone())
    {
      return AbstractMatrix {};
    }

    // 4. Let 2dTransform track the 2D/3D dimension status of parsedValue. If parsedValue consists of any
    //    three-dimensional transform functions, set 2dTransform to false. Otherwise, set 2dTransform to true.
    // 5. Transform all <transform-function>s to 4x4 abstract matrices by following the "Mathematical
    // Description of
    //    Transform Functions".
    // 6. Let matrix be a 4x4 abstract matrix as shown in the initial figure of this section. Post-multiply
    // all
    //    matrices from left to right and set matrix to this product.
    AbstractMatrix result;
    for (const auto &function : *parsedValue)
    {
      function->Apply(result.Matrix, {0, 0});
      if (function->Is3DOperation())
      {
        result.Is2D = false;
      }
    }

    // 7. Return matrix and 2dTransform.
    return result;
  }

  template <typename T>
  ExceptionOr<Ref<T>> DOMMatrixReadOnly::CreateHelper(Maybe<Variant<CSSOMString, List<double>>> init) noexcept
  {
    // If init is omitted: Return the result of invoking create a 2d matrix of type DOMMatrixReadOnly or
    // DOMMatrix as appropriate, with the sequence [1, 0, 0, 1, 0, 0].
    if (!init)
    {
      return T::Create(TransformationMatrix {}, Is2D::Yes);
    }

    return SwitchOn(
      *init,
      [](const CSSOMString &string) -> ExceptionOr<Ref<T>>
      {
        // If init is a DOMString:
        // 1. (Skipped: Booey has no workers, so the current global object is always a Window.)
        // 2. Parse init into an abstract matrix, and let matrix and 2dTransform be the result. If the result
        // is
        //    failure, then throw a "SyntaxError" DOMException.
        auto parsed = ParseStringIntoAbstractMatrix(string);
        if (parsed.HasException())
        {
          return parsed.ReleaseException();
        }

        // 3. If 2dTransform is true, return the result of invoking create a 2d matrix of type
        // DOMMatrixReadOnly or
        //    DOMMatrix as appropriate, with a sequence of numbers, the values being the elements m11, m12,
        //    m21, m22, m41 and m42 of matrix. Otherwise, return the result of invoking create a 3d matrix of
        //    type DOMMatrixReadOnly or DOMMatrix as appropriate, with a sequence of numbers, the values being
        //    the 16 elements of matrix.
        const auto &[matrix, is2D] = parsed.Value();
        return T::Create(matrix, is2D ? Is2D::Yes : Is2D::No);
      },
      [](const List<double> &sequence) -> ExceptionOr<Ref<T>>
      {
        // If init is a sequence with 6 elements: Return the result of invoking create a 2d matrix of type
        // DOMMatrixReadOnly or DOMMatrix as appropriate, with the sequence init.
        // If init is a sequence with 16 elements: Return the result of invoking create a 3d matrix of type
        // DOMMatrixReadOnly or DOMMatrix as appropriate, with the sequence init.
        // Otherwise: Throw a TypeError exception.
        return FromArrayHelper<T, double>(Span<const double>(sequence));
      });
  }

  template ExceptionOr<Ref<DOMMatrixReadOnly>>
    DOMMatrixReadOnly::CreateHelper<DOMMatrixReadOnly>(Maybe<Variant<CSSOMString, List<double>>>) noexcept;
  template ExceptionOr<Ref<DOMMatrix>>
    DOMMatrixReadOnly::CreateHelper<DOMMatrix>(Maybe<Variant<CSSOMString, List<double>>>) noexcept;
}
