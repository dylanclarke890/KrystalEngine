#pragma once

#include "Krystal.HTML/DOM/MutationObserver/EmptyNodeMutationRecord.hpp"

namespace Krys::HTML
{
  class AttributesMutationRecord final : public EmptyNodeMutationRecord
  {
  private:
    DOMStringAtom _attributeName;
    DOMStringAtom _attributeNamespace;

  public:
    AttributesMutationRecord(Ref<Node> &&target, DOMStringAtom attributeName,
                             DOMStringAtom attributeNamespace, DOMStringAtom oldValue) noexcept;

    KRYS_NODISCARD DOMStringAtom Type() const noexcept override
    {
      return u8"attributes";
    }

    KRYS_NODISCARD DOMStringAtom AttributeName() const noexcept final
    {
      return _attributeName;
    }

    KRYS_NODISCARD DOMStringAtom AttributeNamespace() const noexcept final
    {
      return _attributeNamespace;
    }
  };
}