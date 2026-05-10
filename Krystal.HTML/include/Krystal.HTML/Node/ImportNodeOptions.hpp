#pragma once

#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.HTML/Utils/BoolOr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Variant.hpp"

namespace Krys::HTML
{
  class CustomElementRegistry;

  struct ImportNodeOptions
  {
    RefPtr<CustomElementRegistry> CustomElementRegistry;
    bool SelfOnly {false};
  };

  using BoolOrImportNodeOptions = BoolOr<ImportNodeOptions>;
}