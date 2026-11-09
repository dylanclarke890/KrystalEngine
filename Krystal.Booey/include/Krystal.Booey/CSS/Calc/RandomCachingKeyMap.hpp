/*
 * Copyright (C) 2024-2025 Samuel Weinig <sam@webkit.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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