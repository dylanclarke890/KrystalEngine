#pragma once

#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Serialisation/Access.hpp"
#include "Krystal.Serialisation/Concepts.hpp"

/// This simplified implementation is *heavily* inspired by cereal (https://uscilab.github.io/cereal/).
///
/// The main differences are:
/// - Deterministic overload resolution order (no SFINAE).
/// - Unified API.
/// - Versioning can be done via traits or static member, limited to compile-time constants.
/// - Use of concepts instead of SFINAE.
/// - Optimisations for built-in types.
///
/// Not supported:
/// - Polymorphism.
/// - Smart pointers.
/// - Pointers.
///
/// Dispatch Resolution Order:
///
/// T is an ArchiveBuiltin type -> serialised by archive directly, no need for dispatch.
/// NOTE: ArchiveBuiltin types are always non-versioned.
///
/// Regardless of versioning:
/// Is there a member function (public or private with Access friend class):
///   1.  void Transfer(Archive& ar) noexcept;
///   2a. void Save(Archive& ar) noexcept;
///   2b. void Load(Archive& ar) noexcept;
/// Is there a non-member function (NOTE: uses ADL to look for matches!):
///   1.  void Transfer(Archive& ar, T& value) noexcept;
///   2a. void Save(Archive& ar, const T& value) noexcept
///   2b. void Load(Archive& ar, T& value) noexcept;
///
/// NOTE: If T is versioned, the version is always passed as the last argument to the above methods.
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

  class BaseArchive
  {
    NO_COPY_MOVE(BaseArchive)

  protected:
    BaseArchive() noexcept = default;

  public:
    virtual ~BaseArchive() noexcept = default;
  };

  template <typename Derived>
  class BaseArchiveWriter : public BaseArchive
  {
    NO_COPY_MOVE(BaseArchiveWriter)

  private:
    Derived &_self {*static_cast<Derived *>(this)};

  public:
    BaseArchiveWriter() noexcept = default;

    template <typename T>
    Derived &operator()(T &&value)
    {
      TransferGuard guard(_self, value);

      using DecayedT = RemoveCvRef<T>;

      if constexpr (HasVersionedTransferMember<Derived, DecayedT>)
      {
        Version version = Access::GetVersion<T>(value);
        (*this)(version);
        Access::MemberTransfer(_self, const_cast<DecayedT &>(value), version);
      }
      else if constexpr (HasVersionedSaveMember<Derived, DecayedT>)
      {
        Version version = Access::GetVersion<T>(value);
        (*this)(version);
        Access::MemberSave(_self, value, version);
      }
      else if constexpr (HasVersionedTransferNonMember<Derived, DecayedT>)
      {
        Version version = Access::GetVersion<T>(value);
        (*this)(version);
        Access::NonMemberTransfer(_self, const_cast<DecayedT &>(value), version);
      }
      else if constexpr (HasVersionedSaveNonMember<Derived, DecayedT>)
      {
        Version version = Access::GetVersion<T>(value);
        (*this)(version);
        Access::NonMemberSave(_self, value, version);
      }
      else if constexpr (HasNonVersionedTransferMember<Derived, DecayedT>)
      {
        Access::MemberTransfer(_self, const_cast<DecayedT &>(value));
      }
      else if constexpr (HasNonVersionedSaveMember<Derived, DecayedT>)
      {
        Access::MemberSave(_self, value);
      }
      else if constexpr (HasNonVersionedTransferNonMember<Derived, DecayedT>)
      {
        Access::NonMemberTransfer(_self, const_cast<DecayedT &>(value));
      }
      else if constexpr (HasNonVersionedSaveNonMember<Derived, DecayedT>)
      {
        Access::NonMemberSave(_self, value);
      }
      else
      {
        static_assert(DependentFalse<DecayedT>, "Missing 'Save' or 'Transfer' specialisation for this type.");
      }

      return _self;
    }

    template <ArchiveBuiltin T>
    Derived &operator()(T &&value)
    {
      TransferGuard guard(_self, value);
      _self.Write(value);
      return _self;
    }

    template <Pointer T>
    Derived &operator()(T &&ptr)
    {
      static_assert(DependentFalse<T>, "Pointer types are not supported.");
    }

    template <typename... Types>
    requires(sizeof...(Types) > 1)
    Derived &operator()(Types &&...types)
    {
      ((void)(*this)(std::forward<Types>(types)), ...);
      return _self;
    }
  };

  template <typename Derived>
  class BaseArchiveReader : public BaseArchive
  {
    NO_COPY_MOVE(BaseArchiveReader)

  private:
    Derived &_self {*static_cast<Derived *>(this)};

  public:
    BaseArchiveReader() noexcept = default;

    template <typename T>
    Derived &operator()(T &&value)
    {
      TransferGuard guard(_self, value);

      using DecayedT = RemoveCvRef<T>;

      if constexpr (HasVersionedTransferMember<Derived, DecayedT>)
      {
        Version version {0u};
        (*this)(version);
        Access::MemberTransfer(_self, value, version);
      }
      else if constexpr (HasVersionedLoadMember<Derived, DecayedT>)
      {
        Version version {0u};
        (*this)(version);
        Access::MemberLoad(_self, value, version);
      }
      else if constexpr (HasVersionedTransferNonMember<Derived, DecayedT>)
      {
        Version version {0u};
        (*this)(version);
        Access::NonMemberTransfer(_self, value, version);
      }
      else if constexpr (HasVersionedLoadNonMember<Derived, DecayedT>)
      {
        Version version {0u};
        (*this)(version);
        Access::NonMemberLoad(_self, value, version);
      }
      else if constexpr (HasNonVersionedTransferMember<Derived, DecayedT>)
      {
        Access::MemberTransfer(_self, value);
      }
      else if constexpr (HasNonVersionedLoadMember<Derived, DecayedT>)
      {
        Access::MemberLoad(_self, value);
      }
      else if constexpr (HasNonVersionedTransferNonMember<Derived, DecayedT>)
      {
        Access::NonMemberTransfer(_self, value);
      }
      else if constexpr (HasNonVersionedLoadNonMember<Derived, DecayedT>)
      {
        Access::NonMemberLoad(_self, value);
      }
      else
      {
        static_assert(DependentFalse<DecayedT>, "Missing 'Load' or 'Transfer' specialisation for this type.");
      }

      return _self;
    }

    template <ArchiveBuiltin T>
    Derived &operator()(T &&value)
    {
      TransferGuard guard(_self, value);
      _self.Read(value);
      return _self;
    }

    template <Pointer T>
    Derived &operator()(T &&ptr)
    {
      static_assert(DependentFalse<T>, "Pointer types are not supported.");
    }

    template <typename... Types>
    requires(sizeof...(Types) > 1)
    Derived &operator()(Types &&...types)
    {
      ((void)(*this)(std::forward<Types>(types)), ...);
      return _self;
    }
  };

  template <typename Archive, IsArray T>
  void Transfer(Archive &archive, T &value) noexcept
  {
    using ElementType = RemoveCvRef<RemoveExtent<T>>;
    for (size_t i = 0; i < Extent<T>; i++)
    {
      archive(value[i]);
    }
  }

  template <IsTextArchive Archive, typename T>
  void BeforeTransfer(Archive &archive, const NamedField<T> &field) noexcept
  {
    archive.SetNextFieldName(field.Name);
  }

  template <typename Archive, typename T>
  void Transfer(Archive &archive, NamedField<T> &field) noexcept
  {
    archive(field.Value);
  }

  template <IsBinaryArchive Archive, typename T>
  void Transfer(Archive &archive, ContainerSize<T> &container) noexcept
  {
    archive(container.Size);
  }

  template <IsTextArchive Archive, typename T>
  void Transfer(Archive &, ContainerSize<T> &) noexcept
  {
    // No-op by default.
  }

  template <class Archive, typename TKey, typename TValue>
  void Transfer(Archive &archive, KeyValuePair<TKey, TValue> &pair) noexcept
  {
    archive(CreateNamedField("key", pair.Key), CreateNamedField("value", pair.Value));
  }
}