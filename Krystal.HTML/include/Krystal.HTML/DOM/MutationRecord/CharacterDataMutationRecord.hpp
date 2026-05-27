#pragma once

#include "Krystal.HTML/DOM/MutationRecord/EmptyNodeMutationRecord.hpp"

namespace Krys::HTML
{
  class CharacterData;

  class CharacterDataMutationRecord : public EmptyNodeMutationRecord
  {
  public:
    CharacterDataMutationRecord(CharacterData &target, DOMString &&oldValue) noexcept;
  };
}