#pragma once

#include "Krystal.HTML/MutationObserver/detail/EmptyNodeMutationRecord.hpp"

namespace Krys::HTML
{
  class CharacterData;

  class CharacterDataMutationRecord : public ::Krys::HTML::detail::EmptyNodeMutationRecord
  {
  public:
    CharacterDataMutationRecord(CharacterData &target, const DOMString &oldValue) noexcept;

    KRYS_NODISCARD const StringAtom &Type() const noexcept override
    {
      static const StringAtom type = u8"characterData";
      return type;
    }
  };
}