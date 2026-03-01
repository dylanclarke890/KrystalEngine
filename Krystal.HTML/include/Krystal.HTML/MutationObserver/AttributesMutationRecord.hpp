#pragma once

#include "Krystal.HTML/MutationObserver/detail/EmptyNodeMutationRecord.hpp"

namespace Krys::HTML
{
  class Element;
  struct QualifiedName;

  class AttributesMutationRecord : public ::Krys::HTML::detail::EmptyNodeMutationRecord
  {
  private:
    StringAtom _attributeName;
    StringAtom _attributeNamespace;
    DOMString _oldValue;

  public:
    AttributesMutationRecord(Element &target, const QualifiedName &name, const DOMString &oldValue) noexcept;

    KRYS_NODISCARD const StringAtom &Type() const noexcept override
    {
      static const StringAtom type = u8"attributes";
      return type;
    }
  };
}