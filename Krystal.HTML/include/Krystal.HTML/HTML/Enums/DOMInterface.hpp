#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class DOMInterface : uint8
  {
    None,
    Base,
    Body,
    Head,
    Heading,
    HR,
    Html,
    Link,
    LI,
    Menu,
    Meta,
    OList,
    Paragraph,
    Pre,
    Quote,
    Script,
    Slot,
    Style,
    Title,
    UList,
    Unknown,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::DOMInterface, 17u);