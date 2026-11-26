#pragma once

#include "Krystal.Booey/Dom/Types/DOMString.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo
{
  class MediaList;
}

namespace krys::boo::css
{
  using MediaListOrDOMString = Variant<RefPtr<MediaList>, DOMString>;
}
