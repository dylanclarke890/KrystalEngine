#pragma once

#include "Krystal.Booey/CSS/Calc/RandomCachingKeyMap.hpp"
#include "Krystal.Booey/DOM/Internals/RareData/ParentNodeRareData.hpp"
#include "Krystal.Booey/Style/PseudoElementIdentifier.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class ElementRareData : public ParentNodeRareData
  {
    struct PseudoElementIdentifierHash
    {
      KRYS_NODISCARD size_t operator()(const Maybe<style::PseudoElementIdentifier> &identifier) const noexcept
      {
        return identifier ? DefaultHash<style::PseudoElementIdentifier>::Hash(*identifier) + 1 : 0;
      }
    };

  private:
    HashMap<Maybe<style::PseudoElementIdentifier>, Ref<css::calc::RandomCachingKeyMap>,
            PseudoElementIdentifierHash>
      _randomCachingKeyMaps;

  public:
    /// @brief The `random()` base values of the element (or one of its pseudo-elements).
    KRYS_NODISCARD Ref<css::calc::RandomCachingKeyMap>
      EnsureRandomCachingKeyMap(const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept
    {
      auto it = _randomCachingKeyMaps.find(pseudoElementIdentifier);
      if (it == _randomCachingKeyMaps.end())
      {
        it = _randomCachingKeyMaps.emplace(pseudoElementIdentifier, css::calc::RandomCachingKeyMap::Create())
               .first;
      }
      return it->second;
    }

    KRYS_NODISCARD bool HasRandomCachingKeyMap() const noexcept
    {
      return !_randomCachingKeyMaps.empty();
    }
  };
}
