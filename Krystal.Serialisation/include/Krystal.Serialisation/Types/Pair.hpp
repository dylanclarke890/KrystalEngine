#pragma once

#include "Krystal.Lib/Types/Pair.hpp"

namespace Krys::Serialisation
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