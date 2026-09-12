#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Func.hpp"

namespace krys::boo::html
{
  class Document;
  class HTMLElement;

  using CustomElementConstructor = Func<Ref<HTMLElement>(Document &)>;
}