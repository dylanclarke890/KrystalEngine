#pragma once

#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Serialisation/Builtins.hpp"

namespace Krys::Serialisation
{
  template <typename Archive, typename T>
  void Save(Archive &archive, const List<T> &value) noexcept
  {
    size_t size = value.size();
    archive(KRYS_CONTAINER_SIZE(size));

    for (const auto &item : value)
    {
      archive(item);
    }
  }

  template <typename Archive, typename T>
  void Load(Archive &archive, List<T> &value) noexcept
  {
    size_t size {0u};
    archive(KRYS_CONTAINER_SIZE(size));

    value.resize(size);
    for (auto &item : value)
    {
      archive(item);
    }
  }
}