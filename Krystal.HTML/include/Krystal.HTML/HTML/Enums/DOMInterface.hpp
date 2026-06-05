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
    Form,
    Head,
    Heading,
    HR,
    Html,
    IFrame,
    Image,
    Input,
    Link,
    Label,
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
    Table,
    TableCaption,
    TableCell,
    TableCol,
    TableRow,
    TableSection,
    Title,
    Time,
    Track,
    UList,
    Unknown,
    Video,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::DOMInterface, 49u);