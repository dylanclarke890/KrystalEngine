#pragma once

#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class ShadowRoot;

  /// @see https://html.spec.whatwg.org/#gethtmloptions
  struct GetHTMLOptions
  {
    bool SerializableShadowRoots = false;
    List<Ref<ShadowRoot>> ShadowRoots;
  };
}
