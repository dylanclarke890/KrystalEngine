#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class Namespace : uint8
  {
    Unknown,
    HTML,
    SVG,
    MathML,
    XLink,
    XML,
    XMLNS,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::Namespace, 7uz);