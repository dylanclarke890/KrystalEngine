#include "Krystal.Text/CaseMapping.hpp"
#include "Krystal.Core/Strings/CodePointIterator.hpp"
#include "Krystal.Core/Strings/Unicode.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include <string>
#include <unicode/ucasemap.h>
#include <unicode/uchar.h>
#include <unicode/ustring.h>

namespace krys::text
{
  char32 SimpleUppercase(char32 character) noexcept
  {
    return static_cast<char32>(u_toupper(static_cast<UChar32>(character)));
  }

  char32 SimpleTitlecase(char32 character) noexcept
  {
    return static_cast<char32>(u_totitle(static_cast<UChar32>(character)));
  }

  char32 SimpleCaseFold(char32 character) noexcept
  {
    return static_cast<char32>(u_foldCase(static_cast<UChar32>(character), U_FOLD_CASE_DEFAULT));
  }

  utf32_string ToTitlecase(char32 character) noexcept
  {
    Array<UChar, 2> source {};
    int32 sourceLength = EncodeUTF16(character, source);

    Array<UChar, 8> destination {};
    UErrorCode status = U_ZERO_ERROR;
    auto length = u_strToTitle(destination.data(), static_cast<int32>(destination.size()), source.data(),
                               sourceLength, nullptr, "", &status);
    if (U_FAILURE(status))
    {
      return utf32_string(1, character);
    }

    utf32_string result;
    for (int32 i = 0; i < length;)
    {
      auto decoded = DecodeUTF16(Span<const char16>(destination.data() + i, static_cast<size_t>(length - i)));
      result.push_back(decoded.CodePoint);
      i += decoded.Length;
    }
    return result;
  }

  namespace
  {
    KRYS_NODISCARD utf8_string ConvertCase(utf8_stringview text, utf8_stringview locale,
                                           bool toUpper) noexcept
    {
      std::string localeName(reinterpret_cast<const char *>(locale.data()), locale.size());

      UErrorCode status = U_ZERO_ERROR;
      auto *caseMap = ucasemap_open(localeName.c_str(), 0, &status);
      if (U_FAILURE(status))
      {
        return utf8_string(text);
      }

      auto convert = [&](char *destination, int32 capacity)
      {
        status = U_ZERO_ERROR;
        auto *source = reinterpret_cast<const char *>(text.data());
        auto sourceLength = static_cast<int32>(text.size());
        return toUpper ? ucasemap_utf8ToUpper(caseMap, destination, capacity, source, sourceLength, &status)
                       : ucasemap_utf8ToLower(caseMap, destination, capacity, source, sourceLength, &status);
      };

      auto length = convert(nullptr, 0);
      utf8_string result(static_cast<size_t>(length), u8'\0');
      if (status == U_BUFFER_OVERFLOW_ERROR || U_SUCCESS(status))
      {
        (void)convert(reinterpret_cast<char *>(result.data()), length);
      }
      ucasemap_close(caseMap);
      if (U_FAILURE(status))
      {
        return utf8_string(text);
      }
      return result;
    }
  }

  utf8_string ToUppercase(utf8_stringview text, utf8_stringview locale) noexcept
  {
    return ConvertCase(text, locale, true);
  }

  utf8_string ToLowercase(utf8_stringview text, utf8_stringview locale) noexcept
  {
    return ConvertCase(text, locale, false);
  }
}
