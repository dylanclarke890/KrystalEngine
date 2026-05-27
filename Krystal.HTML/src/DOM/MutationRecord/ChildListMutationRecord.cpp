#include "Krystal.HTML/DOM/MutationRecord/ChildListMutationRecord.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/NodeList.hpp"
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
  ChildListMutationRecord::ChildListMutationRecord(ContainerNode &target, Ref<NodeList> &&added,
                                                   Ref<NodeList> &&removed, RefPtr<Node> &&previousSibling,
                                                   RefPtr<Node> &&nextSibling) noexcept
      : MutationRecord(MutationRecordType::ChildList), _target(ShareRef<Node>(target)),
        _addedNodes(Krys::Move(added)), _removedNodes(Krys::Move(removed)),
        _previousSibling(Krys::Move(previousSibling)), _nextSibling(Krys::Move(nextSibling))
  {
  }
}
