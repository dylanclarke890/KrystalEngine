#include "Krystal.HTML/MutationObserver/CharacterDataMutationRecord.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/CharacterData.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/NodeList/NodeList.hpp"

namespace Krys::HTML
{
  CharacterDataMutationRecord::CharacterDataMutationRecord(CharacterData &target,
                                                           const DOMString &oldValue) noexcept
      : EmptyNodeMutationRecord(target, oldValue)
  {
  }
}