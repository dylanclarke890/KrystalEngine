#pragma once

#include <utility>

namespace Krys::Serialisation
{
  template <typename Archive, typename TKey, typename TValue>
  void Save(Archive &archive, const std::pair<TKey, TValue> &pair) noexcept
  {
    archive(pair.first);
    archive(pair.second);
  }

  template <typename Archive, typename TKey, typename TValue>
  void Load(Archive &archive, std::pair<TKey, TValue> &pair) noexcept
  {
    archive(pair.first);
    archive(pair.second);
  }
}