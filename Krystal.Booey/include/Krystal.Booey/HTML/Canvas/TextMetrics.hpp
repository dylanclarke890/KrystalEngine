/*
 * Copyright (C) 2008-2023 Apple Inc. All rights reserved.
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

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"

namespace krys::boo::html
{
  /// @brief The dimensions of a piece of text measured by a 2D context's `MeasureText`, relative to where it
  /// would be drawn.
  /// @see https://html.spec.whatwg.org/#textmetrics
  class TextMetrics final : public RefCounted<TextMetrics>
  {
  private:
    double _width {0};
    double _actualBoundingBoxLeft {0};
    double _actualBoundingBoxRight {0};
    double _fontBoundingBoxAscent {0};
    double _fontBoundingBoxDescent {0};
    double _actualBoundingBoxAscent {0};
    double _actualBoundingBoxDescent {0};
    double _emHeightAscent {0};
    double _emHeightDescent {0};
    double _hangingBaseline {0};
    double _alphabeticBaseline {0};
    double _ideographicBaseline {0};

  public:
    KRYS_NODISCARD static Ref<TextMetrics> Create() noexcept
    {
      return AdoptRef(*new TextMetrics);
    }

    KRYS_NODISCARD double Width() const noexcept
    {
      return _width;
    }
    void SetWidth(double width) noexcept
    {
      _width = width;
    }

    KRYS_NODISCARD double ActualBoundingBoxLeft() const noexcept
    {
      return _actualBoundingBoxLeft;
    }
    void SetActualBoundingBoxLeft(double value) noexcept
    {
      _actualBoundingBoxLeft = value;
    }

    KRYS_NODISCARD double ActualBoundingBoxRight() const noexcept
    {
      return _actualBoundingBoxRight;
    }
    void SetActualBoundingBoxRight(double value) noexcept
    {
      _actualBoundingBoxRight = value;
    }

    KRYS_NODISCARD double FontBoundingBoxAscent() const noexcept
    {
      return _fontBoundingBoxAscent;
    }
    void SetFontBoundingBoxAscent(double value) noexcept
    {
      _fontBoundingBoxAscent = value;
    }

    KRYS_NODISCARD double FontBoundingBoxDescent() const noexcept
    {
      return _fontBoundingBoxDescent;
    }
    void SetFontBoundingBoxDescent(double value) noexcept
    {
      _fontBoundingBoxDescent = value;
    }

    KRYS_NODISCARD double ActualBoundingBoxAscent() const noexcept
    {
      return _actualBoundingBoxAscent;
    }
    void SetActualBoundingBoxAscent(double value) noexcept
    {
      _actualBoundingBoxAscent = value;
    }

    KRYS_NODISCARD double ActualBoundingBoxDescent() const noexcept
    {
      return _actualBoundingBoxDescent;
    }
    void SetActualBoundingBoxDescent(double value) noexcept
    {
      _actualBoundingBoxDescent = value;
    }

    KRYS_NODISCARD double EmHeightAscent() const noexcept
    {
      return _emHeightAscent;
    }
    void SetEmHeightAscent(double value) noexcept
    {
      _emHeightAscent = value;
    }

    KRYS_NODISCARD double EmHeightDescent() const noexcept
    {
      return _emHeightDescent;
    }
    void SetEmHeightDescent(double value) noexcept
    {
      _emHeightDescent = value;
    }

    KRYS_NODISCARD double HangingBaseline() const noexcept
    {
      return _hangingBaseline;
    }
    void SetHangingBaseline(double value) noexcept
    {
      _hangingBaseline = value;
    }

    KRYS_NODISCARD double AlphabeticBaseline() const noexcept
    {
      return _alphabeticBaseline;
    }
    void SetAlphabeticBaseline(double value) noexcept
    {
      _alphabeticBaseline = value;
    }

    KRYS_NODISCARD double IdeographicBaseline() const noexcept
    {
      return _ideographicBaseline;
    }
    void SetIdeographicBaseline(double value) noexcept
    {
      _ideographicBaseline = value;
    }

  private:
    TextMetrics() noexcept = default;
  };
}
