#pragma once

#include "Krystal.Booey/Dom/MutationObserver/EmptyNodeMutationRecord.hpp"

namespace krys::boo::dom
{
  class AttributesMutationRecord final : public EmptyNodeMutationRecord
  {
  private:
    DOMStringAtom _attributeName;
    DOMStringAtom _attributeNamespace;

  public:
    AttributesMutationRecord(Ref<Node> &&target, DOMStringAtom attributeName,
                             DOMStringAtom attributeNamespace, DOMStringAtom oldValue) noexcept;

    KRYS_NODISCARD const DOMStringAtom &Type() const noexcept override
    {
      static const DOMStringAtom type {u8"attributes"};
      return type;
    }

    KRYS_NODISCARD const DOMStringAtom &AttributeName() const noexcept KRYS_LIFETIME_BOUND final
    {
      return _attributeName;
    }

    KRYS_NODISCARD const DOMStringAtom &AttributeNamespace() const noexcept KRYS_LIFETIME_BOUND final
    {
      return _attributeNamespace;
    }
  };
}
