#pragma once

#include "Krystal.HTML/DOM/Types/DOMString.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class Node;
  class NodeList;

  /// @see https://dom.spec.whatwg.org/#interface-mutationrecord
  class MutationRecord : public RefCounted<MutationRecord>
  {
  public:
    virtual ~MutationRecord() noexcept = default;

#pragma region MutationRecord - https://dom.spec.whatwg.org/#mutationrecord

    /// https://dom.spec.whatwg.org/#dom-mutationrecord-type
    KRYS_NODISCARD virtual DOMStringAtom Type() const noexcept = 0;

    /// https://dom.spec.whatwg.org/#dom-mutationrecord-target
    KRYS_NODISCARD virtual Ref<Node> Target() const noexcept = 0;

    /// https://dom.spec.whatwg.org/#dom-mutationrecord-addednodes
    KRYS_NODISCARD virtual RefPtr<NodeList> AddedNodes() const noexcept = 0;

    /// https://dom.spec.whatwg.org/#dom-mutationrecord-removednodes
    KRYS_NODISCARD virtual RefPtr<NodeList> RemovedNodes() const noexcept = 0;

    /// https://dom.spec.whatwg.org/#dom-mutationrecord-previoussibling
    KRYS_NODISCARD virtual RefPtr<Node> PreviousSibling() const noexcept
    {
      return nullptr;
    }

    /// https://dom.spec.whatwg.org/#dom-mutationrecord-nextsibling
    KRYS_NODISCARD virtual RefPtr<Node> NextSibling() const noexcept
    {
      return nullptr;
    }

    /// https://dom.spec.whatwg.org/#dom-mutationrecord-attributename
    KRYS_NODISCARD virtual DOMStringAtom AttributeName() const noexcept
    {
      return DOMStringAtom::Null();
    }

    /// https://dom.spec.whatwg.org/#dom-mutationrecord-attributenamespace
    KRYS_NODISCARD virtual DOMStringAtom AttributeNamespace() const noexcept
    {
      return DOMStringAtom::Null();
    }

    /// https://dom.spec.whatwg.org/#dom-mutationrecord-oldvalue
    KRYS_NODISCARD virtual DOMStringAtom OldValue() const noexcept
    {
      return DOMStringAtom::Null();
    }

#pragma endregion
  };
}