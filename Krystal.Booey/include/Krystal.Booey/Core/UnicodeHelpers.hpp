#pragma once

#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief The direction of the first strong character of @p content, or null for neutral content.
  KRYS_NODISCARD Maybe<TextDirection> BaseTextDirection(utf8_stringview content) noexcept;
}
