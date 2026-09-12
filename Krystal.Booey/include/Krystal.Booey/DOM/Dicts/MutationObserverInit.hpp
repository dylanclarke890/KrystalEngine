#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::dom
{
  /// @see https://dom.spec.whatwg.org/#dictdef-mutationobserverinit
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