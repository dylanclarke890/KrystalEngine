#pragma once

#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Lib/String.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Serialisation/Access.hpp"
#include "Krystal.Serialisation/Builtins.hpp"

namespace Krys::Serialisation
{
  /// @brief A type that needs to be supported by all archives by default.
  template <typename T>
  concept ArchiveBuiltin = Arithmetic<RemoveCvRef<T>> || OneOf<RemoveCvRef<T>, byte, string>;

  /// @brief A special type that provides a name for the next field to be serialised along with its
  /// value.
  template <typename T>
  concept ArchiveNamedField = DerivedFrom<RemoveCvRef<T>, Impl::NamedField>;

  /// @brief A special type that indicates a container is about to be serialised, and provides its size.
  template <typename T>
  concept ArchiveContainerSize = DerivedFrom<RemoveCvRef<T>, Impl::ContainerSize>;

  /// @brief A special type that indicates a container is about to be serialised, and provides its size.
  template <typename T>
  concept ArchiveKeyValuePair = DerivedFrom<RemoveCvRef<T>, Impl::KeyValuePair>;

  /// @brief A type that requires custom serialisation methods.
  template <typename T>
  concept ArchiveCustom =
    !ArchiveBuiltin<T> && !ArchiveNamedField<T> && !ArchiveContainerSize<T> && !ArchiveKeyValuePair<T>;

  template <typename Archive, typename T>
  concept HasTransferMember = requires(Archive &archive, T &value) { Access::Transfer(archive, value); };

  template <typename Archive, typename T>
  concept HasSaveMember = requires(Archive &archive, const T &value) { Access::Save(archive, value); };

  template <typename Archive, typename T>
  concept HasLoadMember = requires(Archive &archive, T &value) { Access::Load(archive, value); };

  template <typename Archive, typename T>
  concept HasTransferNonMember = requires(Archive &archive, T &value) { Transfer(archive, value); };

  template <typename Archive, typename T>
  concept HasSaveNonMember = requires(Archive &archive, const T &value) { Save(archive, value); };

  template <typename Archive, typename T>
  concept HasLoadNonMember = requires(Archive &archive, T &value) { Load(archive, value); };
}