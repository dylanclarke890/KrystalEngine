#include "Krystal.HTML/DOM/MutationRecord/EmptyNodeMutationRecord.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Collections/StaticNodeList.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/CharacterData.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML
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
}
