#pragma once

#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Serialisation/Access.hpp"
#include "Krystal.Serialisation/Concepts.hpp"

/// Dispatch Resolution Order:
/// T is an ArchiveBuiltin type -> serialised by archive directly, no need for overload.
/// Is there a member function (public or private with Access friend class):
///   1.  void Transfer(Archive& ar) noexcept;
///   2a. void Save(Archive& ar) noexcept;
///   2b. void Load(Archive& ar) noexcept;
/// Is there a non-member function (NOTE: uses ADL to look for matches!):
///   1.  void Transfer(Archive& ar, T& value) noexcept;
///   2a. void Save(Archive& ar, const T& value) noexcept
///   2b. void Load(Archive& ar, T& value) noexcept;
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
    BaseArchive() = default;

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

    constexpr static bool IsWriter = true;
    constexpr static bool IsReader = false;

    template <typename T>
    Derived &operator()(T &&value) noexcept
    {
      TransferGuard guard(_self, value);

      using RawType = RemoveCvRef<T>;

      if constexpr (ArchiveBuiltin<RawType>)
      {
        _self.Write(value);
      }
      else if constexpr (HasTransferMember<Derived, RawType>)
      {
        // We need to cast away const-ness for these calls because Transfer requires a non-const reference.
        Access::Transfer(_self, const_cast<RemoveConst<RawType> &>(value));
      }
      else if constexpr (HasSaveMember<Derived, RawType>)
      {
        Access::Save(_self, value);
      }
      else if constexpr (HasTransferNonMember<Derived, RawType>)
      {
        // We need to cast away const-ness for these calls because Transfer requires a non-const reference.
        Transfer(_self, const_cast<RawType &>(value));
      }
      else if constexpr (HasSaveNonMember<Derived, RawType>)
      {
        Save(_self, value);
      }
      else
      {
        static_assert(DependentFalse<RawType>, "Missing 'Save' or 'Transfer' specialisation for this type.");
      }

      return _self;
    }

    template <typename... Types>
    requires(sizeof...(Types) > 1)
    Derived &operator()(Types &&...types) noexcept
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

    constexpr static bool IsReader = true;
    constexpr static bool IsWriter = false;

    template <typename T>
    Derived &operator()(T &&value) noexcept
    {
      TransferGuard guard(_self, value);

      using RawType = RemoveCvRef<T>;

      if constexpr (ArchiveBuiltin<RawType>)
      {
        _self.Read(value);
      }
      else if constexpr (HasTransferMember<Derived, RawType>)
      {
        Access::Transfer(_self, value);
      }
      else if constexpr (HasLoadMember<Derived, RawType>)
      {
        Access::Load(_self, value);
      }
      else if constexpr (HasTransferNonMember<Derived, RawType>)
      {
        Transfer(_self, value);
      }
      else if constexpr (HasLoadNonMember<Derived, RawType>)
      {
        Load(_self, value);
      }
      else
      {
        static_assert(DependentFalse<RawType>, "Missing 'Load' or 'Transfer' specialisation for this type.");
      }

      return _self;
    }

    template <typename... Types>
    requires(sizeof...(Types) > 1)
    Derived &operator()(Types &&...types) noexcept
    {
      ((void)(*this)(std::forward<Types>(types)), ...);
      return _self;
    }
  };

  template <typename T>
  concept IsArchiveWriter = DerivedFrom<BaseArchive, T> && requires(T t) { T::IsWriter; };

  template <typename T>
  concept IsArchiveReader = DerivedFrom<BaseArchive, T> && requires(T t) { T::IsReader; };
}