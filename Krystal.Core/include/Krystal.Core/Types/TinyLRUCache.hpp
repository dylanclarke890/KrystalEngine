#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"
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
    /// @brief The value for `key`, created by the policy if it isn't cached (evicting the least recently used).
    KRYS_NODISCARD const TValue &Get(const TKey &key) noexcept
    {
      if (TPolicy::IsKeyNull(key))
      {
        static NeverDestroyed<TValue> valueForNull {TPolicy::CreateValueForNullKey()};
        return valueForNull.get();
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
