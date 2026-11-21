#pragma once

#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  class ShadowRoot;
}

namespace krys::boo::dom
{
  /// @see https://html.spec.whatwg.org/#gethtmloptions
  struct GetHTMLOptions
  {
    bool SerializableShadowRoots = false;
    List<Ref<ShadowRoot>> ShadowRoots;
  };
}
