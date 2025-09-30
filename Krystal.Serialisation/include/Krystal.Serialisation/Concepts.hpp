#pragma once

#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Lib/String.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Serialisation/Access.hpp"

namespace Krys::Serialisation
{
  template <typename T>
  concept ArchiveBuiltin = Arithmetic<T> || SameType<T, byte> || SameType<T, string>;

  template <typename T>
  concept NonArchiveBuiltin = !ArchiveBuiltin<T>;

  template <class Archive, class T>
  concept HasTransferMember = requires(Archive &a, T &t) { Access::Transfer(a, t); }; // T&

  template <class Archive, class T>
  concept HasSaveMember = requires(Archive &a, const T &t) { Access::Save(a, t); }; // const T&

  template <class Archive, class T>
  concept HasLoadMember = requires(Archive &a, T &t) { Access::Load(a, t); }; // T&

  template <class Archive, class T>
  concept HasTransferNonMember = requires(Archive &a, T &t) { Transfer(a, t); }; // T&

  template <class Archive, class T>
  concept HasSaveNonMember = requires(Archive &a, const T &t) { Save(a, t); }; // const T&

  template <class Archive, class T>
  concept HasLoadNonMember = requires(Archive &a, T &t) { Load(a, t); }; // T&
}