#include "Krystal.Booey/Core/UnicodeHelpers.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include <algorithm>
#include <unicode/uchar.h>

namespace krys::boo
{
  Maybe<TextDirection> BaseTextDirection(utf8_stringview content) noexcept
  {
    // Latin-1 text is left to right.
    if (std::ranges::all_of(content, [](char8 c) { return c < 0x80; }))
    {
      return TextDirection::LTR;
    }

    // The direction of the first strong character, as ubidi_getBaseDirection.
    bool isLatin1 = true;
    for (text::CodePointIterator it {content}; !it.AtEnd(); ++it)
    {
      isLatin1 = isLatin1 && *it <= 0xFF;
      switch (u_charDirection(static_cast<UChar32>(*it)))
      {
        case U_LEFT_TO_RIGHT:        return TextDirection::LTR;
        case U_RIGHT_TO_LEFT:
        case U_RIGHT_TO_LEFT_ARABIC: return TextDirection::RTL;
        default:                     break;
      }
    }
    if (isLatin1)
    {
      return TextDirection::LTR;
    }
    return null;
  }

  Maybe<TextDirection> DefaultWritingDirection(utf8_stringview content) noexcept
  {
    for (text::CodePointIterator it {content}; !it.AtEnd(); ++it)
    {
      switch (u_charDirection(static_cast<UChar32>(*it)))
      {
        case U_LEFT_TO_RIGHT:        return TextDirection::LTR;
        case U_RIGHT_TO_LEFT:
        case U_RIGHT_TO_LEFT_ARABIC: return TextDirection::RTL;
        default:                     break;
      }
    }
    return null;
  }
}
