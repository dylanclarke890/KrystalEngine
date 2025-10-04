#pragma once

#include "Krystal.Serialisation/Builtins.hpp"

namespace Krys::Serialisation
{
  /// @brief Allows archives to use private Transfer/Save/Load methods of types by being a friend. There are
  /// require expressions on each method template that ensures the template is only instantiated if the
  /// requested method actually exists on the target type, allowing the dispatcher to fallback to other
  /// methods as needed.
  class Access
  {
  public:
#pragma region Non-Versioned Member

    template <typename Archive, typename T>
    requires requires(Archive &archive, T &value) { value.Transfer(archive); }
    static auto MemberTransfer(Archive &archive, T &value) noexcept -> decltype(value.Transfer(archive))
    {
      return value.Transfer(archive);
    }

    template <typename Archive, typename T>
    requires requires(Archive &archive, const T &value) { value.Save(archive); }
    static auto MemberSave(Archive &archive, T const &value) noexcept -> decltype(value.Save(archive))
    {
      return value.Save(archive);
    }

    template <typename Archive, typename T>
    requires requires(Archive &archive, T &value) { value.Load(archive); }
    static auto MemberLoad(Archive &archive, T &value) noexcept -> decltype(value.Load(archive))
    {
      return value.Load(archive);
    }

#pragma endregion

#pragma region Non-Versioned Non-Member

    template <typename Archive, typename T>
    requires requires(Archive &archive, T &value) { Transfer(archive, value); }
    static auto NonMemberTransfer(Archive &archive, T &value) noexcept -> decltype(Transfer(archive, value))
    {
      return Transfer(archive, value);
    }

    template <typename Archive, typename T>
    requires requires(Archive &archive, const T &value) { Save(archive, value); }
    static auto NonMemberSave(Archive &archive, T const &value) noexcept -> decltype(Save(archive, value))
    {
      return Save(archive, value);
    }

    template <typename Archive, typename T>
    requires requires(Archive &archive, T &value) { Load(archive, value); }
    static auto NonMemberLoad(Archive &archive, T &value) noexcept -> decltype(Load(archive, value))
    {
      return Load(archive, value);
    }

#pragma endregion

#pragma region Versioned Member

    template <typename Archive, typename T>
    requires requires(Archive &archive, T &value, const Version version) { value.Transfer(archive, version); }
    static auto MemberTransfer(Archive &archive, T &value, const Version version) noexcept
      -> decltype(value.Transfer(archive, version))
    {
      return value.Transfer(archive, version);
    }

    template <typename Archive, typename T>
    requires requires(Archive &archive, const T &value, const Version version) {
      value.Save(archive, version);
    }
    static auto MemberSave(Archive &archive, T const &value, const Version version) noexcept
      -> decltype(value.Save(archive, version))
    {
      return value.Save(archive, version);
    }

    template <typename Archive, typename T>
    requires requires(Archive &archive, T &value, const Version version) { value.Load(archive, version); }
    static auto MemberLoad(Archive &archive, T &value, const Version version) noexcept
      -> decltype(value.Load(archive, version))
    {
      return value.Load(archive, version);
    }

#pragma endregion

#pragma region Versioned Non-Member

    template <typename Archive, typename T>
    requires requires(Archive &archive, T &value, const Version version) {
      Transfer(archive, value, version);
    }
    static auto NonMemberTransfer(Archive &archive, T &value, const Version version) noexcept
      -> decltype(Transfer(archive, value, version))
    {
      return Transfer(archive, value, version);
    }

    template <typename Archive, typename T>
    requires requires(Archive &archive, const T &value, const Version version) {
      Save(archive, value, version);
    }
    static auto NonMemberSave(Archive &archive, T const &value, const Version version) noexcept
      -> decltype(Save(archive, value, version))
    {
      return Save(archive, value, version);
    }

    template <typename Archive, typename T>
    requires requires(Archive &archive, T &value, const Version version) { Load(archive, value, version); }
    static auto NonMemberLoad(Archive &archive, T &value, const Version version) noexcept
      -> decltype(Load(archive, value, version))
    {
      return Load(archive, value, version);
    }

#pragma endregion
  };
}
