/*
 * Copyright (C) 2010-2024 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/NeverDestroyed.hpp"
#include "Krystal.Core/Move.hpp"
#include <array>
#include <utility>

namespace krys
{
  /// @brief How a `TinyLRUCache` treats keys and creates values. Specialise or derive to customise.
  template <typename TKey, typename TValue>
  struct TinyLRUCachePolicy
  {
    KRYS_NODISCARD static bool IsKeyNull(const TKey &) noexcept
    {
      return false;
    }

    KRYS_NODISCARD static TValue CreateValueForNullKey() noexcept
    {
      return {};
    }

    KRYS_NODISCARD static TValue CreateValueForKey(const TKey &) noexcept
    {
      return {};
    }

    KRYS_NODISCARD static TKey CreateKeyForStorage(const TKey &key) noexcept
    {
      return key;
    }
  };

  /// @brief A tiny cache of the `TCapacity` most recently used values, searched linearly.
  template <typename TKey, typename TValue, size_t TCapacity = 4,
            typename TPolicy = TinyLRUCachePolicy<TKey, TValue>>
  class TinyLRUCache
  {
    using Entry = std::pair<TKey, TValue>;

  private:
    std::array<Entry, TCapacity> _cacheBuffer;
    size_t _size {0};

  public:
    /// @brief The value for `key`, created by the policy if it isn't cached (evicting the least recently
    /// used).
    KRYS_NODISCARD const TValue &Get(const TKey &key) noexcept
    {
      if (TPolicy::IsKeyNull(key))
      {
        static NeverDestroyed<TValue> valueForNull {TPolicy::CreateValueForNullKey()};
        return *valueForNull;
      }

      for (size_t i = _size; i-- > 0;)
      {
        if (_cacheBuffer[i].first == key)
        {
          if (i < _size - 1)
          {
            // Move entry to the end of the cache if necessary.
            auto entry = krys::move(_cacheBuffer[i]);
            do
            {
              _cacheBuffer[i] = krys::move(_cacheBuffer[i + 1]);
            } while (++i < _size - 1);
            _cacheBuffer[_size - 1] = krys::move(entry);
          }
          return _cacheBuffer[_size - 1].second;
        }
      }

      // _cacheBuffer[0] is the LRU entry, so remove it.
      if (_size == TCapacity)
      {
        for (size_t i = 0; i < _size - 1; ++i)
        {
          _cacheBuffer[i] = krys::move(_cacheBuffer[i + 1]);
        }
      }
      else
      {
        ++_size;
      }

      _cacheBuffer[_size - 1] = Entry {TPolicy::CreateKeyForStorage(key), TPolicy::CreateValueForKey(key)};
      return _cacheBuffer[_size - 1].second;
    }
  };
}
