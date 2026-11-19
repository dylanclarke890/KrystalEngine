#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::text
{
  /// @brief An embedding level of the Unicode Bidirectional Algorithm (UAX #9): even levels are left to right
  /// and odd ones right to left.
  using BidiLevel = uint8;

  constexpr BidiLevel LeftToRightBidiLevel = 0;
  constexpr BidiLevel RightToLeftBidiLevel = 1;

  /// @brief The deepest level explicit embeddings and overrides reach.
  constexpr BidiLevel MaxExplicitBidiLevel = 125;

  /// @brief A paragraph level its first strong character chooses, left to right if it has none; also marks a
  /// level that hasn't been resolved yet.
  constexpr BidiLevel DefaultLeftToRightBidiLevel = 0xFE;

  /// @brief A character's Bidi_Class property (UAX #9), in ICU's `UCharDirection` order.
  enum class BidiClass : uint8
  {
    LeftToRight,
    RightToLeft,
    EuropeanNumber,
    EuropeanNumberSeparator,
    EuropeanNumberTerminator,
    ArabicNumber,
    CommonNumberSeparator,
    BlockSeparator,
    SegmentSeparator,
    WhiteSpaceNeutral,
    OtherNeutral,
    LeftToRightEmbedding,
    LeftToRightOverride,
    RightToLeftArabic,
    RightToLeftEmbedding,
    RightToLeftOverride,
    PopDirectionalFormat,
    NonSpacingMark,
    BoundaryNeutral,
    FirstStrongIsolate,
    LeftToRightIsolate,
    RightToLeftIsolate,
    PopDirectionalIsolate,
  };

  /// @brief @p character's Bidi_Class property.
  KRYS_NODISCARD BidiClass GetBidiClass(char32 character) noexcept;

  /// @brief @p character's mirrored counterpart (Bidi_Mirroring_Glyph), such as `)` for `(`, or itself if it
  /// has none.
  KRYS_NODISCARD char32 MirroredCharacter(char32 character) noexcept;
}
