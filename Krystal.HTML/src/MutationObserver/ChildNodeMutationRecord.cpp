#include "Krystal.HTML/MutationObserver/ChildNodeMutationRecord.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/NodeList/NodeList.hpp"

namespace Krys::HTML
{
  ChildNodeMutationRecord::ChildNodeMutationRecord(ContainerNode &target, Ref<NodeList> &&added,
                                                   Ref<NodeList> &&removed, RefPtr<Node> &&previousSibling,
                                                   RefPtr<Node> &&nextSibling) noexcept
      : _target(ShareRef<Node>(target)), _addedNodes(Krys::Move(added)), _removedNodes(Krys::Move(removed)),
        _previousSibling(Krys::Move(previousSibling)), _nextSibling(Krys::Move(nextSibling))
  {
  }
}
