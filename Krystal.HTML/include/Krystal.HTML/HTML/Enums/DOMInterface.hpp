#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class DOMInterface : uint16
  {
    HTMLElement,
    Base,
    Body,
    Head,
    Heading,
    Html,
    Link,
    Meta,
    Pre,
    Script,
    Slot,
    Style,
    Title,
    Unknown,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::DOMInterface, 14u);