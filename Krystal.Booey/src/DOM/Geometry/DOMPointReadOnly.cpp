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

#include "Krystal.Booey/DOM/Geometry/DOMPointReadOnly.hpp"
#include "Krystal.Booey/CSS/DOMMatrix/DOMMatrixReadOnly.hpp"
#include "Krystal.Booey/DOM/Geometry/DOMPoint.hpp"

namespace krys::boo::dom
{
  ExceptionOr<Ref<DOMPoint>> DOMPointReadOnly::MatrixTransform(css::DOMMatrixInit &&matrixInit) const noexcept
  {
    auto matrixOrException = css::DOMMatrixReadOnly::FromMatrix(krys::move(matrixInit));
    if (matrixOrException.HasException())
    {
      return matrixOrException.ReleaseException();
    }

    auto matrix = matrixOrException.ReleaseValue();
    double x = X();
    double y = Y();
    double z = Z();
    double w = W();
    matrix->GetTransformationMatrix().Map4ComponentPoint(x, y, z, w);
    return DOMPoint::Create(x, y, z, w);
  }
}
