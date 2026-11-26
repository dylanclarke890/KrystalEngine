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

#include "Krystal.Booey/Css/Geometry/DOMMatrix.hpp"
#include "Krystal.Core/Move.hpp"
#include <limits>

namespace krys::boo
{
  DOMMatrix::DOMMatrix(const TransformationMatrix &matrix, Is2D is2D) noexcept
      : DOMMatrixReadOnly(matrix, is2D)
  {
  }

#pragma region DOMMatrix - https://drafts.csswg.org/geometry-1/#dommatrix

  ExceptionOr<Ref<DOMMatrix>> DOMMatrix::Create(Maybe<Variant<CSSOMString, List<double>>> init) noexcept
  {
    return CreateHelper<DOMMatrix>(krys::move(init));
  }

  ExceptionOr<Ref<DOMMatrix>> DOMMatrix::FromMatrix(DOMMatrixInit other) noexcept
  {
    return FromMatrixHelper<DOMMatrix>(krys::move(other));
  }

  ExceptionOr<Ref<DOMMatrix>> DOMMatrix::FromFloat32Array(Span<const float> array32) noexcept
  {
    return FromArrayHelper<DOMMatrix>(array32);
  }

  ExceptionOr<Ref<DOMMatrix>> DOMMatrix::FromFloat64Array(Span<const double> array64) noexcept
  {
    return FromArrayHelper<DOMMatrix>(array64);
  }

  ExceptionOr<Ref<DOMMatrix>> DOMMatrix::MultiplySelf(DOMMatrixInit other) noexcept
  {
    // 1. Let otherObject be the result of invoking create a DOMMatrix from the dictionary other.
    auto otherObject = FromMatrix(krys::move(other));
    if (otherObject.HasException())
    {
      return otherObject.ReleaseException();
    }

    // 2. The otherObject matrix gets post-multiplied to the current matrix.
    _matrix.Multiply(otherObject.Value()->_matrix);

    // 3. If is 2D of otherObject is false, set is 2D of the current matrix to false.
    if (!otherObject.Value()->_is2D)
    {
      _is2D = false;
    }

    // 4. Return the current matrix.
    return ShareRef(*this);
  }

  ExceptionOr<Ref<DOMMatrix>> DOMMatrix::PreMultiplySelf(DOMMatrixInit other) noexcept
  {
    // 1. Let otherObject be the result of invoking create a DOMMatrix from the dictionary other.
    auto otherObject = FromMatrix(krys::move(other));
    if (otherObject.HasException())
    {
      return otherObject.ReleaseException();
    }

    // 2. The otherObject matrix gets pre-multiplied to the current matrix.
    _matrix = otherObject.Value()->_matrix * _matrix;

    // 3. If is 2D of otherObject is false, set is 2D of the current matrix to false.
    if (!otherObject.Value()->_is2D)
    {
      _is2D = false;
    }

    // 4. Return the current matrix.
    return ShareRef(*this);
  }

  Ref<DOMMatrix> DOMMatrix::TranslateSelf(double tx, double ty, double tz) noexcept
  {
    // 1. Post-multiply a translation transformation on the current matrix.
    _matrix.Translate3d(tx, ty, tz);

    // 2. If tz is specified and not 0 or -0, set is 2D of the current matrix to false.
    if (tz != 0)
    {
      _is2D = false;
    }

    // 3. Return the current matrix.
    return ShareRef(*this);
  }

  Ref<DOMMatrix> DOMMatrix::ScaleSelf(double scaleX, Maybe<double> scaleY, double scaleZ, double originX,
                                      double originY, double originZ) noexcept
  {
    // 1. Perform a translateSelf() transformation on the current matrix with the arguments originX, originY,
    //    originZ.
    TranslateSelf(originX, originY, originZ);

    // 2. If scaleY is missing, set scaleY to the value of scaleX.
    if (!scaleY)
    {
      scaleY = scaleX;
    }

    // 3. Post-multiply a non-uniform scale transformation on the current matrix. The 3D scale matrix is
    // described
    //    in CSS Transforms with sx = scaleX, sy = scaleY and sz = scaleZ.
    _matrix.Scale3d(scaleX, *scaleY, scaleZ);

    // 4. Negate originX, originY and originZ.
    // 5. Perform a translateSelf() transformation on the current matrix with the arguments originX, originY,
    //    originZ.
    TranslateSelf(-originX, -originY, -originZ);

    // 6. If scaleZ is not 1, set is 2D of the current matrix to false.
    if (scaleZ != 1)
    {
      _is2D = false;
    }

    // 7. Return the current matrix.
    return ShareRef(*this);
  }

