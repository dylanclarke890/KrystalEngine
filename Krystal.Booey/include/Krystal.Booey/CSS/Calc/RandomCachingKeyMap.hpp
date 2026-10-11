#pragma once

#include "Krystal.Booey/CSS/Calc/RandomCachingKey.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include <random>

namespace krys::boo::css::calc
{
  class RandomCachingKeyMap final : public RefCounted<RandomCachingKeyMap>
  {
  private:
    HashMap<RandomCachingKey, double, RandomCachingKeyHash> _map;

    RandomCachingKeyMap() noexcept = default;

  public:
    KRYS_NODISCARD static Ref<RandomCachingKeyMap> Create() noexcept
    {
      return AdoptRef(*new RandomCachingKeyMap);
    }

    KRYS_NODISCARD double LookupCSSRandomBaseValue(const RandomCachingKey &key) noexcept
    {
      if (auto it = _map.find(key); it != _map.end())
      {
        return it->second;
      }

      // TODO: probably want to do this better but works for now.
      std::random_device rd;
      std::mt19937 gen {rd()};
      std::uniform_real_distribution<double> dis(0.0, 1.0);

      auto next = dis(gen);
      _map[key] = next;

      return next;
    }
  };
}