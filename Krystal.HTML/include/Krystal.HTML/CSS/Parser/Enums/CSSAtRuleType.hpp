#pragma once

#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Core/MagicEnum.hpp"

namespace Krys::HTML
{
  enum class CSSAtRuleType : uint8
  {
    Invalid = 0,
    Charset,
    Import,
    Namespace
  };

  KRYS_NODISCARD CSSAtRuleType ParseCSSAtRuleType(utf8_stringview ident) noexcept
  {
    auto result =
      magic_enum::enum_cast<CSSAtRuleType>(
        stringview(reinterpret_cast<const char *>(ident.data()), ident.size()), magic_enum::case_insensitive)
        .value_or(CSSAtRuleType::Invalid);

    if (result == CSSAtRuleType::Charset && ident != u8"charset") KRYS_UNLIKELY
    {
      // SPEC: @charset is case-sensitive and must be a lowercase match.
      result = CSSAtRuleType::Invalid;
    }

    return result;
  }
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::CSSAtRuleType, 4uz);
