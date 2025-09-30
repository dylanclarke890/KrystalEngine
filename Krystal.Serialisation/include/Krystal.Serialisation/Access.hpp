#pragma once

namespace Krys::Serialisation
{
  class Access
  {
  public:
    template <typename Archive, typename T>
    requires requires(Archive &archive, T &value) { value.Transfer(archive); }
    static auto Transfer(Archive &archive, T &value) -> decltype(value.Transfer(archive))
    {
      return value.Transfer(archive);
    }

    template <typename Archive, typename T>
    requires requires(Archive &archive, const T &value) { value.Save(archive); }
    static auto Save(Archive &archive, T const &value) -> decltype(value.Save(archive))
    {
      return value.Save(archive);
    }

    template <typename Archive, typename T>
    requires requires(Archive &archive, T &value) { value.Load(archive); }
    static auto Load(Archive &archive, T &value) -> decltype(value.Load(archive))
    {
      return value.Load(archive);
    }
  };
}
