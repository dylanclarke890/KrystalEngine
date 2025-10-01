#pragma once

#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Serialisation/Access.hpp"
#include "Krystal.Serialisation/Concepts.hpp"

namespace Krys::Serialisation
{
  /// @brief Hook called before serialisation or deserialisation of a value. No-op by default.
  template <typename Archive, typename T>
  void BeforeTransfer(Archive &, const T &) noexcept
  {
  }

  /// @brief Hook called after serialisation or deserialisation of a value. No-op by default.
  template <typename Archive, typename T>
  void AfterTransfer(Archive &, const T &) noexcept
  {
  }

  /// @brief RAII guard that calls BeforeTransfer on construction and AfterTransfer on destruction.
  template <typename Archive, typename T>
  class TransferGuard
  {
  private:
    Archive &_archive;
    const T &_value;

  public:
    TransferGuard(Archive &archive, const T &value) noexcept : _archive(archive), _value(value)
    {
      BeforeTransfer(_archive, _value);
    }

    ~TransferGuard() noexcept
    {
      AfterTransfer(_archive, _value);
    }
  };

  /// Dispatch Resolution Order:
  /// T is an ArchiveBuiltin type -> serialised by archive directly, no need for overload.
  /// Is there a member function (public or private with Access friend class):
  ///   1.  void Transfer(Archive& ar) noexcept;
  ///   2a. void Save(Archive& ar) noexcept;
  ///   2b. void Load(Archive& ar) noexcept;
  /// Is there a non-member function:
  ///   1.  void Transfer(Archive& ar, T& value) noexcept;
  ///   2a. void Save(Archive& ar, const T& value) noexcept
  ///   2b. void Load(Archive& ar, T& value) noexcept;

  template <typename Archive, typename T>
  void DispatchSave(Archive &archive, const T &value) noexcept
  {
    if constexpr (ArchiveBuiltin<T>)
    {
      archive(value);
    }
    else if constexpr (HasTransferMember<Archive, RemoveConst<T>>)
    {
      // We need to cast away const-ness for these calls because Transfer requires a non-const reference.
      Access::Transfer(archive, const_cast<RemoveConst<T> &>(value));
    }
    else if constexpr (HasSaveMember<Archive, T>)
    {
      Access::Save(archive, value);
    }
    else if constexpr (HasTransferNonMember<Archive, RemoveConst<T>>)
    {
      // We need to cast away const-ness for these calls because Transfer requires a non-const reference.
      Transfer(archive, const_cast<RemoveConst<T> &>(value));
    }
    else if constexpr (HasSaveNonMember<Archive, T>)
    {
      Save(archive, value);
    }
    else
    {
      static_assert(AlwaysFalse<T>::value, "Missing 'Save' or 'Transfer' specialisation for this type.");
    }
  }

  template <typename Archive, typename T>
  void DispatchLoad(Archive &archive, T &value) noexcept
  {
    if constexpr (ArchiveBuiltin<T>)
    {
      archive(value);
    }
    else if constexpr (HasTransferMember<Archive, T>)
    {
      Access::Transfer(archive, value);
    }
    else if constexpr (HasLoadMember<Archive, T>)
    {
      Access::Load(archive, value);
    }
    else if constexpr (HasTransferNonMember<Archive, T>)
    {
      Transfer(archive, value);
    }
    else if constexpr (HasLoadNonMember<Archive, T>)
    {
      Load(archive, value);
    }
    else
    {
      static_assert(AlwaysFalse<T>::value, "Missing 'Load' or 'Transfer' specialisation for this type.");
    }
  }
}