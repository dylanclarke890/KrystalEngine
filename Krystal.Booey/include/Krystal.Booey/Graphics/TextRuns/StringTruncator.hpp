/*
 * Copyright (C) 2005, 2006, 2007 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  class FontCascade;

  /// @brief Shortens strings to fit a width in a font, cutting whole grapheme clusters and marking the cut
  /// with an ellipsis.
  class StringTruncator
  {
  public:
    /// @brief @p string with its middle cut to fit @p maxWidth.
    KRYS_NODISCARD static utf8_string CenterTruncate(utf8_stringview string, float maxWidth,
                                                     const FontCascade &font) noexcept;
    /// @brief @p string with its end cut to fit @p maxWidth.
    KRYS_NODISCARD static utf8_string RightTruncate(utf8_stringview string, float maxWidth,
                                                    const FontCascade &font) noexcept;
    /// @param resultWidth Set to the width of the result.
    /// @param shouldInsertEllipsis Whether an ellipsis marks the cut.
    /// @param customTruncationElementWidth Without an ellipsis, the width kept free for what the caller
    /// draws at the cut instead.
    KRYS_NODISCARD static utf8_string CenterTruncate(utf8_stringview string, float maxWidth,
                                                     const FontCascade &font, float &resultWidth,
                                                     bool shouldInsertEllipsis = true,
                                                     float customTruncationElementWidth = 0) noexcept;
    KRYS_NODISCARD static utf8_string RightTruncate(utf8_stringview string, float maxWidth,
                                                    const FontCascade &font, float &resultWidth,
                                                    bool shouldInsertEllipsis = true,
                                                    float customTruncationElementWidth = 0) noexcept;
    KRYS_NODISCARD static utf8_string LeftTruncate(utf8_stringview string, float maxWidth,
                                                   const FontCascade &font, float &resultWidth,
                                                   bool shouldInsertEllipsis = true,
                                                   float customTruncationElementWidth = 0) noexcept;
    /// @brief @p string with its end cut to fit @p maxWidth, without an ellipsis.
    KRYS_NODISCARD static utf8_string RightClipToCharacter(utf8_stringview string, float maxWidth,
                                                           const FontCascade &font, float &resultWidth,
                                                           bool shouldInsertEllipsis = true,
                                                           float customTruncationElementWidth = 0) noexcept;
    /// @brief @p string with its end cut at a word boundary to fit @p maxWidth, without an ellipsis.
    /// @param alwaysTruncate Whether the custom truncation element's width counts even when @p string fits.
    KRYS_NODISCARD static utf8_string RightClipToWord(utf8_stringview string, float maxWidth,
                                                      const FontCascade &font, float &resultWidth,
                                                      bool shouldInsertEllipsis = true,
                                                      float customTruncationElementWidth = 0,
                                                      bool alwaysTruncate = false) noexcept;
    /// @brief The width of @p string in @p font.
    KRYS_NODISCARD static float Width(utf8_stringview string, const FontCascade &font) noexcept;
  };
}
