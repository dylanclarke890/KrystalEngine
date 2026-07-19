#pragma once

#include "Krystal.HTML/DOM/Types/DOMString.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Variant.hpp"

namespace Krys::HTML
{
  class MediaList;

  using MediaListOrDOMString = Variant<RefPtr<MediaList>, DOMString>;
}