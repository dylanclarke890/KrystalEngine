#include "Krystal.Text/Bidi.hpp"
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
}
