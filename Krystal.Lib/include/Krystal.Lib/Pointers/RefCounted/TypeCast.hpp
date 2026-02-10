#pragma once

#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Pointers/RefCounted/CheckedPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted/CheckedRef.hpp"
#include "Krystal.Lib/Pointers/RefCounted/Ref.hpp"
#include "Krystal.Lib/Pointers/RefCounted/RefPtr.hpp"

namespace Krys
{
#pragma region Ref

  template <typename T, typename U = RawPtrTraits<T>, typename V = DefaultRefDerefTraits<T>, typename X,
            typename Y, typename Z>
  KRYS_NODISCARD constexpr inline Ref<match_constness_t<X, T>, U, V>
    Upcast(const Ref<X, Y, Z> &source) noexcept
  {
    static_assert(!UnnecessaryTypeCast<X, T>, "Unnecessary cast to same type");
    static_assert(CanUpcast<X, T>, "Invalid upcast");
    return ShareRef<match_constness_t<X, T>, U, V>(*static_cast<RawPtr<T>>(source.get()));
  }

  template <typename T, typename U = RawPtrTraits<T>, typename V = DefaultRefDerefTraits<T>, typename X,
            typename Y, typename Z>
  KRYS_NODISCARD constexpr inline Ref<match_constness_t<X, T>, U, V> Upcast(Ref<X, Y, Z> &&source) noexcept
  {
    static_assert(!UnnecessaryTypeCast<X, T>, "Unnecessary cast to same type");
    static_assert(CanUpcast<X, T>, "Invalid upcast");
    return AdoptRef<match_constness_t<X, T>, U, V>(*static_cast<RawPtr<T>>(source.release()));
  }

  template <typename T, typename U = RawPtrTraits<T>, typename V = DefaultRefDerefTraits<T>, typename X,
            typename Y, typename Z>
  KRYS_NODISCARD constexpr inline Ref<match_constness_t<X, T>, U, V>
    Downcast(const Ref<X, Y, Z> &source) noexcept
  {
    static_assert(!UnnecessaryTypeCast<T, X>, "Unnecessary cast to same type");
    static_assert(CanDowncast<X, T>, "Invalid downcast");
    assert(Is<T>(source));
    return ShareRef<match_constness_t<X, T>, U, V>(*static_cast<RawPtr<T>>(source.get()));
  }

  template <typename T, typename U = RawPtrTraits<T>, typename V = DefaultRefDerefTraits<T>, typename X,
            typename Y, typename Z>
  KRYS_NODISCARD constexpr inline Ref<match_constness_t<X, T>, U, V> Downcast(Ref<X, Y, Z> &&source) noexcept
  {
    static_assert(!UnnecessaryTypeCast<T, X>, "Unnecessary cast to same type");
    static_assert(CanDowncast<X, T>, "Invalid downcast");
    assert(Is<T>(source));
    return AdoptRef<match_constness_t<X, T>, U, V>(*static_cast<RawPtr<T>>(source.release()));
  }

  template <typename T, typename U = RawPtrTraits<T>, typename V = DefaultRefDerefTraits<T>, typename X,
            typename Y, typename Z>
  KRYS_NODISCARD constexpr inline RefPtr<match_constness_t<X, T>, U, V>
    DynamicDowncast(const Ref<X, Y, Z> &source) noexcept
  {
    static_assert(!UnnecessaryTypeCast<T, X>, "Unnecessary cast to same type");
    static_assert(CanDowncast<X, T>, "Invalid downcast");

    if (!Is<T>(source))
    {
      return RefPtr<match_constness_t<X, T>, U, V>(nullptr);
    }

    return ShareRefPtr<match_constness_t<X, T>, U, V>(static_cast<RawPtr<T>>(source.get()));
  }

  template <typename T, typename U = RawPtrTraits<T>, typename V = DefaultRefDerefTraits<T>, typename X,
            typename Y, typename Z>
  KRYS_NODISCARD constexpr inline RefPtr<match_constness_t<X, T>, U, V>
    DynamicDowncast(Ref<X, Y, Z> &&source) noexcept
  {
    static_assert(!UnnecessaryTypeCast<T, X>, "Unnecessary cast to same type");
    static_assert(CanDowncast<X, T>, "Invalid downcast");

    if (!Is<T>(source))
    {
      return RefPtr<match_constness_t<X, T>, U, V>(nullptr);
    }

    return AdoptRefPtr<match_constness_t<X, T>, U, V>(static_cast<RawPtr<T>>(source.release()));
  }

#pragma endregion

#pragma region RefPtr

