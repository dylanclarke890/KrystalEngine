/*
 * Copyright (C) 2006, 2008, 2017 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/HTML/Canvas/CanvasPattern.hpp"
#include "Krystal.Booey/CSS/DOMMatrix/DOMMatrixReadOnly.hpp"
#include "Krystal.Booey/Graphics/Images/NativeImage.hpp"
#include "Krystal.Booey/Graphics/Images/SourceImage.hpp"
#include "Krystal.Booey/Graphics/Pattern.hpp"
#include <cmath>

namespace krys::boo::html
{
  Ref<CanvasPattern> CanvasPattern::Create(SourceImage &&image, bool repeatX, bool repeatY) noexcept
  {
    return AdoptRef(*new CanvasPattern(krys::move(image), repeatX, repeatY));
  }

  CanvasPattern::CanvasPattern(SourceImage &&image, bool repeatX, bool repeatY) noexcept
      : _pattern(Pattern::Create(krys::move(image), {repeatX, repeatY}))
  {
  }

  CanvasPattern::~CanvasPattern() noexcept = default;

  bool CanvasPattern::ParseRepetitionType(dom::DOMStringView type, bool &repeatX, bool &repeatY) noexcept
  {
    if (type.empty() || type == u8"repeat")
    {
      repeatX = true;
      repeatY = true;
      return true;
    }
    if (type == u8"no-repeat")
    {
      repeatX = false;
      repeatY = false;
      return true;
    }
    if (type == u8"repeat-x")
    {
      repeatX = true;
      repeatY = false;
      return true;
    }
    if (type == u8"repeat-y")
    {
      repeatX = false;
      repeatY = true;
      return true;
    }
    return false;
  }

  // https://html.spec.whatwg.org/#dom-canvaspattern-settransform
  dom::ExceptionOr<void> CanvasPattern::SetTransform(css::DOMMatrix2DInit &&matrixInit) noexcept
  {
    // 1. Let matrix be the result of creating a DOMMatrix from the 2D dictionary transform.
    auto checkValid = css::DOMMatrixReadOnly::ValidateAndFixup(matrixInit);
    if (checkValid.HasException())
    {
      return checkValid.ReleaseException();
    }

    // 2. If one or more of matrix's m11 element, m12 element, m21 element, m22 element, m41 element, or m42
    //    element are infinite or NaN, then return.
    for (auto value : {*matrixInit.M11, *matrixInit.M12, *matrixInit.M21, *matrixInit.M22, *matrixInit.M41,
                       *matrixInit.M42})
    {
      if (!std::isfinite(value))
      {
        return {};
      }
    }

    // 3. Reset the pattern's transformation matrix to matrix.
    _pattern->SetPatternSpaceTransform(
      {*matrixInit.M11, *matrixInit.M12, *matrixInit.M21, *matrixInit.M22, *matrixInit.M41, *matrixInit.M42});
    return {};
  }
}
