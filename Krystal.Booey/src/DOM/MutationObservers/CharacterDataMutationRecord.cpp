#include "Krystal.Booey/DOM/MutationObservers/CharacterDataMutationRecord.hpp"
#include "Krystal.Booey/DOM/Collections/NodeList.hpp"

namespace krys::boo::dom
{
  CharacterDataMutationRecord::CharacterDataMutationRecord(Ref<Node> &&target,
                                                           DOMStringAtom oldValue) noexcept
      : EmptyNodeMutationRecord(krys::move(target), oldValue)
  {
  }
}
