#include "Krystal.Booey/Core/TextBoundaries.hpp"
#include "Krystal.Booey/Core/TextBreakIterator.hpp"
#include <unicode/ubrk.h>
#include <unicode/utf8.h>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD const uint8_t *Bytes(utf8_stringview text) noexcept
    {
      return reinterpret_cast<const uint8_t *>(text.data());
    }

    /// @brief The code point that starts at @p offset.
    KRYS_NODISCARD UChar32 CodePointAt(utf8_stringview text, int32 offset) noexcept
    {
      UChar32 c;
      U8_NEXT(Bytes(text), offset, static_cast<int32>(text.size()), c);
      return c;
    }

    /// @brief The code point that ends at @p offset.
    KRYS_NODISCARD UChar32 CodePointBefore(utf8_stringview text, int32 offset) noexcept
    {
      UChar32 c;
      U8_PREV(Bytes(text), 0, offset, c);
      return c;
    }
  }

  uint32 EndOfFirstWordBoundaryContext(utf8_stringview text) noexcept
  {
    int32 length = static_cast<int32>(text.size());
    for (int32 i = 0; i < length;)
    {
      int32 first = i;
      UChar32 ch;
      U8_NEXT(Bytes(text), i, length, ch);
      if (!RequiresContextForWordBoundary(static_cast<char32>(ch)))
      {
        return static_cast<uint32>(first);
      }
    }
    return static_cast<uint32>(length);
  }

  uint32 StartOfLastWordBoundaryContext(utf8_stringview text) noexcept
  {
    int32 length = static_cast<int32>(text.size());
    for (int32 i = length; i > 0;)
    {
      int32 last = i;
      UChar32 ch;
      U8_PREV(Bytes(text), 0, i, ch);
      if (!RequiresContextForWordBoundary(static_cast<char32>(ch)))
      {
        return static_cast<uint32>(last);
      }
    }
    return 0;
  }

  int32 FindNextWordFromIndex(utf8_stringview text, int32 position, bool forward) noexcept
  {
    UBreakIterator *it = WordBreakIterator(text);

    if (forward)
    {
      position = ubrk_following(it, position);
      while (position != UBRK_DONE)
      {
        // We stop searching when the character preceeding the break is alphanumeric.
        if (static_cast<uint32>(position) < text.size() && u_isalnum(CodePointBefore(text, position)))
        {
          return position;
        }

        position = ubrk_following(it, position);
      }

      return static_cast<int32>(text.size());
    }
    else
    {
      position = ubrk_preceding(it, position);
      while (position != UBRK_DONE)
      {
        // We stop searching when the character following the break is alphanumeric.
        if (position && u_isalnum(CodePointAt(text, position)))
        {
          return position;
        }

        position = ubrk_preceding(it, position);
      }

      return 0;
    }
  }

  void FindWordBoundary(utf8_stringview text, int32 position, int32 *start, int32 *end) noexcept
  {
    UBreakIterator *it = WordBreakIterator(text);
    *end = ubrk_following(it, position);
    if (*end < 0)
    {
      *end = ubrk_last(it);
    }
    *start = ubrk_previous(it);
  }

  void FindEndWordBoundary(utf8_stringview text, int32 position, int32 *end) noexcept
  {
    UBreakIterator *it = WordBreakIterator(text);
    *end = ubrk_following(it, position);
    if (*end < 0)
    {
      *end = ubrk_last(it);
    }
  }
}
