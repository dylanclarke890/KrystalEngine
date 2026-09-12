#pragma once

#include "Krystal.Core/Types/Pair.hpp"

namespace krys::Serialisation
{
  template <typename Archive, typename TKey, typename TValue>
  void Save(Archive &archive, const Pair<TKey, TValue> &pair) noexcept
  {
    archive(pair.first, pair.second);
  }

  template <typename Archive, typename TKey, typename TValue>
  void Load(Archive &archive, Pair<TKey, TValue> &pair) noexcept
  {
    archive(pair.first, pair.second);
  }
}