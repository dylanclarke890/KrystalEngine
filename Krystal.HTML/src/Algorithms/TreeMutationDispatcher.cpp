#include "Krystal.HTML/Algorithms/TreeMutationDispatcher.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/Algorithms/TreeTraversal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/MutationObserver/MutationObserver.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include <cassert>

namespace Krys::HTML
{
  void TreeMutationDispatcher::QueueMutationRecord(
    DOMString &&type, Node &target, Maybe<DOMStringAtom> name, Maybe<DOMStringAtom> namespaceURI,
    Maybe<DOMStringView> oldValue, const SmallNodeList &addedNodes, const SmallNodeList &removedNodes,
    RefPtr<Node> &&previousSibling, RefPtr<Node> &&nextSibling) noexcept
  {
    // TODO(impl): MUTATION-OBSERVERS - Queue a mutation record
    List<int> interestedObservers;

    auto nodes = TreeQueries::InclusiveAncestors(target);
    for (auto &node : nodes)
    {
    }
  }

  void TreeMutationDispatcher::QueueTreeMutationRecord(Node &target, const SmallNodeList &addedNodes,
                                                       const SmallNodeList &removedNodes,
                                                       RefPtr<Node> &&previousSibling,
                                                       RefPtr<Node> &&nextSibling) noexcept
  {
    assert(addedNodes.size() > 0 || removedNodes.size() > 0);
    QueueMutationRecord(u8"childList", target, Null, Null, Null, addedNodes, removedNodes,
                        Krys::Move(previousSibling), Krys::Move(nextSibling));
  }
}
