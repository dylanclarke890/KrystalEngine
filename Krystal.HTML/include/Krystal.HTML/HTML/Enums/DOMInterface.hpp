#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class DOMInterface : uint8
  {
    None,
    Anchor,
    Area,
    Audio,
    Base,
    Body,
    BR,
    Data,
    Div,
    DList,
    Embed,
    Head,
    Heading,
    HR,
    Html,
    IFrame,
    Image,
    Link,
    LI,
    Map,
    Media,
    Menu,
    Meta,
    Mod,
    Object,
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
    Track,
    UList,
    Unknown,
    Video,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::DOMInterface, 39u);