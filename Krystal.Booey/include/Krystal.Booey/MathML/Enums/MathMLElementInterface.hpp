#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::mathml
{
  /// @brief The class a MathML element is made as, which its tag name selects.
  enum class MathMLElementInterface : uint8
  {
    None,
    Fraction,
    Math,
    Menclose,
    Operator,
    Padded,
    Presentation,
    Root,
    Row,
    Scripts,
    Space,
    Token,
    UnderOver,
    Unknown,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::mathml::MathMLElementInterface, 14uz);
