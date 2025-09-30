#pragma once

#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Serialisation/Access.hpp"
#include "Krystal.Serialisation/Concepts.hpp"

namespace Krys::Serialisation
{
  /// Dispatch Resolution Order:
  /// Is T an ArchiveBuiltin type?
  /// Is there a member function:
  ///   1.  void value.Transfer(Archive& ar) noexcept;
  ///   2a. void value.Save(Archive& ar) noexcept;
  ///   2b. void value.Load(Archive& ar) noexcept;
  /// Is there a non-member function:
  ///   1.  void Transfer(Archive& ar, T& value) noexcept;
  ///   2a. void Save(Archive& ar, const T& value) noexcept
  ///   2b. void Load(Archive& ar, T& value) noexcept;

  template <class Archive, class T>
  void DispatchSave(Archive &ar, const T &v) noexcept
  {
    if constexpr (ArchiveBuiltin<T>)
    {
      ar(v);
    }
    else if constexpr (HasTransferMember<Archive, RemoveConst<T>>)
    {
      // We need to cast away const-ness for these calls because Transfer requires a non-const reference.
      Access::Transfer(ar, const_cast<RemoveConst<T> &>(v));
    }
    else if constexpr (HasTransferNonMember<Archive, RemoveConst<T>>)
    {
      // We need to cast away const-ness for these calls because Transfer requires a non-const reference.
      Transfer(ar, const_cast<RemoveConst<T> &>(v));
    }
    else if constexpr (HasSaveMember<Archive, T>)
    {
      Access::Save(ar, v);
    }
    else if constexpr (HasSaveNonMember<Archive, T>)
    {
      Save(ar, v);
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
    else if constexpr (HasTransferMember<Archive, T>)
    {
      Access::Transfer(ar, v);
    }
    else if constexpr (HasTransferNonMember<Archive, T>)
    {
      Transfer(ar, v);
    }
    else if constexpr (HasLoadMember<Archive, T>)
    {
      Access::Load(ar, v);
    }
    else if constexpr (HasLoadNonMember<Archive, T>)
    {
      Load(ar, v);
    }
    else
    {
      static_assert(false, "Missing 'Load' or 'Transfer' specialisation for this type.");
    }
  }
}