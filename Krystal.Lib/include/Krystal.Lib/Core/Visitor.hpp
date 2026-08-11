#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
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
}