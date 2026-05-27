#pragma once

#include "Krystal.HTML/DOM/MutationObserver/EmptyNodeMutationRecord.hpp"

namespace Krys::HTML
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