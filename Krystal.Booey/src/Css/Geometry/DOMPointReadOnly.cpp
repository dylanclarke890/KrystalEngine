/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Css/Geometry/DOMPointReadOnly.hpp"
#include "Krystal.Booey/Css/Geometry/DOMMatrix.hpp"
#include "Krystal.Booey/Css/Geometry/DOMPoint.hpp"
#include "Krystal.Core/Move.hpp"

namespace krys::boo
{
#pragma region DOMPointReadOnly - https://drafts.csswg.org/geometry-1/#dompointreadonly

  Ref<DOMPointReadOnly> DOMPointReadOnly::Create(double x, double y, double z, double w) noexcept
  {
    // 1. Let point be a new DOMPointReadOnly or DOMPoint object as appropriate.
    // 2. Set point's variables x coordinate to x, y coordinate to y, z coordinate to z and w perspective to
    // w.
    // 3. Return point.
    return AdoptRef(*new DOMPointReadOnly(x, y, z, w));
  }

  Ref<DOMPointReadOnly> DOMPointReadOnly::FromPoint(const DOMPointInit &other) noexcept
  {
    // https://drafts.csswg.org/geometry-1/#create-a-dompointreadonly-from-the-dictionary
    // 1. Let point be a new DOMPointReadOnly or DOMPoint as appropriate.
    // 2. Set point's variables x coordinate to other's x dictionary member, y coordinate to other's y
    // dictionary
    //    member, z coordinate to other's z dictionary member and w perspective to other's w dictionary
    //    member.
    // 3. Return point.
    return Create(other.X, other.Y, other.Z, other.W);
  }

  ExceptionOr<Ref<DOMPoint>> DOMPointReadOnly::MatrixTransform(DOMMatrixInit matrix) const noexcept
  {
    // 1. Let matrixObject be the result of invoking create a DOMMatrix from the dictionary matrix.
    auto matrixObject = DOMMatrix::FromMatrix(krys::move(matrix));
    if (matrixObject.HasException())
    {
      return matrixObject.ReleaseException();
    }

    // 2. Return the result of invoking transform a point with a matrix, given the current point and
    // matrixObject.
    //    The current point does not get modified.
    return TransformPointWithMatrix({_x, _y, _z, _w}, *matrixObject.ReleaseValue());
  }

#pragma endregion

#pragma region DOMPoint - https://drafts.csswg.org/geometry-1/#dompoint

  Ref<DOMPoint> DOMPoint::FromPoint(const DOMPointInit &other) noexcept
  {
    // https://drafts.csswg.org/geometry-1/#create-a-dompointreadonly-from-the-dictionary
    // 1. Let point be a new DOMPointReadOnly or DOMPoint as appropriate.
    // 2. Set point's variables x coordinate to other's x dictionary member, y coordinate to other's y
    // dictionary
    //    member, z coordinate to other's z dictionary member and w perspective to other's w dictionary
    //    member.
    // 3. Return point.
    return Create(other.X, other.Y, other.Z, other.W);
  }

#pragma endregion

  Ref<DOMPoint> TransformPointWithMatrix(const DOMPointInit &point, const DOMMatrixReadOnly &matrix) noexcept
  {
    // 1. Let x be point's x coordinate.
    double x = point.X;
    // 2. Let y be point's y coordinate.
    double y = point.Y;
    // 3. Let z be point's z coordinate.
    double z = point.Z;
    // 4. Let w be point's w perspective.
    double w = point.W;

    // 5. Let pointVector be a new column vector with the elements being x, y, z, and w, respectively.
    // 6. Set pointVector to pointVector pre-multiplied by matrix.
    matrix.GetTransformationMatrix().Map4ComponentPoint(x, y, z, w);

    // 7. Let transformedPoint be a new DOMPoint object.
    // 8. Set transformedPoint's x coordinate to pointVector's first element.
    // 9. Set transformedPoint's y coordinate to pointVector's second element.
    // 10. Set transformedPoint's z coordinate to pointVector's third element.
    // 11. Set transformedPoint's w perspective to pointVector's fourth element.
    // 12. Return transformedPoint.
    return DOMPoint::Create(x, y, z, w);
  }
}
