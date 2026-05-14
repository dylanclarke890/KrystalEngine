#pragma once

#include "Krystal.HTML/CustomElement/CustomElementConstructor.hpp"
#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/multipage/custom-elements.html#custom-element-definition
  class CustomElementDefinition
  {
    friend class CustomElementAlgorithms;

  private:
    DOMStringAtom _name;
    DOMStringAtom _localName;
    CustomElementConstructor _constructor;
    List<DOMStringAtom> _observedAttributes;
    bool _formAssociated : 1 {false};
    bool _disableInternals : 1 {false};
    bool _disableShadow : 1 {false};
    // TODO(impl): CUSTOM-ELEMENTS

  public:
    KRYS_NODISCARD DOMStringAtom Name() const noexcept
    {
      return _name;
    }

    KRYS_NODISCARD DOMStringAtom LocalName() const noexcept
    {
      return _localName;
    }
  };
}