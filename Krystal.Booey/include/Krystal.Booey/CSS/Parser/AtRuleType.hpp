#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/MagicEnum.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo::css
{
  enum class AtRuleType : uint8
  {
    Invalid = 0,
    Charset,
    Import,
    Namespace
  };

  KRYS_NODISCARD AtRuleType ParseCSSAtRuleType(utf8_stringview ident) noexcept
  {
    // TODO: this won't be good enough when at-rules are hyphenated, but for now it will work for the three
    // at-rules we support.
    return magic_enum::enum_cast<AtRuleType>(
             stringview(reinterpret_cast<const char *>(ident.data()), ident.size()),
             magic_enum::case_insensitive)
      .value_or(AtRuleType::Invalid);
  }
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::css::AtRuleType, 4uz);
