#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  /// @brief Specifies the allowed rules in a CSS stylesheet.
  enum class CSSAllowedRules : uint8
  {
    /// @brief The @charset rule and later rules are allowed.
    Charset,
    /// @brief The @import rule and later rules are allowed.
    Import,
    /// @brief The @namespace rule and later rules are allowed.
    Namespace,
    /// @brief General at-rules and qualified rules are allowed.
    Regular,
    /// @brief No rules are allowed.
    None,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::CSSAllowedRules, 5uz);
