#pragma once

namespace Krys::Serialisation
{
  struct Access
  {
    template <class Archive, class T>
    requires requires(Archive &ar, T &t) { t.Transfer(ar); }
    static auto Transfer(Archive &ar, T &t) -> decltype(t.Transfer(ar))
    {
      return t.Transfer(ar);
    }

    template <class Archive, class T>
    requires requires(Archive &ar, const T &t) { t.Save(ar); }
    static auto Save(Archive &ar, T const &t) -> decltype(t.Save(ar))
    {
      return t.Save(ar);
    }

    template <class Archive, class T>
    requires requires(Archive &ar, T &t) { t.Load(ar); }
    static auto Load(Archive &ar, T &t) -> decltype(t.Load(ar))
    {
      return t.Load(ar);
    }
  };
}
