#pragma once

#include "Krystal.Booey/DOM/Dicts/EventInit.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::dom
{
  /// @see https://dom.spec.whatwg.org/#dictdef-customeventinit
  template <typename T>
  struct CustomEventInit : public EventInit
  {
    Maybe<T> Detail;
  };
}