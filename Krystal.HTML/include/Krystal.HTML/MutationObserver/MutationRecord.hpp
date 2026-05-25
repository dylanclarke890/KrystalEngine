#pragma once

#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.HTML/MutationObserver/MutationRecordType.hpp"
#include "Krystal.HTML/QualifiedName.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CharacterData;
  class ContainerNode;
  class Element;
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

  namespace detail
  {
    class EmptyNodeMutationRecord : public MutationRecord
    {
    private:
      Ref<Node> _target;
      mutable RefPtr<NodeList> _addedNodes;
      mutable RefPtr<NodeList> _removedNodes;
      DOMString _oldValue;

    protected:
      EmptyNodeMutationRecord(MutationRecordType type, Node &target, DOMString &&oldValue) noexcept;

    public:
      KRYS_NODISCARD RefPtr<Node> Target() const noexcept final
      {
        return _target;
      }

      KRYS_NODISCARD RefPtr<NodeList> AddedNodes() const noexcept final;

      KRYS_NODISCARD RefPtr<NodeList> RemovedNodes() const noexcept final;

      KRYS_NODISCARD DOMString OldValue() const noexcept final
      {
        return _oldValue;
      }
    };

    class AttributesMutationRecord : public EmptyNodeMutationRecord
    {
    private:
      DOMStringAtom _attributeName;
      DOMStringAtom _attributeNamespace;
      DOMString _oldValue;

    public:
      AttributesMutationRecord(Element &target, const QualifiedName &name, DOMString &&oldValue) noexcept;
    };

    class CharacterDataMutationRecord : public EmptyNodeMutationRecord
    {
    public:
      CharacterDataMutationRecord(CharacterData &target, DOMString &&oldValue) noexcept;
    };

    class ChildListMutationRecord : public MutationRecord
    {
    private:
      Ref<Node> _target;
      Ref<NodeList> _addedNodes;
      Ref<NodeList> _removedNodes;
      Ref<Node> _previousSibling;
      Ref<Node> _nextSibling;

    public:
      ChildListMutationRecord(ContainerNode &target, Ref<NodeList> &&added, Ref<NodeList> &&removed,
                              RefPtr<Node> &&previousSibling, RefPtr<Node> &&nextSibling) noexcept;

      KRYS_NODISCARD RefPtr<Node> Target() const noexcept override
      {
        return _target;
      }

      KRYS_NODISCARD RefPtr<NodeList> AddedNodes() const noexcept override
      {
        return _addedNodes;
      }

      KRYS_NODISCARD RefPtr<NodeList> RemovedNodes() const noexcept override
      {
        return _removedNodes;
      }

      KRYS_NODISCARD RefPtr<Node> PreviousSibling() const noexcept override
      {
        return _previousSibling;
      }

      KRYS_NODISCARD RefPtr<Node> NextSibling() const noexcept override
      {
        return _nextSibling;
      }
    };
  }
}