  template <typename T, typename U = RawPtrTraits<T>, typename V = DefaultRefDerefTraits<T>, typename X,
            typename Y, typename Z>
  KRYS_NODISCARD constexpr inline RefPtr<match_constness_t<X, T>, U, V>
    Upcast(const RefPtr<X, Y, Z> &source) noexcept
  {
    static_assert(!UnnecessaryTypeCast<X, T>, "Unnecessary cast to same type");
    static_assert(CanUpcast<X, T>, "Invalid upcast");
    return ShareRefPtr<match_constness_t<X, T>, U, V>(static_cast<RawPtr<T>>(source.get()));
  }

  template <typename T, typename U = RawPtrTraits<T>, typename V = DefaultRefDerefTraits<T>, typename X,
            typename Y, typename Z>
  KRYS_NODISCARD constexpr inline RefPtr<match_constness_t<X, T>, U, V>
    Upcast(RefPtr<X, Y, Z> &&source) noexcept
  {
    static_assert(!UnnecessaryTypeCast<X, T>, "Unnecessary cast to same type");
    static_assert(CanUpcast<X, T>, "Invalid upcast");
    return AdoptRefPtr<match_constness_t<X, T>, U, V>(static_cast<RawPtr<T>>(source.release()));
  }

  template <typename T, typename U = RawPtrTraits<T>, typename V = DefaultRefDerefTraits<T>, typename X,
            typename Y, typename Z>
  KRYS_NODISCARD constexpr inline RefPtr<match_constness_t<X, T>, U, V>
    Downcast(const RefPtr<X, Y, Z> &source) noexcept
  {
    static_assert(!UnnecessaryTypeCast<T, X>, "Unnecessary cast to same type");
    static_assert(CanDowncast<X, T>, "Invalid downcast");
    assert(Is<T>(source));
    return ShareRefPtr<match_constness_t<X, T>, U, V>(*static_cast<RawPtr<T>>(source.get()));
  }

  template <typename T, typename U = RawPtrTraits<T>, typename V = DefaultRefDerefTraits<T>, typename X,
            typename Y, typename Z>
  KRYS_NODISCARD constexpr inline RefPtr<match_constness_t<X, T>, U, V>
    Downcast(RefPtr<X, Y, Z> &&source) noexcept
  {
    static_assert(!UnnecessaryTypeCast<T, X>, "Unnecessary cast to same type");
    static_assert(CanDowncast<X, T>, "Invalid downcast");
    assert(Is<T>(source));
    return AdoptRefPtr<match_constness_t<X, T>, U, V>(*static_cast<RawPtr<T>>(source.release()));
  }

  template <typename T, typename U = RawPtrTraits<T>, typename V = DefaultRefDerefTraits<T>, typename X,
            typename Y, typename Z>
  KRYS_NODISCARD constexpr inline RefPtr<match_constness_t<X, T>, U, V>
    DynamicDowncast(const RefPtr<X, Y, Z> &source) noexcept
  {
    static_assert(!UnnecessaryTypeCast<T, X>, "Unnecessary cast to same type");
    static_assert(CanDowncast<X, T>, "Invalid downcast");

    if (!Is<T>(source))
    {
      return RefPtr<match_constness_t<X, T>, U, V>(nullptr);
    }

    return ShareRefPtr<match_constness_t<X, T>, U, V>(static_cast<RawPtr<T>>(source.get()));
  }

  template <typename T, typename U = RawPtrTraits<T>, typename V = DefaultRefDerefTraits<T>, typename X,
            typename Y, typename Z>
  KRYS_NODISCARD constexpr inline RefPtr<match_constness_t<X, T>, U, V>
    DynamicDowncast(RefPtr<X, Y, Z> &&source) noexcept
  {
    static_assert(!UnnecessaryTypeCast<T, X>, "Unnecessary cast to same type");
    static_assert(CanDowncast<X, T>, "Invalid downcast");

    if (!Is<T>(source))
    {
      return RefPtr<match_constness_t<X, T>, U, V>(nullptr);
    }

    return AdoptRefPtr<match_constness_t<X, T>, U, V>(static_cast<RawPtr<T>>(source.release()));
  }

#pragma endregion

#pragma region CheckedRef

