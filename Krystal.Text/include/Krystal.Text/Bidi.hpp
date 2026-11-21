#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Containers/Span.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Strings/String.hpp"

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

  /// @brief A run of a paragraph whose characters resolved to one embedding level, as byte offsets.
  struct BidiLevelRun
  {
    size_t Start;
    size_t End;
    BidiLevel Level;
  };

  /// @brief Resolves the embedding levels of @p paragraph with the Unicode Bidirectional Algorithm (UAX #9).
  /// @param paragraphLevel The paragraph's level, or `DefaultLeftToRightBidiLevel` to take it from the first
  /// strong character.
  /// @return The runs of characters at one level, in logical order and covering the whole paragraph; empty if
  /// the paragraph is empty or the levels couldn't be resolved.
  KRYS_NODISCARD List<BidiLevelRun> ResolveBidiLevelRuns(utf8_stringview paragraph,
                                                         BidiLevel paragraphLevel) noexcept;

  /// @brief Reorders runs from logical to visual order by their levels (UAX #9 rule L2).
  /// @param levels Each run's level, in logical order; none above `MaxExplicitBidiLevel + 1`.
  /// @param visualToLogical Receives the logical index of the run at each visual position; as long as
  /// @p levels.
  void ReorderBidiRunsVisually(Span<const BidiLevel> levels, Span<int32> visualToLogical) noexcept;
}
