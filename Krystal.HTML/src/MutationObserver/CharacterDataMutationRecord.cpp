#include "Krystal.HTML/MutationObserver/CharacterDataMutationRecord.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/CharacterData.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML
{
  CharacterDataMutationRecord::CharacterDataMutationRecord(CharacterData &target,
                                                           const DOMString &oldValue) noexcept
      : EmptyNodeMutationRecord(target, oldValue)
  {
  }
}