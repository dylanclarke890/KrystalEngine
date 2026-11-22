#pragma once

#include "Krystal.Core/Move.hpp"
#include "Krystal.Core/Serialisation/Builtins.hpp"

namespace krys::serialisation
{
  /// @brief A std-like associative container of key-value pairs.
  template <typename T>
  concept KeyValuePairContainer = requires {
    typename T::key_type;
    typename T::mapped_type;
  };

  /// @brief Saving for std-like pair associative containers.
  template <typename Archive, KeyValuePairContainer TMap>
  void Save(Archive &archive, const TMap &map) noexcept
  {
    archive(KRYS_CONTAINER_SIZE(map.size()));

    for (const auto &i : map)
    {
      archive(KRYS_KEY_VALUE_PAIR(i.first, i.second));
    }
  }

  /// @brief Loading for std-like pair associative containers.
  template <typename Archive, KeyValuePairContainer TMap>
  void Load(Archive &archive, TMap &map) noexcept
  {
    size_t size;
    archive(KRYS_CONTAINER_SIZE(size));

    map.clear();
    map.reserve(size);

    for (size_t i = 0; i < size; i++)
    {
      typename TMap::key_type key;
      typename TMap::mapped_type value;

      archive(KRYS_KEY_VALUE_PAIR(key, value));
      map.emplace(krys::move(key), krys::move(value));
    }
  }
}
