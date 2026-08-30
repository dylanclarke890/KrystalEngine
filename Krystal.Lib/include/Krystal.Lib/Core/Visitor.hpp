#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/Variant.hpp"
#include <utility>

namespace Krys
{
  /// @see http://stackoverflow.com/questions/25338795/is-there-a-name-for-this-tuple-creation-idiom
  template <typename A, typename... B>
  struct Visitor : Visitor<A>, Visitor<B...>
  {
    Visitor(A a, B... b) noexcept : Visitor<A>(a), Visitor<B...>(b...)
    {
    }

    using Visitor<A>::operator();
    using Visitor<B...>::operator();
  };

  template <typename A>
  struct Visitor<A> : A
  {
    Visitor(A a) noexcept : A(a)
    {
    }

    using A::operator();
  };

  template <typename... F>
  KRYS_ALWAYS_INLINE Visitor<F...> CreateVisitor(F... f) noexcept
  {
    return Visitor<F...>(f...);
  }

  // Macros to implement switching over an integer range in chunks of 32.
  // Useful for efficient implementations of variant and tuple type visiting.
  // Adapted from https://www.reddit.com/r/cpp/comments/kst2pu/comment/giilcxv/.

#define KRYS_UNROLLED_CASE_VISIT_SWITCH_CASE(CASE, Min, Max, N)                                              \
  case Min + N:                                                                                              \
  {                                                                                                          \
    if constexpr (Min + N < Max)                                                                             \
    {                                                                                                        \
      return CASE(Min, Max, N);                                                                              \
    }                                                                                                        \
    else                                                                                                     \
    {                                                                                                        \
      std::unreachable();                                                                                    \
    }                                                                                                        \
  }

#define KRYS_UNROLLED_32_CASE_VISIT_SWITCH(INDEX, MIN, MAX, CASE, NEXT)                                      \
  switch (INDEX)                                                                                             \
  {                                                                                                          \
    KRYS_UNROLLED_CASE_VISIT_SWITCH_CASE(CASE, MIN, MAX, 0)                                                  \
    KRYS_UNROLLED_CASE_VISIT_SWITCH_CASE(CASE, MIN, MAX, 1)                                                  \
    KRYS_UNROLLED_CASE_VISIT_SWITCH_CASE(CASE, MIN, MAX, 2)                                                  \
    KRYS_UNROLLED_CASE_VISIT_SWITCH_CASE(CASE, MIN, MAX, 3)                                                  \
    KRYS_UNROLLED_CASE_VISIT_SWITCH_CASE(CASE, MIN, MAX, 4)                                                  \
    KRYS_UNROLLED_CASE_VISIT_SWITCH_CASE(CASE, MIN, MAX, 5)                                                  \
    KRYS_UNROLLED_CASE_VISIT_SWITCH_CASE(CASE, MIN, MAX, 6)                                                  \
    KRYS_UNROLLED_CASE_VISIT_SWITCH_CASE(CASE, MIN, MAX, 7)                                                  \
    KRYS_UNROLLED_CASE_VISIT_SWITCH_CASE(CASE, MIN, MAX, 8)                                                  \
    KRYS_UNROLLED_CASE_VISIT_SWITCH_CASE(CASE, MIN, MAX, 9)                                                  \
    KRYS_UNROLLED_CASE_VISIT_SWITCH_CASE(CASE, MIN, MAX, 10)                                                 \
    KRYS_UNROLLED_CASE_VISIT_SWITCH_CASE(CASE, MIN, MAX, 11)                                                 \
    KRYS_UNROLLED_CASE_VISIT_SWITCH_CASE(CASE, MIN, MAX, 12)                                                 \
    KRYS_UNROLLED_CASE_VISIT_SWITCH_CASE(CASE, MIN, MAX, 13)                                                 \
    KRYS_UNROLLED_CASE_VISIT_SWITCH_CASE(CASE, MIN, MAX, 14)                                                 \
    KRYS_UNROLLED_CASE_VISIT_SWITCH_CASE(CASE, MIN, MAX, 15)                                                 \
    KRYS_UNROLLED_CASE_VISIT_SWITCH_CASE(CASE, MIN, MAX, 16)                                                 \
    KRYS_UNROLLED_CASE_VISIT_SWITCH_CASE(CASE, MIN, MAX, 17)                                                 \
    KRYS_UNROLLED_CASE_VISIT_SWITCH_CASE(CASE, MIN, MAX, 18)                                                 \
    KRYS_UNROLLED_CASE_VISIT_SWITCH_CASE(CASE, MIN, MAX, 19)                                                 \
    KRYS_UNROLLED_CASE_VISIT_SWITCH_CASE(CASE, MIN, MAX, 20)                                                 \
    KRYS_UNROLLED_CASE_VISIT_SWITCH_CASE(CASE, MIN, MAX, 21)                                                 \
    KRYS_UNROLLED_CASE_VISIT_SWITCH_CASE(CASE, MIN, MAX, 22)                                                 \
    KRYS_UNROLLED_CASE_VISIT_SWITCH_CASE(CASE, MIN, MAX, 23)                                                 \
    KRYS_UNROLLED_CASE_VISIT_SWITCH_CASE(CASE, MIN, MAX, 24)                                                 \
    KRYS_UNROLLED_CASE_VISIT_SWITCH_CASE(CASE, MIN, MAX, 25)                                                 \
    KRYS_UNROLLED_CASE_VISIT_SWITCH_CASE(CASE, MIN, MAX, 26)                                                 \
    KRYS_UNROLLED_CASE_VISIT_SWITCH_CASE(CASE, MIN, MAX, 27)                                                 \
    KRYS_UNROLLED_CASE_VISIT_SWITCH_CASE(CASE, MIN, MAX, 28)                                                 \
    KRYS_UNROLLED_CASE_VISIT_SWITCH_CASE(CASE, MIN, MAX, 29)                                                 \
    KRYS_UNROLLED_CASE_VISIT_SWITCH_CASE(CASE, MIN, MAX, 30)                                                 \
    KRYS_UNROLLED_CASE_VISIT_SWITCH_CASE(CASE, MIN, MAX, 31)                                                 \
  }                                                                                                          \
                                                                                                             \
  constexpr auto nextMin = std::min(MIN + 32, MAX);                                                          \
  if constexpr (nextMin < MAX)                                                                               \
    return NEXT(nextMin, MAX);                                                                               \
  std::unreachable();

