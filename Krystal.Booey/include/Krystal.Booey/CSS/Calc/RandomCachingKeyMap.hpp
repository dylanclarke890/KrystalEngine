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

      // FIXME: Upstream uses a cryptographically random unit interval (noting it probably doesn't need to
      // be); a per-thread Mersenne Twister is enough here.
      thread_local std::mt19937_64 generator {std::random_device {}()};
      std::uniform_real_distribution<double> distribution {0.0, 1.0};

      return _map.emplace(key, distribution(generator)).first->second;
    }
  };
}