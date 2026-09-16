#include "Krystal.Booey/DOM/Internals/MutationRecord/CharacterDataMutationRecord.hpp"
#include "Krystal.Booey/DOM/NodeList.hpp"

namespace krys::boo::dom
{
  CharacterDataMutationRecord::CharacterDataMutationRecord(Ref<Node> &&target,
                                                           DOMStringAtom oldValue) noexcept
      : EmptyNodeMutationRecord(krys::move(target), oldValue)
  {
  }
}
