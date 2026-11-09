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

#pragma once

#include "Krystal.Booey/CSS/DOMMatrix/DOMMatrix2DInit.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class Pattern;
  class SourceImage;
}

namespace krys::boo::html
{
  /// @brief An image a 2D context fills or strokes with, repeated.
  /// @see https://html.spec.whatwg.org/#canvaspattern
  class CanvasPattern : public RefCounted<CanvasPattern>
  {
  private:
    const Ref<Pattern> _pattern;

  public:
    KRYS_NODISCARD static Ref<CanvasPattern> Create(SourceImage &&, bool repeatX, bool repeatY) noexcept;

    ~CanvasPattern() noexcept;

    /// @brief Reads @p type as a pattern's repetition (`repeat`, `repeat-x`, `repeat-y` or `no-repeat`; empty
    /// is `repeat`).
    /// @return False if @p type isn't one.
    KRYS_NODISCARD static bool ParseRepetitionType(dom::DOMStringView type, bool &repeatX,
                                                   bool &repeatY) noexcept;

    KRYS_NODISCARD Pattern &GetPattern() noexcept
    {
      return *_pattern;
    }
    KRYS_NODISCARD const Pattern &GetPattern() const noexcept
    {
      return *_pattern;
    }

    /// @see https://html.spec.whatwg.org/#dom-canvaspattern-settransform
    dom::ExceptionOr<void> SetTransform(css::DOMMatrix2DInit && = {}) noexcept;

  private:
    CanvasPattern(SourceImage &&, bool repeatX, bool repeatY) noexcept;
  };
}
