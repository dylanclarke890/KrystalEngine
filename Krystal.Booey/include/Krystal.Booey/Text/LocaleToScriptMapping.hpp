#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/String.hpp"
#include <unicode/uscript.h>

namespace krys::boo
{
  /// @brief The script a BCP 47 locale (e.g. `zh-Hant`, `sr_Latn`) is written in: the locale's language and
  /// region, then its script subtag. `USCRIPT_COMMON` when nothing matches.
  KRYS_NODISCARD UScriptCode LocaleToScriptCode(utf8_stringview) noexcept;

  /// @brief The script an ISO 15924 script code (e.g. `Latn`, ASCII case-insensitive) names.
  /// `USCRIPT_INVALID_CODE` when nothing matches.
  KRYS_NODISCARD UScriptCode ScriptNameToCode(utf8_stringview) noexcept;
}
