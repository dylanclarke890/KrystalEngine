#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class DOMInterface : uint8
  {
    None,
    Anchor,
    Base,
    Body,
    BR,
    Data,
    Div,
    DList,
    Head,
    Heading,
    HR,
    Html,
    Image,
    Link,
    LI,
    Menu,
    Meta,
    Mod,
    OList,
    Paragraph,
    Picture,
    Pre,
    Quote,
    Script,
    Slot,
    Source,
    Span,
    Style,
    Title,
    Time,
    UList,
    Unknown,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::DOMInterface, 32u);