  Ref<DOMMatrix> DOMMatrix::Scale3dSelf(double scale, double originX, double originY, double originZ) noexcept
  {
    // 1. Apply a translateSelf() transformation to the current matrix with the arguments originX, originY,
    //    originZ.
    TranslateSelf(originX, originY, originZ);

    // 2. Post-multiply a uniform 3D scale transformation (m11 = m22 = m33 = scale) on the current matrix. The
    // 3D
    //    scale matrix is described in CSS Transforms with sx = sy = sz = scale.
    _matrix.Scale3d(scale, scale, scale);

    // 3. Apply a translateSelf() transformation to the current matrix with the arguments -originX, -originY,
    //    -originZ.
    TranslateSelf(-originX, -originY, -originZ);

    // 4. If scale is not 1, set is 2D of the current matrix to false.
    if (scale != 1)
    {
      _is2D = false;
    }

    // 5. Return the current matrix.
    return ShareRef(*this);
  }

  Ref<DOMMatrix> DOMMatrix::RotateSelf(double rotX, Maybe<double> rotY, Maybe<double> rotZ) noexcept
  {
    // 1. If rotY and rotZ are both missing, set rotZ to the value of rotX and set rotX and rotY to 0.
    if (!rotY && !rotZ)
    {
      rotZ = rotX;
      rotX = 0;
      rotY = 0;
    }

    // 2. If rotY is still missing, set rotY to 0.
    if (!rotY)
    {
      rotY = 0;
    }

    // 3. If rotZ is still missing, set rotZ to 0.
    if (!rotZ)
    {
      rotZ = 0;
    }

    // 4. If rotX or rotY are not 0 or -0, set is 2D of the current matrix to false.
    if (rotX != 0 || *rotY != 0)
    {
      _is2D = false;
    }

    // 5. Post-multiply a rotation transformation on the current matrix around the vector 0, 0, 1 by the
    // specified
    //    rotation rotZ in degrees.
    // 6. Post-multiply a rotation transformation on the current matrix around the vector 0, 1, 0 by the
    // specified
    //    rotation rotY in degrees.
    // 7. Post-multiply a rotation transformation on the current matrix around the vector 1, 0, 0 by the
    // specified
    //    rotation rotX in degrees.
    _matrix.Rotate3d(rotX, *rotY, *rotZ);

    // 8. Return the current matrix.
    return ShareRef(*this);
  }

  Ref<DOMMatrix> DOMMatrix::RotateFromVectorSelf(double x, double y) noexcept
  {
    // 1. Post-multiply a rotation transformation on the current matrix. The rotation angle is determined by
    // the
    //    angle between the vector (1,0)T and (x,y)T in the clockwise direction. If x and y should both be 0
    //    or -0, the angle is specified as 0.
    _matrix.RotateFromVector(x, y);

    // 2. Return the current matrix.
    return ShareRef(*this);
  }

  Ref<DOMMatrix> DOMMatrix::RotateAxisAngleSelf(double x, double y, double z, double angle) noexcept
  {
    // 1. Post-multiply a rotation transformation on the current matrix around the specified vector x, y, z by
    // the
    //    specified rotation angle in degrees.
    _matrix.Rotate3d(x, y, z, angle);

    // 2. If x or y are not 0 or -0, set is 2D of the current matrix to false.
    if (x != 0 || y != 0)
    {
      _is2D = false;
    }

    // 3. Return the current matrix.
    return ShareRef(*this);
  }

  Ref<DOMMatrix> DOMMatrix::SkewXSelf(double sx) noexcept
  {
    // 1. Post-multiply a skewX transformation on the current matrix by the specified angle sx in degrees.
    _matrix.SkewX(sx);

    // 2. Return the current matrix.
    return ShareRef(*this);
  }

  Ref<DOMMatrix> DOMMatrix::SkewYSelf(double sy) noexcept
  {
    // 1. Post-multiply a skewX transformation on the current matrix by the specified angle sy in degrees.
    _matrix.SkewY(sy);

    // 2. Return the current matrix.
    return ShareRef(*this);
  }

  Ref<DOMMatrix> DOMMatrix::InvertSelf() noexcept
  {
    // 1. Invert the current matrix.
    if (auto inverse = _matrix.Inverse())
    {
      _matrix = *inverse;
    }
    // 2. If the current matrix is not invertible set all attributes to NaN and set is 2D to false.
    else
    {
      constexpr auto nan = std::numeric_limits<double>::quiet_NaN();
      _matrix.SetMatrix(nan, nan, nan, nan, nan, nan, nan, nan, nan, nan, nan, nan, nan, nan, nan, nan);
      _is2D = false;
    }

    // 3. Return the current matrix.
    return ShareRef(*this);
  }

  ExceptionOr<Ref<DOMMatrix>> DOMMatrix::SetMatrixValue(CSSOMString transformList) noexcept
  {
    // 1. Parse transformList into an abstract matrix, and let matrix and 2dTransform be the result. If the
    // result
    //    is failure, then throw a "SyntaxError" DOMException.
    auto parsed = ParseStringIntoAbstractMatrix(krys::move(transformList));
    if (parsed.HasException())
    {
      return parsed.ReleaseException();
    }

    // 2. Set is 2D to the value of 2dTransform.
    _is2D = parsed.Value().Is2D;

    // 3. Set m11 element through m44 element to the element values of matrix in column-major order.
    _matrix = parsed.Value().Matrix;

    // 4. Return the current matrix.
    return ShareRef(*this);
  }

#pragma endregion
}
