#pragma once

#include "Krystal.Booey/DOM/Internals/MutationRecord/EmptyNodeMutationRecord.hpp"

namespace krys::boo::dom
{
  class CharacterDataMutationRecord final : public EmptyNodeMutationRecord
  {
  public:
    CharacterDataMutationRecord(Ref<Node> &&target, DOMStringAtom oldValue) noexcept;

    KRYS_NODISCARD DOMStringAtom Type() const noexcept override
    {
      return u8"characterData";
    }
  };
}