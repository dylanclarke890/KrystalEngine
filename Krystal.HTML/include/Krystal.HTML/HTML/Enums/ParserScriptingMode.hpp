#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/multipage/parsing.html#parser-scripting-mode
  enum class ParserScriptingMode : uint8
  {
    Normal,
    Disabled,
    Inert,
    Fragment
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::ParserScriptingMode, 4u)

