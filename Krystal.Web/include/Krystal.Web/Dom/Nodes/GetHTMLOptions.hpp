#pragma once

#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"

namespace krys::web
{
  class ShadowRoot;
}

namespace krys::web::dom
{
  /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#gethtmloptions
  struct GetHTMLOptions
  {
    bool SerializableShadowRoots = false;
    List<Ref<ShadowRoot>> ShadowRoots;
  };
}