  template <size_t Minimum, size_t Maximum, class F>
  KRYS_NODISCARD KRYS_ALWAYS_INLINE decltype(auto) VisitAtIndex(size_t index, F &&f) noexcept
  {
#define KRYS_INDEX_VISIT_CASE(Min, Max, N) f.template operator()<Min + N>()
#define KRYS_INDEX_VISIT_NEXT(Min, Max) VisitAtIndex<Min, Max>(index, std::forward<F>(f))

    KRYS_UNROLLED_32_CASE_VISIT_SWITCH(index, Minimum, Maximum, KRYS_INDEX_VISIT_CASE, KRYS_INDEX_VISIT_NEXT)

#undef KRYS_INDEX_VISIT_NEXT
#undef KRYS_INDEX_VISIT_CASE
  }

  // `AsVariant` is used to allow subclasses of Variant to work with `SwitchOn`.

  template <typename... Ts>
  KRYS_ALWAYS_INLINE constexpr Variant<Ts...> &AsVariant(Variant<Ts...> &v) noexcept
  {
    return v;
  }

  template <typename... Ts>
  KRYS_ALWAYS_INLINE constexpr const Variant<Ts...> &AsVariant(const Variant<Ts...> &v) noexcept
  {
    return v;
  }

  template <typename... Ts>
  KRYS_ALWAYS_INLINE constexpr Variant<Ts...> &&AsVariant(Variant<Ts...> &&v) noexcept
  {
    return std::move(v);
  }

  template <typename... Ts>
  KRYS_ALWAYS_INLINE constexpr const Variant<Ts...> &&AsVariant(const Variant<Ts...> &&v) noexcept
  {
    return std::move(v);
  }

  template <typename T>
  concept HasSwitchOn = requires(T t) { t.SwitchOn([](const auto &) {}); };

  template <typename Derived, typename Base>
  concept DerivedFromOrConvertibleTo =
    std::is_base_of_v<Base, Derived> || std::is_convertible_v<Derived, Base>;

  template <class V, class... F>
  requires(!HasSwitchOn<V>)
  KRYS_ALWAYS_INLINE constexpr auto SwitchOn(V &&v, F &&...f) noexcept
    -> decltype(std::visit(CreateVisitor(std::forward<F>(f)...), AsVariant(std::forward<V>(v))))
  {
    return std::visit(CreateVisitor(std::forward<F>(f)...), AsVariant(std::forward<V>(v)));
  }

  template <class V, class... F>
  requires(HasSwitchOn<V>)
  KRYS_ALWAYS_INLINE auto SwitchOn(V &&v, F &&...f) -> decltype(v.SwitchOn(std::forward<F>(f)...))
  {
    return v.SwitchOn(std::forward<F>(f)...);
  }

  template <class F, class Tuple>
  KRYS_ALWAYS_INLINE constexpr decltype(auto) VisitTupleElementAtIndex(F &&f, size_t index,
                                                                       Tuple &&tuple) noexcept
  {
    return VisitAtIndex<0, std::tuple_size_v<std::remove_cvref_t<Tuple>>>(
      index,
      [&]<size_t I>() { return std::invoke(std::forward<F>(f), std::get<I>(std::forward<Tuple>(tuple))); });
  }

  template <typename Tuple, typename... F>
  KRYS_ALWAYS_INLINE constexpr auto SwitchOnTupleAtIndex(size_t index, Tuple &&tuple, F &&...f)
    -> decltype(VisitTupleElementAtIndex(index, CreateVisitor(std::forward<F>(f)...),
                                         std::forward<Tuple>(tuple)))
  {
    return VisitTupleElementAtIndex(CreateVisitor(std::forward<F>(f)...), index, std::forward<Tuple>(tuple));
  }
}