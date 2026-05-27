#include "Krystal.HTML/DOM/MutationObserver/CharacterDataMutationRecord.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/NodeList.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Node.hpp"

namespace Krys::HTML
{
  CharacterDataMutationRecord::CharacterDataMutationRecord(Ref<Node> &&target,
                                                           DOMStringAtom oldValue) noexcept
      : EmptyNodeMutationRecord(Krys::Move(target), oldValue)
  {
  }
}
