#pragma once

#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief The direction of the first strong character of @p content, or null for neutral content.
  KRYS_NODISCARD Maybe<TextDirection> BaseTextDirection(utf8_stringview content) noexcept;

  /// @brief The direction of the first strong character of @p content, or null if it has none.
  /// @note Unlike `BaseTextDirection`, Latin-1 text without strong characters (digits, punctuation) is
  /// neutral.
  KRYS_NODISCARD Maybe<TextDirection> DefaultWritingDirection(utf8_stringview content) noexcept;
}
