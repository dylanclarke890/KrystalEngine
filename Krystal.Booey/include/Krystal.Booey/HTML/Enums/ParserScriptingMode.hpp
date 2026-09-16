#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::html
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

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::html::ParserScriptingMode, 4u)

