#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include <cassert>
#include <cstddef>
#include <memory>
#include <utility>

namespace Krys
{
  template <typename Target, typename Source>
  struct TypeCastTraits
  {
    KRYS_NODISCARD constexpr static bool IsOfType(const Source &) noexcept
    {
      // If you're hitting this assertion, it is likely because you used Is<>() or Downcast<>() etc. with a
      // type that doesn't have the needed TypeCastTraits specialization. Use the following macros to add it:
      // KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN() / KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
      static_assert(Void<Target>, "Missing TypeCastTraits specialization");
      return false;
    }
  };

  /// @brief Returns T with the same constness as TRef.
  template <typename TRef, typename T>
  using match_constness_t = conditional_t<IsConst<remove_ref_t<TRef>>, add_const_t<T>, remove_const_t<T>>;

  template <typename Target, typename Source>
  concept UnnecessaryTypeCast = SameType<remove_const_t<Target>, remove_const_t<Source>>;

  template <typename TDerived, typename TBase>
  concept CanUpcast = DerivedFrom<remove_const_t<TDerived>, remove_const_t<TBase>>;

  template <typename TBase, typename TDerived>
  concept CanDowncast = BaseOf<remove_const_t<TBase>, remove_const_t<TDerived>>;

  template <typename Target, typename Source>
  KRYS_NODISCARD constexpr inline bool Is(const Source &source) noexcept
  {
    static_assert(CanDowncast<Source, Target>, "Unrelated types");

    if constexpr (SameType<Target, Source>)
    {
      return true;
    }

    return TypeCastTraits<const Target, const Source>::IsOfType(source);
  }

  template <typename Target, typename Source>
  KRYS_NODISCARD constexpr inline bool Is(Source *source) noexcept
  {
    static_assert(CanDowncast<Source, Target>, "Unrelated types");

    if constexpr (SameType<Target, Source>)
    {
      return true;
    }

    return source && TypeCastTraits<const Target, const Source>::IsOfType(*source);
  }

  template <typename... Ts, typename Source>
  KRYS_NODISCARD constexpr inline bool IsOneOf(const Source &source) noexcept
  {
    return (Is<Ts>(source) || ...);
  }

  template <typename Target, typename Source>
  KRYS_NODISCARD constexpr inline decltype(auto) Downcast(Source &source) noexcept
  {
    static_assert(!UnnecessaryTypeCast<Source, Target>, "Unnecessary cast to same type");
    static_assert(CanDowncast<Source, Target>, "Invalid downcast");
    assert(Is<Target>(source));
    return static_cast<match_constness_t<Source, Target> &>(source);
  }

  template <typename Target, typename Source>
  KRYS_NODISCARD constexpr inline decltype(auto) Downcast(Source *source) noexcept
  {
    static_assert(!UnnecessaryTypeCast<Source, Target>, "Unnecessary cast to same type");
    static_assert(CanDowncast<Source, Target>, "Invalid downcast");
    assert(!source || Is<Target>(*source));
    return static_cast<match_constness_t<Source, Target> *>(source);
  }

  template <typename Target, typename Source>
  KRYS_NODISCARD constexpr inline decltype(auto) DynamicDowncast(Source &source) noexcept
  {
    static_assert(!UnnecessaryTypeCast<Source, Target>, "Unnecessary cast to same type");
    static_assert(CanDowncast<Source, Target>, "Invalid downcast");
    return Is<Target>(source) ? static_cast<match_constness_t<Source, Target> *>(&source) : nullptr;
  }

  template <typename Target, typename Source>
  KRYS_NODISCARD constexpr inline decltype(auto) DynamicDowncast(Source *source) noexcept
  {
    static_assert(!UnnecessaryTypeCast<Source, Target>, "Unnecessary cast to same type");
    static_assert(CanDowncast<Source, Target>, "Invalid downcast");
    return Is<Target>(source) ? static_cast<match_constness_t<Source, Target> *>(source) : nullptr;
  }

/// @brief Adds support for the type cast methods for a type.
#define KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Target)                                                       \
  namespace Krys                                                                                             \
  {                                                                                                          \
    template <typename Source>                                                                               \
    class TypeCastTraits<const Target, Source>                                                               \
    {                                                                                                        \
    public:                                                                                                  \
      KRYS_NODISCARD constexpr static bool IsOfType(Source &source) noexcept                                 \
      {                                                                                                      \
        return IsType(source);                                                                               \
      }                                                                                                      \
                                                                                                             \
    private:

#define KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()                                                               \
  }                                                                                                          \
  ;                                                                                                          \
  }

/// @brief Helper macro to allow the type cast traits specialization to access private members of the type.
/// Use this in the class definition of the type for which you are specializing TypeCastTraits.
#define KRYS_TYPE_CAST_TRAITS_ACCESS()                                                                       \
  template <typename, typename>                                                                              \
  friend struct Krys::TypeCastTraits
}
