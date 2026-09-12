#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::html
{
  class CustomElementRegistry;
}

namespace krys::boo::dom
{
  /// @see https://dom.spec.whatwg.org/#dictdef-elementcreationoptions
  struct ElementCreationOptions
  {
    RefPtr<html::CustomElementRegistry> CustomElementRegistry;
    DOMStringAtom Is {DOMStringAtom::Null()};
  };

  using ElementCreationOptionsOrString = Variant<ElementCreationOptions, DOMString>;
}