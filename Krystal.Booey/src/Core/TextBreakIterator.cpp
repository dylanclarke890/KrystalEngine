#include "Krystal.Booey/Core/TextBreakIterator.hpp"
#include <algorithm>
#include <unicode/ubrk.h>
#include <unicode/utext.h>

namespace krys::boo
{
  uint32 NumGraphemeClusters(utf8_stringview string) noexcept
  {
    if (string.empty())
    {
      return 0;
    }

    // ASCII without CR (CR LF is one cluster) has one cluster per character.
    if (std::ranges::all_of(string, [](char8 c) { return c < 0x80 && c != u8'\r'; }))
    {
      return static_cast<uint32>(string.size());
    }

    UErrorCode status = U_ZERO_ERROR;
    UText text = UTEXT_INITIALIZER;
    utext_openUTF8(&text, reinterpret_cast<const char *>(string.data()), static_cast<int64_t>(string.size()),
                   &status);
    UBreakIterator *iterator = ubrk_open(UBRK_CHARACTER, nullptr, nullptr, 0, &status);
    ubrk_setUText(iterator, &text, &status);

    uint32 count = 0;
    if (U_SUCCESS(status))
    {
      for (int32_t position = ubrk_next(iterator); position != UBRK_DONE; position = ubrk_next(iterator))
      {
        ++count;
      }
    }

    ubrk_close(iterator);
    utext_close(&text);
    return count;
  }
}
