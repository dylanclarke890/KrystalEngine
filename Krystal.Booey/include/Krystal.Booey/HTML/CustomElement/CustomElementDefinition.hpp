#pragma once

#include "Krystal.Booey/HTML/CustomElement/CustomElementConstructor.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/multipage/custom-elements.html#custom-element-definition
  class CustomElementDefinition
  {
    friend class CustomElementAlgorithms;

  private:
    dom::DOMStringAtom _name;
    dom::DOMStringAtom _localName;
    CustomElementConstructor _constructor;
    List<dom::DOMStringAtom> _observedAttributes;
    bool _formAssociated : 1 {false};
    bool _disableInternals : 1 {false};
    bool _disableShadow : 1 {false};
    // TODO(impl): CUSTOM-ELEMENTS

  public:
    KRYS_NODISCARD dom::DOMStringAtom Name() const noexcept
    {
      return _name;
    }

    KRYS_NODISCARD dom::DOMStringAtom LocalName() const noexcept
    {
      return _localName;
    }
  };
}