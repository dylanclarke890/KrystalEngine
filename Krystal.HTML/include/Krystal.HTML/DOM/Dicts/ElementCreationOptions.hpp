#pragma once

#include "Krystal.HTML/DOM/Types/DOMString.hpp"
#include "Krystal.HTML/DOM/Types/DOMString.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Variant.hpp"

namespace Krys::HTML
{
  class CustomElementRegistry;

  /// @see https://dom.spec.whatwg.org/#dictdef-elementcreationoptions
  struct ElementCreationOptions
  {
    RefPtr<CustomElementRegistry> CustomElementRegistry;
    DOMStringAtom Is {DOMStringAtom::Null()};
  };

  using ElementCreationOptionsOrString = Variant<ElementCreationOptions, DOMString>;
}