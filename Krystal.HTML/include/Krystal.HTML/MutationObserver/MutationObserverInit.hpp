#pragma once

#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::HTML
{
  struct MutationObserverInit
  {
    bool ChildList : 1 {false};
    bool Attributes : 1 {false};
    bool CharacterData : 1 {false};
    bool Subtree : 1 {false};
    bool AttributeOldValue : 1 {false};
    bool CharacterDataOldValue : 1 {false};
    List<DOMString> AttributeFilter;
  };
}