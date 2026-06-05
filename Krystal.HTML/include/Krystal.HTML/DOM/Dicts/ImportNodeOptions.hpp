#pragma once

#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/BoolOr.hpp"

namespace Krys::HTML
{
  class CustomElementRegistry;

  /// @see https://dom.spec.whatwg.org/#dictdef-importnodeoptions
  struct ImportNodeOptions
  {
    RefPtr<CustomElementRegistry> CustomElementRegistry;
    bool SelfOnly {false};
  };

  using BoolOrImportNodeOptions = BoolOr<ImportNodeOptions>;
}