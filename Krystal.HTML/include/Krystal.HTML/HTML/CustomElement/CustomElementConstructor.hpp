#pragma once

#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Func.hpp"

namespace Krys::HTML
{
  class Document;
  class HTMLElement;

  using CustomElementConstructor = Func<Ref<HTMLElement>(Document &)>;
}