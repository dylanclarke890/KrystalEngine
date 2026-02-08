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
  template <typename TExpected, typename TArg, bool IsBaseType = BaseOf<TExpected, TArg>>
  struct TypeCastTraits
  {
    static bool IsOfType(TArg &) noexcept
    {
      // If you're hitting this assertion, it is likely because you used is<>() or downcast<>() with a type
      // that doesn't have the needed TypeCastTraits specialization. Please use the following macro to add
      // that specialization: KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN() /
      // KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
      static_assert(Void<TExpected>, "Missing TypeCastTraits specialization");
      return false;
    }
  };

  // Template specialization for the case where TExpected is a base of TArg,
  // so we can return return true unconditionally.
  template <typename TExpected, typename TArg>
  struct TypeCastTraits<TExpected, TArg, true /* IsBaseType */>
  {
    KRYS_NODISCARD static bool IsOfType(TArg &) noexcept
    {
      return true;
    }
  };

  /// @brief Type checking function, to use before casting with downcast<>().
  template <typename TExpected, typename TArg>
  KRYS_NODISCARD inline bool Is(const TArg &source) noexcept
  {
    static_assert(BaseOf<TArg, TExpected>, "Unnecessary type check");
    return TypeCastTraits<const TExpected, const TArg>::IsOfType(source);
  }

  template <typename TExpected, typename TArg>
  KRYS_NODISCARD inline bool Is(TArg *source) noexcept
  {
    static_assert(BaseOf<TArg, TExpected>, "Unnecessary type check");
    return source && TypeCastTraits<const TExpected, const TArg>::IsOfType(*source);
  }

  // Update T's constness to match Reference's.
  template <typename Reference, typename T>
  using match_constness_t = conditional_t<Const<Reference>, add_const_t<T>, remove_const_t<T>>;

  template <typename Target, typename Source>
  inline match_constness_t<Source, Target> &UncheckedDowncast(Source &source) noexcept
  {
    static_assert(!SameType<Source, Target>, "Unnecessary cast to same type");
    static_assert(BaseOf<Source, Target>, "Should be a downcast");
    assert(Is<Target>(source));
    return static_cast<match_constness_t<Source, Target> &>(source);
  }

  template <typename Target, typename Source>
  inline match_constness_t<Source, Target> *UncheckedDowncast(Source *source) noexcept
  {
    static_assert(!SameType<Source, Target>, "Unnecessary cast to same type");
    static_assert(BaseOf<Source, Target>, "Should be a downcast");
    assert(!source || Is<Target>(*source));
    return static_cast<match_constness_t<Source, Target> *>(source);
  }

  template <typename Target, typename Source>
  inline match_constness_t<Source, Target> &Downcast(Source &source) noexcept
  {
    static_assert(!SameType<Source, Target>, "Unnecessary cast to same type");
    static_assert(BaseOf<Source, Target>, "Should be a downcast");
    assert(Is<Target>(source));
    return static_cast<match_constness_t<Source, Target> &>(source);
  }

  template <typename Target, typename Source>
  inline match_constness_t<Source, Target> *Downcast(Source *source) noexcept
  {
    static_assert(!SameType<Source, Target>, "Unnecessary cast to same type");
    static_assert(BaseOf<Source, Target>, "Should be a downcast");
    assert(!source || is<Target>(*source));
    return static_cast<match_constness_t<Source, Target> *>(source);
  }

  template <typename Target, typename Source>
  inline match_constness_t<Source, Target> *DynamicDowncast(Source &source) noexcept
  {
    static_assert(!SameType<Source, Target>, "Unnecessary cast to same type");
    static_assert(BaseOf<Source, Target>, "Should be a downcast");
    return Is<Target>(source) ? &static_cast<match_constness_t<Source, Target> &>(source) : nullptr;
  }

  template <typename Target, typename Source>
  inline match_constness_t<Source, Target> *DynamicDowncast(Source *source) noexcept
  {
    static_assert(!SameType<Source, Target>, "Unnecessary cast to same type");
    static_assert(BaseOf<Source, Target>, "Should be a downcast");
    return Is<Target>(source) ? static_cast<match_constness_t<Source, Target> *>(source) : nullptr;
  }

// Add support for type checking / casting using Is<>() / Downcast<>() helpers for a specific class.
#define KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(ClassName)                                                    \
  namespace Krys                                                                                             \
  {                                                                                                          \
    template <typename TArg>                                                                                 \
    class TypeCastTraits<const ClassName, TArg, false /* IsBaseType */>                                      \
    {                                                                                                        \
    public:                                                                                                  \
      static bool IsOfType(TArg &source) noexcept                                                            \
      {                                                                                                      \
        return IsType(source);                                                                               \
      }                                                                                                      \
                                                                                                             \
    private:

#define KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()                                                               \
  }                                                                                                          \
  ;                                                                                                          \
  }

  // Explicit specialization for C++ standard library types.
  template <typename TExpected, typename TArg, typename Deleter>
  inline bool Is(std::unique_ptr<TArg, Deleter> &source) noexcept
  {
    return Is<TExpected>(source.get());
  }

  template <typename TExpected, typename TArg, typename Deleter>
  inline bool Is(const std::unique_ptr<TArg, Deleter> &source) noexcept
  {
    return Is<TExpected>(source.get());
  }
}
