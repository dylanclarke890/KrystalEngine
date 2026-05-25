#pragma once

#include "Krystal.HTML/Types/DOMStringAtom.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  struct MutationObserverInit
  {
    bool ChildList {false};
    Maybe<bool> Attributes {};
    Maybe<bool> CharacterData {};
    bool Subtree {false};
    Maybe<bool> AttributeOldValue {};
    Maybe<bool> CharacterDataOldValue {};
    Maybe<List<DOMStringAtom>> AttributeFilter;
  };
}