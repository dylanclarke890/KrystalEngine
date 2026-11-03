#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  /// @see https://html.spec.whatwg.org/#navigationtype
  enum class NavigationNavigationType : uint8
  {
    Push,
    Replace,
    Reload,
    Traverse,
  };
}
