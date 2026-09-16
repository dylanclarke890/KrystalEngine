#pragma once

#include "Krystal.Core/Types/BoolOr.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::html
{
  class CustomElementRegistry;
}

namespace krys::boo::dom
{
  /// @see https://dom.spec.whatwg.org/#dictdef-importnodeoptions
  struct ImportNodeOptions
  {
    RefPtr<html::CustomElementRegistry> CustomElementRegistry;
    bool SelfOnly {false};
  };

  using BoolOrImportNodeOptions = BoolOr<ImportNodeOptions>;
}