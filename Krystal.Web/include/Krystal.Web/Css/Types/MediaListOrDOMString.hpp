#pragma once

#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include "Krystal.Web/Dom/Types/DOMString.hpp"

namespace krys::web
{
  class MediaList;
}

namespace krys::web::css
{
  using MediaListOrDOMString = Variant<RefPtr<MediaList>, DOMString>;
}
