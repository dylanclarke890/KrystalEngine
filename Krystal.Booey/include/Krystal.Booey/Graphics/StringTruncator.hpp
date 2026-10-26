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
