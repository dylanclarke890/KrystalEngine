#pragma once

#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/String/StringAtom.hpp"

namespace Krys::HTML
{
  class ContainerNode;
  class Node;
  class NodeList;

  class MutationRecord : public RefCounted<MutationRecord>
  {
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

    KRYS_NODISCARD virtual DOMStringAtom AttributeName() const noexcept
    {
      return DOMStringAtom::Empty();
    }

    KRYS_NODISCARD virtual DOMStringAtom AttributeNamespace() const noexcept
    {
      return DOMStringAtom::Empty();
    }

    KRYS_NODISCARD virtual DOMString OldValue() const noexcept
    {
      return u8"";
    }
  };
}