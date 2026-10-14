#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::style
{
  /// @brief Where a declaration comes from, in cascade order.
  /// @see https://drafts.csswg.org/css-cascade-5/#cascading-origins
  enum class DeclarationOrigin : uint8
  {
    UserAgent,
    User,
    Author
  };
}