  template <typename T, typename U = RawPtrTraits<T>, typename V = CheckedRefDerefTraits<T>, typename X,
            typename Y, typename Z>
  KRYS_NODISCARD constexpr inline CheckedRef<match_constness_t<X, T>, U, V>
    Upcast(const CheckedRef<X, Y, Z> &source) noexcept
  {
    static_assert(!UnnecessaryTypeCast<X, T>, "Unnecessary cast to same type");
    static_assert(CanUpcast<X, T>, "Invalid upcast");
    return ShareCheckedRef<match_constness_t<X, T>, U, V>(*static_cast<RawPtr<T>>(source.get()));
  }

  template <typename T, typename U = RawPtrTraits<T>, typename V = CheckedRefDerefTraits<T>, typename X,
            typename Y, typename Z>
  KRYS_NODISCARD constexpr inline CheckedRef<match_constness_t<X, T>, U, V>
    Upcast(CheckedRef<X, Y, Z> &&source) noexcept
  {
    static_assert(!UnnecessaryTypeCast<X, T>, "Unnecessary cast to same type");
    static_assert(CanUpcast<X, T>, "Invalid upcast");
    return AdoptCheckedRef<match_constness_t<X, T>, U, V>(*static_cast<RawPtr<T>>(source.release()));
  }

  template <typename T, typename U = RawPtrTraits<T>, typename V = CheckedRefDerefTraits<T>, typename X,
            typename Y, typename Z>
  KRYS_NODISCARD constexpr inline CheckedRef<match_constness_t<X, T>, U, V>
    Downcast(const CheckedRef<X, Y, Z> &source) noexcept
  {
    static_assert(!UnnecessaryTypeCast<T, X>, "Unnecessary cast to same type");
    static_assert(CanDowncast<X, T>, "Invalid downcast");
    assert(Is<T>(source));
    return ShareCheckedRef<match_constness_t<X, T>, U, V>(*static_cast<RawPtr<T>>(source.get()));
  }

  template <typename T, typename U = RawPtrTraits<T>, typename V = CheckedRefDerefTraits<T>, typename X,
            typename Y, typename Z>
  KRYS_NODISCARD constexpr inline CheckedRef<match_constness_t<X, T>, U, V>
    Downcast(CheckedRef<X, Y, Z> &&source) noexcept
  {
    static_assert(!UnnecessaryTypeCast<T, X>, "Unnecessary cast to same type");
    static_assert(CanDowncast<X, T>, "Invalid downcast");
    assert(Is<T>(source));
    return AdoptCheckedRef<match_constness_t<X, T>, U, V>(*static_cast<RawPtr<T>>(source.release()));
  }

  template <typename T, typename U = RawPtrTraits<T>, typename V = CheckedRefDerefTraits<T>, typename X,
            typename Y, typename Z>
  KRYS_NODISCARD constexpr inline CheckedPtr<match_constness_t<X, T>, U, V>
    DynamicDowncast(const CheckedRef<X, Y, Z> &source) noexcept
  {
    static_assert(!UnnecessaryTypeCast<T, X>, "Unnecessary cast to same type");
    static_assert(CanDowncast<X, T>, "Invalid downcast");

    if (!Is<T>(source))
    {
      return CheckedPtr<match_constness_t<X, T>, U, V>(nullptr);
    }

    return ShareCheckedPtr<match_constness_t<X, T>, U, V>(static_cast<RawPtr<T>>(source.get()));
  }

  template <typename T, typename U = RawPtrTraits<T>, typename V = CheckedRefDerefTraits<T>, typename X,
            typename Y, typename Z>
  KRYS_NODISCARD constexpr inline CheckedPtr<match_constness_t<X, T>, U, V>
    DynamicDowncast(CheckedRef<X, Y, Z> &&source) noexcept
  {
    static_assert(!UnnecessaryTypeCast<T, X>, "Unnecessary cast to same type");
    static_assert(CanDowncast<X, T>, "Invalid downcast");

    if (!Is<T>(source))
    {
      return CheckedPtr<match_constness_t<X, T>, U, V>(nullptr);
    }

    return AdoptCheckedPtr<match_constness_t<X, T>, U, V>(static_cast<RawPtr<T>>(source.release()));
  }

#pragma endregion
}