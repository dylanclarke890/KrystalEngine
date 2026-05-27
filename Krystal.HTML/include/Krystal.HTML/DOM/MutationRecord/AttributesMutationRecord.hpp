#pragma once

#include "Krystal.HTML/DOM/MutationRecord/EmptyNodeMutationRecord.hpp"
#include "Krystal.HTML/QualifiedName.hpp"

namespace Krys::HTML
{
  class Element;

  class AttributesMutationRecord : public EmptyNodeMutationRecord
  {
  private:
    DOMStringAtom _attributeName;
    DOMStringAtom _attributeNamespace;
    DOMString _oldValue;

  public:
    AttributesMutationRecord(Element &target, const QualifiedName &name, DOMString &&oldValue) noexcept;
  };
}