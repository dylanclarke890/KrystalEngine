#include "Krystal.HTML/DOM/Internals/MutationRecord/CharacterDataMutationRecord.hpp"
#include "Krystal.HTML/DOM/NodeList.hpp"

namespace Krys::HTML
{
  CharacterDataMutationRecord::CharacterDataMutationRecord(Ref<Node> &&target,
                                                           DOMStringAtom oldValue) noexcept
      : EmptyNodeMutationRecord(Krys::Move(target), oldValue)
  {
  }
}
