#pragma once

#include "Krystal.Booey/DOM/Observers/Mutation/EmptyNodeMutationRecord.hpp"

namespace krys::boo::dom
{
  class CharacterDataMutationRecord final : public EmptyNodeMutationRecord
  {
  public:
    CharacterDataMutationRecord(Ref<Node> &&target, DOMStringAtom oldValue) noexcept;

    KRYS_NODISCARD const DOMStringAtom &Type() const noexcept override
    {
      static const DOMStringAtom type {u8"characterData"};
      return type;
    }
  };
}
