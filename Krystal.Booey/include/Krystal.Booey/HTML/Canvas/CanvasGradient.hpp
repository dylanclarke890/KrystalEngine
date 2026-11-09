/*
 * Copyright (C) 2006-2025 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatPoint.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class Gradient;
}

namespace krys::boo::html
{
  /// @brief A gradient a 2D context fills or strokes with.
  /// @see https://html.spec.whatwg.org/#canvasgradient
  class CanvasGradient : public RefCounted<CanvasGradient>
  {
  private:
    const Ref<Gradient> _gradient;

  public:
    KRYS_NODISCARD static Ref<CanvasGradient> Create(const FloatPoint &p0, const FloatPoint &p1) noexcept;
    KRYS_NODISCARD static Ref<CanvasGradient> Create(const FloatPoint &p0, float r0, const FloatPoint &p1,
                                                     float r1) noexcept;
    KRYS_NODISCARD static Ref<CanvasGradient> Create(const FloatPoint &centerPoint,
                                                     float angleInRadians) noexcept;

    ~CanvasGradient() noexcept;

    KRYS_NODISCARD Gradient &GetGradient() noexcept
    {
      return *_gradient;
    }
    KRYS_NODISCARD const Gradient &GetGradient() const noexcept
    {
      return *_gradient;
    }

    /// @see https://html.spec.whatwg.org/#dom-canvasgradient-addcolorstop
    dom::ExceptionOr<void> AddColorStop(double value, dom::DOMStringView color) noexcept;

  private:
    CanvasGradient(const FloatPoint &p0, const FloatPoint &p1) noexcept;
    CanvasGradient(const FloatPoint &p0, float r0, const FloatPoint &p1, float r1) noexcept;
    CanvasGradient(const FloatPoint &centerPoint, float angleInRadians) noexcept;
  };
}
