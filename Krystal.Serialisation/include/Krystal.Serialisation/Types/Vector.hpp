#pragma once

#include <vector>

namespace Krys::Serialisation
{
  template <typename Archive, typename T>
  void Save(Archive &archive, const std::vector<T> &vec) noexcept
  {
    size_t size = vec.size();
    archive(size);

    for (const auto &item : vec)
      archive(item);
  }

  template <typename Archive, typename T>
  void Load(Archive &archive, std::vector<T> &vec) noexcept
  {
    size_t size {};
    archive(size);

    vec.resize(size);
    for (auto &item : vec)
      archive(item);
  }
}