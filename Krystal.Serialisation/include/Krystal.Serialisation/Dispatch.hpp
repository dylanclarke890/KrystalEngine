#pragma once

#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Serialisation/Concepts.hpp"

namespace Krys::Serialisation
{
  /// Hook Resolution Order (ADL-discovered):
  /// void Load(Archive& ar, T& value) noexcept;
  /// void Save(Archive& ar, const T& value) noexcept
  /// void Transfer(Archive& ar, T& value) noexcept; NOTE: do not mutate 'value' in custom implementations!

  template <class Archive, class T>
  void DispatchSave(Archive &ar, const T &v) noexcept
  {
    if constexpr (ArchiveBuiltin<T>)
    {
      ar(v);
    }
    else if constexpr (HasNonMemberSave<Archive, T>)
    {
      Save(ar, v);
    }
    else if constexpr (HasNonMemberTransfer<Archive, std::remove_const_t<T>>)
    {
      // We need to cast away const-ness here because Transfer requires a non-const reference.
      // It's safe as long as custom Transfer implementations do not modify the object.
      auto &m = const_cast<RemoveConst<T> &>(v);
      Transfer(ar, m);
    }
    else
    {
      static_assert(false, "Missing 'Save' or 'Transfer' specialisation for this type.");
    }
  }

  template <class Archive, class T>
  void DispatchLoad(Archive &ar, T &v) noexcept
  {
    if constexpr (ArchiveBuiltin<T>)
    {
      ar(v);
    }
    else if constexpr (HasNonMemberLoad<Archive, T>)
    {
      Load(ar, v);
    }
    else if constexpr (HasNonMemberTransfer<Archive, T>)
    {
      Transfer(ar, v);
    }
    else
    {
      static_assert(false, "Missing 'Load' or 'Transfer' specialisation for this type.");
    }
  }
}