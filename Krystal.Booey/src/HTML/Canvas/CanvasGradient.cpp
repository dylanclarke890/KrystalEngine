/*
 * Copyright (C) 2006-2024 Apple Inc. All rights reserved.
 * Copyright (C) 2007 Alp Toker <alp@atoker.com>
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

#include "Krystal.Booey/HTML/Canvas/CanvasGradient.hpp"
#include "Krystal.Booey/Graphics/Gradient.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasStyle.hpp"
#include <cmath>

namespace krys::boo::html
{
  CanvasGradient::CanvasGradient(const FloatPoint &p0, const FloatPoint &p1) noexcept
      : _gradient(Gradient::Create(Gradient::LinearData {p0, p1}, {ColourInterpolationMethod::SRGB {},
                                                                   AlphaPremultiplication::Unpremultiplied}))
  {
  }

  CanvasGradient::CanvasGradient(const FloatPoint &p0, float r0, const FloatPoint &p1, float r1) noexcept
      : _gradient(
          Gradient::Create(Gradient::RadialData {p0, p1, r0, r1, 1},
                           {ColourInterpolationMethod::SRGB {}, AlphaPremultiplication::Unpremultiplied}))
  {
  }

  CanvasGradient::CanvasGradient(const FloatPoint &centerPoint, float angleInRadians) noexcept
      : _gradient(
          Gradient::Create(Gradient::ConicData {centerPoint, angleInRadians},
                           {ColourInterpolationMethod::SRGB {}, AlphaPremultiplication::Unpremultiplied}))
  {
  }

  Ref<CanvasGradient> CanvasGradient::Create(const FloatPoint &p0, const FloatPoint &p1) noexcept
  {
    return AdoptRef(*new CanvasGradient(p0, p1));
  }

  Ref<CanvasGradient> CanvasGradient::Create(const FloatPoint &p0, float r0, const FloatPoint &p1,
                                             float r1) noexcept
  {
    return AdoptRef(*new CanvasGradient(p0, r0, p1, r1));
  }

  Ref<CanvasGradient> CanvasGradient::Create(const FloatPoint &centerPoint, float angleInRadians) noexcept
  {
    return AdoptRef(*new CanvasGradient(centerPoint, angleInRadians));
  }

  CanvasGradient::~CanvasGradient() noexcept = default;

  // https://html.spec.whatwg.org/#dom-canvasgradient-addcolorstop
  dom::ExceptionOr<void> CanvasGradient::AddColorStop(double value, dom::DOMStringView colorString) noexcept
  {
    // (The offset is a double, so a non-finite one is a TypeError.)
    if (!std::isfinite(value))
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    // 1. If the offset is less than 0 or greater than 1, then throw an "IndexSizeError" DOMException.
    if (value < 0 || value > 1)
    {
      return dom::Exception {dom::ExceptionCode::IndexSizeError};
    }

    // 2. Let parsed color be the result of parsing color.
    // 3. If parsed color is failure, throw a "SyntaxError" DOMException.
    auto color = ParseColor(colorString);
    if (!color.IsValid())
    {
      return dom::Exception {dom::ExceptionCode::SyntaxError};
    }

    // 4. Place a new stop on the gradient, at offset offset relative to the whole gradient, and whose color
    // is
    //    parsed color.
    _gradient->AddColorStop({static_cast<float>(value), krys::move(color)});
    return {};
  }
}
