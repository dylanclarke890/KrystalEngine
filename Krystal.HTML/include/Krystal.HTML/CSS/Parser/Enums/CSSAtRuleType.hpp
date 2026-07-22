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
    // TODO: this won't be good enough when at-rules are hyphenated, but for now it will work for the three
    // at-rules we support.
    return magic_enum::enum_cast<CSSAtRuleType>(
             stringview(reinterpret_cast<const char *>(ident.data()), ident.size()),
             magic_enum::case_insensitive)
      .value_or(CSSAtRuleType::Invalid);
  }
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::CSSAtRuleType, 4uz);
