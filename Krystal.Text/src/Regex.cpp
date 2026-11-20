#include "Krystal.Text/Regex.hpp"
#include <unicode/uregex.h>
#include <unicode/utext.h>
#include <utility>

namespace krys::text
{
  Maybe<Regex> Regex::Compile(utf8_stringview pattern) noexcept
  {
    UErrorCode status = U_ZERO_ERROR;
    UText patternText = UTEXT_INITIALIZER;
    utext_openUTF8(&patternText, reinterpret_cast<const char *>(pattern.data()),
                   static_cast<int64_t>(pattern.size()), &status);
    auto *regex = uregex_openUText(&patternText, UREGEX_ERROR_ON_UNKNOWN_ESCAPES, nullptr, &status);
    utext_close(&patternText);
    if (U_FAILURE(status) || !regex)
    {
      if (regex)
      {
        uregex_close(regex);
      }
      return null;
    }
    return Regex(regex);
  }

  Regex::Regex(Regex &&other) noexcept : _regex(std::exchange(other._regex, nullptr))
  {
  }

  Regex &Regex::operator=(Regex &&other) noexcept
  {
    if (this != &other)
    {
      if (_regex)
      {
        uregex_close(_regex);
      }
      _regex = std::exchange(other._regex, nullptr);
    }
    return *this;
  }

  Regex::~Regex() noexcept
  {
    if (_regex)
    {
      uregex_close(_regex);
    }
  }

  bool Regex::MatchesWhole(utf8_stringview text) const noexcept
  {
    UErrorCode status = U_ZERO_ERROR;
    UText utext = UTEXT_INITIALIZER;
    utext_openUTF8(&utext, reinterpret_cast<const char *>(text.data()), static_cast<int64_t>(text.size()),
                   &status);
    uregex_setUText(_regex, &utext, &status);
    bool matches = uregex_matches64(_regex, -1, &status);
    utext_close(&utext);
    return U_SUCCESS(status) && matches;
  }
}
