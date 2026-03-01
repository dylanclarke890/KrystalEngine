#pragma once

#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  struct MutationObserverInit
  {
    bool ChildList {false};
    Maybe<bool> Attributes {false};
    Maybe<bool> CharacterData {false};
    bool Subtree {false};
    Maybe<bool> AttributeOldValue {false};
    Maybe<bool> CharacterDataOldValue {false};
    Maybe<List<DOMStringAtom>> AttributeFilter;
  };
}