#pragma once

#include "Krystal.HTML/DOM/Dicts/EventInit.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  /// @see https://dom.spec.whatwg.org/#dictdef-customeventinit
  template <typename T>
  struct CustomEventInit : public EventInit
  {
    Maybe<T> Detail;
  };
}