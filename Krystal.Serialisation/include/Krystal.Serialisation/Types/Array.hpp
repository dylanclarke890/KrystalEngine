#pragma once

#include <array>

namespace Krys::Serialisation
{
  template <typename Archive, typename T, size_t N>
  void Save(Archive &archive, const std::array<T, N> &arr) noexcept
  {
    size_t size = arr.size();
    archive(size);

    for (const auto &item : arr)
      archive(item);
  }

  template <typename Archive, typename T, size_t N>
  void Load(Archive &archive, std::array<T, N> &arr) noexcept
  {
    size_t size {};
    archive(size);

    for (auto &item : arr)
      archive(item);
  }
}