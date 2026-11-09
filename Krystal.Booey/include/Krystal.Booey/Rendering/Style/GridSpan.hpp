/**
 * Copyright (C) 2025 Apple Inc. All rights reserved.
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
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  /// @brief A span in a single direction (either rows or columns). Note that `StartLine` and `EndLine` are
  /// grid lines' indexes.
  /// @note Despite line numbers in the spec start in "1", the indexes here start in "0".
  class GridSpan
  {
  private:
    enum GridSpanType : uint8
    {
      UntranslatedDefinite,
      TranslatedDefinite,
      Indefinite
    };

    int _startLine;
    int _endLine;
    GridSpanType _type;

  public:
    struct GridSpanIterator
    {
      unsigned Value;

      GridSpanIterator(unsigned value) noexcept : Value(value)
      {
      }

      operator unsigned &() noexcept
      {
        return Value;
      }

      KRYS_NODISCARD unsigned operator*() const noexcept
      {
        return Value;
      }
    };

    KRYS_NODISCARD static GridSpan UntranslatedDefiniteGridSpan(int startLine, int endLine) noexcept;
    KRYS_NODISCARD static GridSpan TranslatedDefiniteGridSpan(unsigned startLine, unsigned endLine) noexcept;
    KRYS_NODISCARD static GridSpan IndefiniteGridSpan() noexcept;

    KRYS_NODISCARD static GridSpan MasonryAxisTranslatedDefiniteGridSpan() noexcept;

    KRYS_NODISCARD friend bool operator==(const GridSpan &, const GridSpan &) noexcept = default;

    KRYS_NODISCARD unsigned IntegerSpan() const noexcept;

    KRYS_NODISCARD int UntranslatedStartLine() const noexcept;
    KRYS_NODISCARD int UntranslatedEndLine() const noexcept;

    KRYS_NODISCARD unsigned StartLine() const noexcept;
    KRYS_NODISCARD unsigned EndLine() const noexcept;

    KRYS_NODISCARD GridSpanIterator begin() const noexcept;
    KRYS_NODISCARD GridSpanIterator end() const noexcept;

    KRYS_NODISCARD bool IsTranslatedDefinite() const noexcept;
    KRYS_NODISCARD bool IsIndefinite() const noexcept;

    void Translate(unsigned offset) noexcept;

    /// @brief Moves this span to be in the same coordinate space as `parent`.
    /// @param reverse Swaps the direction to handle RTL/LTR changes.
    void TranslateTo(const GridSpan &parent, bool reverse) noexcept;

    void Clamp(int max) noexcept;

    bool Clamp(int min, int max) noexcept;

  private:
    GridSpan(int startLine, int endLine, GridSpanType type) noexcept;
  };
}
