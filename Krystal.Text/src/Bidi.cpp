#include "Krystal.Text/Bidi.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Strings/CodePointIterator.hpp"
#include <unicode/ubidi.h>
#include <unicode/uchar.h>

namespace krys::text
{
  static_assert(LeftToRightBidiLevel == UBIDI_LTR);
  static_assert(RightToLeftBidiLevel == UBIDI_RTL);
  static_assert(MaxExplicitBidiLevel == UBIDI_MAX_EXPLICIT_LEVEL);
  static_assert(DefaultLeftToRightBidiLevel == UBIDI_DEFAULT_LTR);

  static_assert(static_cast<int>(BidiClass::LeftToRight) == U_LEFT_TO_RIGHT);
  static_assert(static_cast<int>(BidiClass::RightToLeft) == U_RIGHT_TO_LEFT);
  static_assert(static_cast<int>(BidiClass::EuropeanNumber) == U_EUROPEAN_NUMBER);
  static_assert(static_cast<int>(BidiClass::EuropeanNumberSeparator) == U_EUROPEAN_NUMBER_SEPARATOR);
  static_assert(static_cast<int>(BidiClass::EuropeanNumberTerminator) == U_EUROPEAN_NUMBER_TERMINATOR);
  static_assert(static_cast<int>(BidiClass::ArabicNumber) == U_ARABIC_NUMBER);
  static_assert(static_cast<int>(BidiClass::CommonNumberSeparator) == U_COMMON_NUMBER_SEPARATOR);
  static_assert(static_cast<int>(BidiClass::BlockSeparator) == U_BLOCK_SEPARATOR);
  static_assert(static_cast<int>(BidiClass::SegmentSeparator) == U_SEGMENT_SEPARATOR);
  static_assert(static_cast<int>(BidiClass::WhiteSpaceNeutral) == U_WHITE_SPACE_NEUTRAL);
  static_assert(static_cast<int>(BidiClass::OtherNeutral) == U_OTHER_NEUTRAL);
  static_assert(static_cast<int>(BidiClass::LeftToRightEmbedding) == U_LEFT_TO_RIGHT_EMBEDDING);
  static_assert(static_cast<int>(BidiClass::LeftToRightOverride) == U_LEFT_TO_RIGHT_OVERRIDE);
  static_assert(static_cast<int>(BidiClass::RightToLeftArabic) == U_RIGHT_TO_LEFT_ARABIC);
  static_assert(static_cast<int>(BidiClass::RightToLeftEmbedding) == U_RIGHT_TO_LEFT_EMBEDDING);
  static_assert(static_cast<int>(BidiClass::RightToLeftOverride) == U_RIGHT_TO_LEFT_OVERRIDE);
  static_assert(static_cast<int>(BidiClass::PopDirectionalFormat) == U_POP_DIRECTIONAL_FORMAT);
  static_assert(static_cast<int>(BidiClass::NonSpacingMark) == U_DIR_NON_SPACING_MARK);
  static_assert(static_cast<int>(BidiClass::BoundaryNeutral) == U_BOUNDARY_NEUTRAL);
  static_assert(static_cast<int>(BidiClass::FirstStrongIsolate) == U_FIRST_STRONG_ISOLATE);
  static_assert(static_cast<int>(BidiClass::LeftToRightIsolate) == U_LEFT_TO_RIGHT_ISOLATE);
  static_assert(static_cast<int>(BidiClass::RightToLeftIsolate) == U_RIGHT_TO_LEFT_ISOLATE);
  static_assert(static_cast<int>(BidiClass::PopDirectionalIsolate) == U_POP_DIRECTIONAL_ISOLATE);

  BidiClass GetBidiClass(char32 character) noexcept
  {
    return static_cast<BidiClass>(u_charDirection(static_cast<UChar32>(character)));
  }

  char32 MirroredCharacter(char32 character) noexcept
  {
    return static_cast<char32>(u_charMirror(static_cast<UChar32>(character)));
  }

  namespace
  {
    /// @brief The UTF-16 form of UTF-8 text, for ICU's bidi algorithm, with the byte offset of each UTF-16
    /// offset (and of the end).
    struct UTF16Paragraph
    {
      List<UChar> Characters;
      List<size_t> ByteOffsets;

      explicit UTF16Paragraph(utf8_stringview text) noexcept
      {
        Characters.reserve(text.size());
        ByteOffsets.reserve(text.size() + 1);
        for (size_t position = 0; position < text.size();)
        {
          auto decoded = DecodeUTF8(Span<const char8>(text.data() + position, text.size() - position));
          if (decoded.CodePoint <= 0xFFFF)
          {
            Characters.push_back(static_cast<UChar>(decoded.CodePoint));
            ByteOffsets.push_back(position);
          }
          else
          {
            auto [lead, trail] = UTF16SplitSurrogates(decoded.CodePoint);
            Characters.push_back(lead);
            Characters.push_back(trail);
            ByteOffsets.push_back(position);
            ByteOffsets.push_back(position);
          }
          position += decoded.Length;
        }
        ByteOffsets.push_back(text.size());
      }
    };
  }

  List<BidiLevelRun> ResolveBidiLevelRuns(utf8_stringview paragraph, BidiLevel paragraphLevel) noexcept
  {
    if (paragraph.empty())
    {
      return {};
    }

    UBiDi *bidi = ubidi_open();
    if (!bidi)
    {
      return {};
    }

    // ICU's bidi algorithm takes UTF-16: the runs it finds are mapped back to byte offsets.
    UTF16Paragraph utf16 {paragraph};
    auto length = static_cast<int32>(utf16.Characters.size());
    UErrorCode error = U_ZERO_ERROR;
    ubidi_setPara(bidi, utf16.Characters.data(), length, paragraphLevel, nullptr, &error);

    List<BidiLevelRun> runs;
    if (U_SUCCESS(error))
    {
      for (int32 start = 0; start < length;)
      {
        int32 end = start;
        UBiDiLevel level;
        ubidi_getLogicalRun(bidi, start, &end, &level);
        runs.push_back({.Start = utf16.ByteOffsets[static_cast<size_t>(start)],
                        .End = utf16.ByteOffsets[static_cast<size_t>(end)],
                        .Level = level});
        start = end;
      }
    }
    ubidi_close(bidi);
    return runs;
  }

  void ReorderBidiRunsVisually(Span<const BidiLevel> levels, Span<int32> visualToLogical) noexcept
  {
    krys_debug_assert(levels.size() == visualToLogical.size());
    ubidi_reorderVisual(levels.data(), static_cast<int32>(levels.size()), visualToLogical.data());
  }
}
