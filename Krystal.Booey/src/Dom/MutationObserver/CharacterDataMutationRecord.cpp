#include "Krystal.Booey/Dom/MutationObserver/CharacterDataMutationRecord.hpp"

namespace krys::boo::dom
{
  CharacterDataMutationRecord::CharacterDataMutationRecord(Ref<Node> &&target,
                                                           DOMStringAtom oldValue) noexcept
      : EmptyNodeMutationRecord(krys::move(target), krys::move(oldValue))
  {
  }
}
