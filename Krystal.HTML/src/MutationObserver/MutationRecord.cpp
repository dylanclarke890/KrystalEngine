#include "Krystal.HTML/MutationObserver/MutationRecord.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/CharacterData.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML::detail
{
  EmptyNodeMutationRecord::EmptyNodeMutationRecord(MutationRecordType type, Node &target,
                                                   DOMString &&oldValue) noexcept
      : MutationRecord(type), _target(ShareRef<Node>(target)), _addedNodes(nullptr), _removedNodes(nullptr),
        _oldValue(Krys::Move(oldValue))
  {
  }

  RefPtr<NodeList> EmptyNodeMutationRecord::AddedNodes() const noexcept
  {
    if (_addedNodes == nullptr)
    {
      _addedNodes = CreateRefPtr<StaticNodeList>();
    }

    return _addedNodes;
  }

  RefPtr<NodeList> EmptyNodeMutationRecord::RemovedNodes() const noexcept
  {
    if (_removedNodes == nullptr)
    {
      _removedNodes = CreateRefPtr<StaticNodeList>();
    }

    return _removedNodes;
  }

  AttributesMutationRecord::AttributesMutationRecord(Element &target, const QualifiedName &name,
                                                     DOMString &&oldValue) noexcept
      : EmptyNodeMutationRecord(MutationRecordType::Attributes, target, Krys::Move(oldValue)),
        _attributeName(name.LocalName), _attributeNamespace(name.NamespaceURI)
  {
  }

  CharacterDataMutationRecord::CharacterDataMutationRecord(CharacterData &target,
                                                           DOMString &&oldValue) noexcept
      : EmptyNodeMutationRecord(MutationRecordType::ChildList, target, Krys::Move(oldValue))
  {
  }

  ChildListMutationRecord::ChildListMutationRecord(ContainerNode &target, Ref<NodeList> &&added,
                                                   Ref<NodeList> &&removed, RefPtr<Node> &&previousSibling,
                                                   RefPtr<Node> &&nextSibling) noexcept
      : MutationRecord(MutationRecordType::ChildList), _target(ShareRef<Node>(target)),
        _addedNodes(Krys::Move(added)), _removedNodes(Krys::Move(removed)),
        _previousSibling(Krys::Move(previousSibling)), _nextSibling(Krys::Move(nextSibling))
  {
  }
}
