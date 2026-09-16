#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::css
{
  class MediaList;

  using MediaListOrDOMString = Variant<RefPtr<MediaList>, dom::DOMString>;
}