#pragma once

#include "Krystal.HTML/CSS/Types/MediaListOrDOMString.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  struct CSSStyleSheetInit
  {
    Maybe<DOMString> BaseURL = Null;
    MediaListOrDOMString Media = u8"";
    bool Disabled = false;
  };
}