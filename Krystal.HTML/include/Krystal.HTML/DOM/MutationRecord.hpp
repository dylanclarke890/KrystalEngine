#pragma once

#include "Krystal.HTML/MutationObserver/MutationRecordType.hpp"
#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.HTML/Types/DOMStringAtom.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class Node;
  class NodeList;

  class MutationRecord : public RefCounted<MutationRecord>
  {
  protected:
    MutationRecordType _type : BitCount<MutationRecordType>() {};

    MutationRecord(MutationRecordType type) noexcept : _type(type)
    {
    }

  public:
    virtual ~MutationRecord() noexcept = default;

    KRYS_NODISCARD MutationRecordType Type() const noexcept
    {
      return _type;
    }

    KRYS_NODISCARD virtual RefPtr<Node> Target() const noexcept = 0;

    KRYS_NODISCARD virtual RefPtr<NodeList> AddedNodes() const noexcept = 0;

    KRYS_NODISCARD virtual RefPtr<NodeList> RemovedNodes() const noexcept = 0;

    KRYS_NODISCARD virtual RefPtr<Node> PreviousSibling() const noexcept
    {
      return nullptr;
    }

    KRYS_NODISCARD virtual RefPtr<Node> NextSibling() const noexcept
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