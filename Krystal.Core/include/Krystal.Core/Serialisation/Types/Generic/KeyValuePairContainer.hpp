#pragma once

#include "Krystal.Core/Serialisation/Builtins.hpp"
#include "Krystal.Core/Utils/Move.hpp"

namespace krys::serialisation
{
  /// @brief Saving for std-like pair associative containers.
  template <typename Archive, template <typename...> typename TMap, typename... Args,
            typename = typename TMap<Args...>::mapped_type>
  void Save(Archive &archive, const TMap<Args...> &map) noexcept
  {
    archive(KRYS_CONTAINER_SIZE(map.size()));

    for (const auto &i : map)
    {
      archive(KRYS_KEY_VALUE_PAIR(i.first, i.second));
    }
  }

  /// @brief Loading for std-like pair associative containers.
  template <typename Archive, template <typename...> typename TMap, typename... Args,
            typename = typename TMap<Args...>::mapped_type>
  void Load(Archive &archive, TMap<Args...> &map) noexcept
  {
    size_t size;
    archive(KRYS_CONTAINER_SIZE(size));

    map.clear();

    auto hint = map.begin();
    for (size_t i = 0; i < size; i++)
    {
      typename TMap<Args...>::key_type key;
      typename TMap<Args...>::mapped_type value;

      archive(KRYS_KEY_VALUE_PAIR(key, value));
      hint = map.emplace_hint(hint, krys::move(key), krys::move(value));
    }
  }
}
