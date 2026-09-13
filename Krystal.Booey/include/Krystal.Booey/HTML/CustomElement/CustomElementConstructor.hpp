#pragma once

#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::html
{
  class HTMLElement;

  using CustomElementConstructor = Func<Ref<HTMLElement>(dom::Document &)>;
}