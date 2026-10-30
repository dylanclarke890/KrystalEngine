#include "Krystal.Booey/CSS/Counter/CounterStyleRegistry.hpp"
#include "Krystal.Booey/Style/Values/CounterStyles/CounterStyle.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo::css
{
  void CounterStyleRegistry::ResolveUserAgentReferences() noexcept
  {
    for (auto &[name, counter] : UserAgentCounterStyles())
    {
      // decimal counter has no fallback or extended references because it is the last resource for both
      // cases.
      if (counter->Name().View() == u8"decimal")
      {
        continue;
      }
      if (counter->IsFallbackUnresolved())
      {
        ResolveFallbackReference(*counter);
      }
      if (counter->IsExtendsSystem() && counter->IsExtendsUnresolved())
      {
        ResolveExtendsReference(*counter);
      }
    }
  }

  void CounterStyleRegistry::ResolveReferencesIfNeeded() noexcept
  {
    if (!_hasUnresolvedReferences)
    {
      return;
    }

    for (auto &[name, counter] : _authorCounterStyles)
    {
      if (counter->IsFallbackUnresolved())
      {
        ResolveFallbackReference(*counter, &_authorCounterStyles);
      }
      if (counter->IsExtendsSystem() && counter->IsExtendsUnresolved())
      {
        ResolveExtendsReference(*counter, &_authorCounterStyles);
      }
    }
    _hasUnresolvedReferences = false;
  }

  void CounterStyleRegistry::ResolveExtendsReference(CounterStyle &counterStyle,
                                                     CounterStyleMap *map) noexcept
  {
    HashSet<CounterStyle *> countersInChain;
    ResolveExtendsReference(counterStyle, countersInChain, map);
  }

  void CounterStyleRegistry::ResolveExtendsReference(CounterStyle &counter,
                                                     HashSet<CounterStyle *> &countersInChain,
                                                     CounterStyleMap *map) noexcept
  {
    krys_debug_assert(counter.IsExtendsSystem() && counter.IsExtendsUnresolved());
    if (!(counter.IsExtendsSystem() && counter.IsExtendsUnresolved()))
    {
      return;
    }

    if (countersInChain.contains(&counter))
    {
      // Chain of references forms a circle. Treat all as extending decimal
      // (https://www.w3.org/TR/css-counter-styles-3/#extends-system).
      auto decimal = DecimalCounter();
      for (auto *counterInChain : countersInChain)
      {
        krys_debug_assert(counterInChain);
        if (!counterInChain)
        {
          continue;
        }
        counterInChain->ExtendAndResolve(*decimal);
      }
      // Recursion return for circular chain.
      return;
    }
    countersInChain.insert(&counter);

    auto extendedCounter = GetCounterStyle(counter.ExtendsName(), map);
    if (extendedCounter->IsExtendsSystem() && extendedCounter->IsExtendsUnresolved())
    {
      ResolveExtendsReference(*extendedCounter, countersInChain, map);
    }

    // Recursion return for non-circular chain. Calling ResolveExtendsReference() for the extendedCounter
    // might have already resolved this counter style if a circle was formed. If it is still unresolved, it
    // should get resolved here.
    if (counter.IsExtendsUnresolved())
    {
      counter.ExtendAndResolve(*extendedCounter);
    }
  }

  void CounterStyleRegistry::ResolveFallbackReference(CounterStyle &counter, CounterStyleMap *map) noexcept
  {
    counter.SetFallbackReference(GetCounterStyle(counter.FallbackName(), map));
  }

  void CounterStyleRegistry::AddCounterStyle(const CounterStyleDescriptors &descriptors) noexcept
  {
    _hasUnresolvedReferences = true;
    _authorCounterStyles.insert_or_assign(descriptors.Name, CounterStyle::Create(descriptors, false));
  }

  void CounterStyleRegistry::AddUserAgentCounterStyle(const CounterStyleDescriptors &descriptors) noexcept
  {
    UserAgentCounterStyles().insert_or_assign(descriptors.Name, CounterStyle::Create(descriptors, true));
  }

  Ref<CounterStyle> CounterStyleRegistry::DecimalCounter() noexcept
  {
    auto &userAgentCounters = UserAgentCounterStyles();
    auto iterator = userAgentCounters.find(CSSOMStringAtom(u8"decimal"));
    // user agent counter style should always be populated with a counter named decimal if
    // counter-style-at-rule is enabled
    krys_assert(iterator != userAgentCounters.end());
    return iterator->second;
  }

  // A valid map means that the search begins at the author counter style map, otherwise we skip the search to
  // the UA counter styles.
  Ref<CounterStyle> CounterStyleRegistry::GetCounterStyle(const CSSOMStringAtom &name,
                                                          CounterStyleMap *map) noexcept
  {
    if (name.View().empty())
    {
      return DecimalCounter();
    }

    // If there is a map, the search starts from the given map.
    if (map)
    {
      if (auto it = map->find(name); it != map->end())
      {
        return it->second;
      }
    }

    // If there was no map (called for user-agent references resolution), or the counter was not found in the
    // given map, we search at the user-agent map.
    auto &userAgentCounters = UserAgentCounterStyles();
    if (auto it = userAgentCounters.find(name); it != userAgentCounters.end())
    {
      return it->second;
    }

    return DecimalCounter();
  }

  Ref<CounterStyle> CounterStyleRegistry::ResolvedCounterStyle(const CSSOMStringAtom &name) noexcept
  {
    ResolveReferencesIfNeeded();
    return GetCounterStyle(name, &_authorCounterStyles);
  }

  Ref<CounterStyle> CounterStyleRegistry::ResolvedCounterStyle(const style::CounterStyle &style) noexcept
  {
    return ResolvedCounterStyle(style.Identifier.Value);
  }

  CounterStyleMap &CounterStyleRegistry::UserAgentCounterStyles() noexcept
  {
    static NeverDestroyed<CounterStyleMap> counters;
    return counters;
  }

  bool CounterStyleRegistry::operator==(const CounterStyleRegistry &other) const noexcept
  {
    // Intentionally doesn't check _hasUnresolvedReferences.
    if (_authorCounterStyles.size() != other._authorCounterStyles.size())
    {
      return false;
    }
    for (auto &[name, counter] : _authorCounterStyles)
    {
      auto it = other._authorCounterStyles.find(name);
      if (it == other._authorCounterStyles.end() || &*it->second != &*counter)
      {
        return false;
      }
    }
    return true;
  }

  void CounterStyleRegistry::ClearAuthorCounterStyles() noexcept
  {
    if (_authorCounterStyles.empty())
    {
      return;
    }
    _authorCounterStyles.clear();
    Invalidate();
  }

  void CounterStyleRegistry::Invalidate() noexcept
  {
    _hasUnresolvedReferences = true;
  }
}
