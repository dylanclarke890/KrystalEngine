#pragma once

#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/String/StringAtom.hpp"
#include "Krystal.Lib/Types/Lazy.hpp"

namespace Krys::HTML
{
  class Node;
  class NodeList;

  class MutationRecord : public RefCounted<MutationRecord>
  {
  private:
    StringAtom _type;
    RawPtr<Node> _target;
    LazyValue<RefPtr<NodeList>> _addedNodes;
    LazyValue<RefPtr<NodeList>> _removedNodes;
    RawPtr<Node> _previousSibling;
    RawPtr<Node> _nextSibling;
    StringAtom _attributeName;
    StringAtom _attributeNamespace;
    DOMString _oldValue;

  public:
    constexpr virtual ~MutationRecord() noexcept = default;

    KRYS_NODISCARD virtual const StringAtom &Type() const noexcept = 0;
    KRYS_NODISCARD virtual RawPtr<Node> Target() const noexcept = 0;

    KRYS_NODISCARD virtual RawPtr<NodeList> AddedNodes() const noexcept = 0;
    KRYS_NODISCARD virtual RawPtr<NodeList> RemovedNodes() const noexcept = 0;

    KRYS_NODISCARD virtual RawPtr<Node> PreviousSibling() const noexcept
    {
      return nullptr;
    }
    KRYS_NODISCARD virtual RawPtr<Node> NextSibling() const noexcept
    {
      return nullptr;
    }

    KRYS_NODISCARD virtual const StringAtom &AttributeName() const noexcept = 0;
    KRYS_NODISCARD virtual const StringAtom &AttributeNamespace() const noexcept = 0;

    KRYS_NODISCARD virtual DOMString OldValue() const noexcept
    {
      return u8"";
    }
  };
}