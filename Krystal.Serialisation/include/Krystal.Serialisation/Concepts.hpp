#pragma once

#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Lib/String.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::Serialisation
{
  template <typename T>
  concept ArchiveBuiltin = Arithmetic<T> || SameType<T, byte> || SameType<T, string>;

  template <typename T>
  concept NonArchiveBuiltin = !ArchiveBuiltin<T>;

  template <class Archive, class T>
  concept HasMemberSave = requires(Archive &a, const T &t) { t.Save(a); }; // const T&

  template <class Archive, class T>
  concept HasNonMemberSave = requires(Archive &a, const T &t) { Save(a, t); }; // const T&

  template <class Archive, class T>
  concept HasMemberLoad = requires(Archive &a, T &t) { t.Load(a); }; // T&

  template <class Archive, class T>
  concept HasNonMemberLoad = requires(Archive &a, T &t) { Load(a, t); }; // T&

  template <class Archive, class T>
  concept HasMemberTransfer = requires(Archive &a, T &t) { t.Transfer(a); }; // T&

  template <class Archive, class T>
  concept HasNonMemberTransfer = requires(Archive &a, T &t) { Transfer(a, t); }; // T&
}