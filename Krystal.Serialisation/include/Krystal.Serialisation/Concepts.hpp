#pragma once

#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Lib/String.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Serialisation/Access.hpp"
#include "Krystal.Serialisation/Builtins.hpp"

namespace Krys::Serialisation
{
#pragma region Archive Type Concepts

  /// @brief A type that needs to be supported by all archives by default.
  template <typename T>
  concept ArchiveBuiltin =
    Arithmetic<RemoveCvRef<T>> || OneOf<RemoveCvRef<T>, byte, char, uchar, short, ushort, string>;

  /// @brief A special type that provides a name for the next field to be serialised along with its
  /// value.
  template <typename T>
  concept ArchiveNamedField = DerivedFrom<RemoveCvRef<T>, Impl::NamedField>;

  /// @brief A special type that indicates a container is about to be serialised, and provides its size.
  template <typename T>
  concept ArchiveContainerSize = DerivedFrom<RemoveCvRef<T>, Impl::ContainerSize>;

  /// @brief A special type that indicates a key-value pair is being serialised.
  template <typename T>
  concept ArchiveKeyValuePair = DerivedFrom<RemoveCvRef<T>, Impl::KeyValuePair>;

  /// @brief A type that requires custom serialisation methods.
  template <typename T>
  concept ArchiveCustom =
    !ArchiveBuiltin<T> && !ArchiveNamedField<T> && !ArchiveContainerSize<T> && !ArchiveKeyValuePair<T>;

  template <typename T>
  concept ArchiveVersion = SameType<T, Version>;

  template <typename T>
  concept HasTraitVersion = requires { VersionTraits<RemoveCvRef<T>>::ClassVersion; };

  template <typename T>
  concept HasStaticVersion = requires { RemoveCvRef<T>::ClassVersion; };

  template <typename T>
  concept IsVersioned = HasTraitVersion<T> || HasStaticVersion<T>;

#pragma endregion

#pragma region Serialization Method Concepts

  template <typename Archive, typename T>
  concept HasNonVersionedTransferMember =
    requires(Archive &archive, T &value) { Access::MemberTransfer(archive, value); };

  template <typename Archive, typename T>
  concept HasNonVersionedSaveMember =
    requires(Archive &archive, const T &value) { Access::MemberSave(archive, value); };

  template <typename Archive, typename T>
  concept HasNonVersionedLoadMember =
    requires(Archive &archive, T &value) { Access::MemberLoad(archive, value); };

  template <typename Archive, typename T>
  concept HasNonVersionedTransferNonMember =
    requires(Archive &archive, T &value) { Access::NonMemberTransfer(archive, value); };

  template <typename Archive, typename T>
  concept HasNonVersionedSaveNonMember =
    requires(Archive &archive, const T &value) { Access::NonMemberSave(archive, value); };

  template <typename Archive, typename T>
  concept HasNonVersionedLoadNonMember =
    requires(Archive &archive, T &value) { Access::NonMemberLoad(archive, value); };

  template <typename Archive, typename T>
  concept HasVersionedTransferMember = requires(Archive &archive, T &value, const Version version) {
    Access::MemberTransfer(archive, value, version);
  };

  template <typename Archive, typename T>
  concept HasVersionedSaveMember = requires(Archive &archive, const T &value, const Version version) {
    Access::MemberSave(archive, value, version);
  };

  template <typename Archive, typename T>
  concept HasVersionedLoadMember = requires(Archive &archive, T &value, const Version version) {
    Access::MemberLoad(archive, value, version);
  };

  template <typename Archive, typename T>
  concept HasVersionedTransferNonMember = requires(Archive &archive, T &value, const Version version) {
    Access::NonMemberTransfer(archive, value, version);
  };

  template <typename Archive, typename T>
  concept HasVersionedSaveNonMember = requires(Archive &archive, const T &value, const Version version) {
    Access::NonMemberSave(archive, value, version);
  };

  template <typename Archive, typename T>
  concept HasVersionedLoadNonMember = requires(Archive &archive, T &value, const Version version) {
    Access::NonMemberLoad(archive, value, version);
  };

#pragma endregion

#pragma region Archive Traits

  template <typename T>
  struct ArchiveTraits
  {
    static constexpr bool IsWriter = false;
    static constexpr bool IsReader = false;
    static constexpr bool IsBinary = false;
    static constexpr bool IsText = false;
  };

  template <typename Archive>
  concept IsArchiveWriter = ArchiveTraits<Archive>::IsWriter;

  template <typename Archive>
  concept IsArchiveReader = ArchiveTraits<Archive>::IsReader;

  template <typename Archive>
  concept IsArchive = IsArchiveWriter<Archive> || IsArchiveReader<Archive>;

  template <typename Archive>
  concept IsBinaryArchive = ArchiveTraits<Archive>::IsBinary;

  template <typename Archive>
  concept IsTextArchive = ArchiveTraits<Archive>::IsText;

#pragma endregion
}