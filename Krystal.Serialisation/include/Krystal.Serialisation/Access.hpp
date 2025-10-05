#pragma once

#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Serialisation/Builtins.hpp"

namespace Krys::Serialisation
{
#define KRYS_ACCESS_FRIEND() friend class Krys::Serialisation::Access

  /// @brief Allows archives to use private Transfer/Save/Load methods of types by being a friend.
  class Access
  {
  public:
    template <typename T>
    static auto GetVersion(T &&value) noexcept -> Version
    {
      if constexpr (HasTraitVersion<T>)
      {
        return VersionTraits<RemoveCvRef<T>>::ClassVersion;
      }
      else if constexpr (requires(RemoveCvRef<T> &value) { value.GetVersion(); })
      {
        return value.GetVersion();
      }
      else
      {
        static_assert(DependentFalse<T>, "Missing version information for type.");
      }
    }

#pragma region Non-Versioned Member

    template <typename Archive, typename T>
    static auto MemberTransfer(Archive &archive, T &value) noexcept -> decltype(value.Transfer(archive))
    {
      return value.Transfer(archive);
    }

    template <typename Archive, typename T>
    static auto MemberSave(Archive &archive, const T &value) noexcept -> decltype(value.Save(archive))
    {
      return value.Save(archive);
    }

    template <typename Archive, typename T>
    static auto MemberLoad(Archive &archive, T &value) noexcept -> decltype(value.Load(archive))
    {
      return value.Load(archive);
    }

#pragma endregion

#pragma region Non-Versioned Non-Member

    template <typename Archive, typename T>
    static auto NonMemberTransfer(Archive &archive, T &value) noexcept -> decltype(Transfer(archive, value))
    {
      return Transfer(archive, value);
    }

    template <typename Archive, typename T>
    static auto NonMemberSave(Archive &archive, const T &value) noexcept -> decltype(Save(archive, value))
    {
      return Save(archive, value);
    }

    template <typename Archive, typename T>
    static auto NonMemberLoad(Archive &archive, T &value) noexcept -> decltype(Load(archive, value))
    {
      return Load(archive, value);
    }

#pragma endregion

#pragma region Versioned Member

    template <typename Archive, typename T>
    static auto MemberTransfer(Archive &archive, T &value, Version version) noexcept
      -> decltype(value.Transfer(archive, version))
    {
      return value.Transfer(archive, version);
    }

    template <typename Archive, typename T>
    static auto MemberSave(Archive &archive, const T &value, Version version) noexcept
      -> decltype(value.Save(archive, version))
    {
      return value.Save(archive, version);
    }

    template <typename Archive, typename T>
    static auto MemberLoad(Archive &archive, T &value, Version version) noexcept
      -> decltype(value.Load(archive, version))
    {
      return value.Load(archive, version);
    }

#pragma endregion

#pragma region Versioned Non-Member

    template <typename Archive, typename T>
    static auto NonMemberTransfer(Archive &archive, T &value, Version version) noexcept
      -> decltype(Transfer(archive, value, version))
    {
      return Transfer(archive, value, version);
    }

    template <typename Archive, typename T>
    static auto NonMemberSave(Archive &archive, const T &value, Version version) noexcept
      -> decltype(Save(archive, value, version))
    {
      return Save(archive, value, version);
    }

    template <typename Archive, typename T>
    static auto NonMemberLoad(Archive &archive, T &value, Version version) noexcept
      -> decltype(Load(archive, value, version))
    {
      return Load(archive, value, version);
    }

#pragma endregion
  };
}
