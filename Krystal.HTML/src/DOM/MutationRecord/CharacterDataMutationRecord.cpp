#include "Krystal.HTML/DOM/MutationRecord/CharacterDataMutationRecord.hpp"
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
  CharacterDataMutationRecord::CharacterDataMutationRecord(CharacterData &target,
                                                           DOMString &&oldValue) noexcept
      : EmptyNodeMutationRecord(MutationRecordType::ChildList, target, Krys::Move(oldValue))
  {
  }
}
