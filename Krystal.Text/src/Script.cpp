#include "Krystal.Text/Script.hpp"
#include <unicode/uscript.h>

namespace krys::text
{
  // Script.hpp is generated from ICU's UScriptCode, so the values are ICU's.
  static_assert(static_cast<int>(Script::Invalid) == USCRIPT_INVALID_CODE);
  static_assert(static_cast<int>(Script::Common) == USCRIPT_COMMON);
  static_assert(static_cast<int>(Script::Latin) == USCRIPT_LATIN);
  static_assert(static_cast<int>(Script::Han) == USCRIPT_HAN);
  static_assert(static_cast<int>(Script::KatakanaOrHiragana) == USCRIPT_KATAKANA_OR_HIRAGANA);
  static_assert(static_cast<int>(Script::Unknown) == USCRIPT_UNKNOWN);

  Script GetScript(char32 character) noexcept
  {
    UErrorCode error = U_ZERO_ERROR;
    auto script = uscript_getScript(static_cast<UChar32>(character), &error);
    return U_FAILURE(error) ? Script::Invalid : static_cast<Script>(script);
  }

  bool HasScript(char32 character, Script script) noexcept
  {
    return uscript_hasScript(static_cast<UChar32>(character), static_cast<UScriptCode>(script));
  }

  utf8_stringview ScriptShortName(Script script) noexcept
  {
    const char *name = uscript_getShortName(static_cast<UScriptCode>(script));
    if (!name)
    {
      return {};
    }
    return utf8_stringview(reinterpret_cast<const char8 *>(name));
  }
}
