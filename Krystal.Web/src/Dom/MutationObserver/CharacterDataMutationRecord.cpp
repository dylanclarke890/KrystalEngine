#include "Krystal.Web/Dom/MutationObserver/CharacterDataMutationRecord.hpp"

namespace krys::web::dom
{
  CharacterDataMutationRecord::CharacterDataMutationRecord(Ref<Node> &&target,
                                                           DOMStringAtom oldValue) noexcept
      : EmptyNodeMutationRecord(krys::move(target), krys::move(oldValue))
  {
  }
}
