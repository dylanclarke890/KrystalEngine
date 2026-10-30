#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::dom
{
  /// @brief How markup given as a string is parsed.
  enum class ParserContentPolicy : uint8
  {
    /// @brief `template` elements with a `shadowrootmode` attach declarative shadow roots.
    AllowDeclarativeShadowRoots,
    /// @brief The markup is parsed as HTML even in an XML document.
    AlwaysParseAsHTML
  };
}
