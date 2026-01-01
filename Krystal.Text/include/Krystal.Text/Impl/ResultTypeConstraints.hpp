#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Utils/ReferenceWrapper.hpp"

namespace Krys::Impl
{
  template <template <typename...> class TResult, typename TInput, typename TArgInput, typename TOutput,
            typename TArgOutput, typename TState = void, typename TArgState = void, typename TState2 = void,
            typename TArgState2 = void, typename TPivot = void, typename TArgPivot = void>
  constexpr bool ResultTypeCopyNoexcept() noexcept
  {
    if constexpr (Void<TState>)
    {
      return !NoThrowConstructible<TInput, const TArgInput &>
             && NoThrowConstructible<TOutput, const TArgOutput &>;
    }
    else
    {
      if constexpr (Void<TState2>)
      {
        return NoThrowConstructible<TInput, const TArgInput &>
               && NoThrowConstructible<TOutput, const TArgOutput &>
               && NoThrowConstructible<ReferenceWrapper<TState>, TArgState &>;
      }
      else
      {
        if constexpr (Void<TPivot>)
        {
          return NoThrowConstructible<TInput, const TArgInput &>
                 && NoThrowConstructible<TOutput, const TArgOutput &>
                 && NoThrowConstructible<ReferenceWrapper<TState>, TArgState &>
                 && NoThrowConstructible<ReferenceWrapper<TState2>, TArgState2 &>;
        }
        else
        {
          return NoThrowConstructible<TInput, const TArgInput &>
                 && NoThrowConstructible<TOutput, const TArgOutput &>
                 && NoThrowConstructible<ReferenceWrapper<TState>, TArgState &>
                 && NoThrowConstructible<ReferenceWrapper<TState2>, TArgState2 &>
                 && NoThrowConstructible<TPivot, const TArgPivot &>;
        }
      }
    }
  }

  template <template <typename...> class TResult, typename TInput, typename TArgInput, typename TOutput,
            typename TArgOutput, typename TState = void, typename TArgState = void, typename TState2 = void,
            typename TArgState2 = void, typename TPivot = void, typename TArgPivot = void>
  constexpr bool ResultTypeCopyConstraint() noexcept
  {
    if constexpr (Void<TState>)
    {
      return !SameType<TResult<TInput, TOutput>, TResult<TArgInput, TArgOutput>>
             && Constructible<TInput, const TArgInput &> && Constructible<TOutput, const TArgOutput &>;
    }
    else
    {
      if constexpr (Void<TState2>)
      {
        return !SameType<TResult<TInput, TOutput, TState>, TResult<TArgInput, TArgOutput, TArgState>>
               && Constructible<TInput, const TArgInput &> && Constructible<TOutput, const TArgOutput &>
               && Constructible<ReferenceWrapper<TState>, TArgState &>;
      }
      else
      {
        if constexpr (Void<TPivot>)
        {
          return !SameType<TResult<TInput, TOutput, TState, TState2>,
                           TResult<TArgInput, TArgOutput, TArgState, TArgState2>>
                 && Constructible<TInput, const TArgInput &> && Constructible<TOutput, const TArgOutput &>
                 && Constructible<ReferenceWrapper<TState>, TArgState &>
                 && Constructible<ReferenceWrapper<TState2>, TArgState2 &>;
        }
        else
        {
          return !SameType<TResult<TInput, TOutput, TState, TState2, TPivot>,
                           TResult<TArgInput, TArgOutput, TArgState, TArgState2, TArgPivot>>
                 && Constructible<TInput, const TArgInput &> && Constructible<TOutput, const TArgOutput &>
                 && Constructible<ReferenceWrapper<TState>, TArgState &>
                 && Constructible<ReferenceWrapper<TState2>, TArgState2 &>
                 && Constructible<TPivot, const TArgPivot &>;
        }
      }
    }
  }

  template <template <typename...> class TResult, typename TInput, typename TArgInput, typename TOutput,
            typename TArgOutput, typename TState = void, typename TArgState = void, typename TState2 = void,
            typename TArgState2 = void, typename TPivot = void, typename TArgPivot = void>
  constexpr bool ResultTypeMoveNoexcept() noexcept
  {
    if constexpr (Void<TState>)
    {
      return NoThrowConstructible<TInput, TArgInput &&> && NoThrowConstructible<TOutput, TArgOutput &&>;
    }
    else
    {
      if constexpr (Void<TState2>)
      {
        return NoThrowConstructible<TInput, TArgInput &&> && NoThrowConstructible<TOutput, TArgOutput &&>
               && NoThrowConstructible<ReferenceWrapper<TState>, TArgState &>;
      }
      else
      {
        if constexpr (Void<TPivot>)
        {
          return NoThrowConstructible<TInput, TArgInput &&> && NoThrowConstructible<TOutput, TArgOutput &&>
                 && NoThrowConstructible<ReferenceWrapper<TState>, TArgState &>
                 && NoThrowConstructible<ReferenceWrapper<TState2>, TArgState2 &>;
        }
        else
        {
          return NoThrowConstructible<TInput, TArgInput &&> && NoThrowConstructible<TOutput, TArgOutput &&>
                 && NoThrowConstructible<ReferenceWrapper<TState>, TArgState &>
                 && NoThrowConstructible<ReferenceWrapper<TState2>, TArgState2 &>
                 && NoThrowConstructible<TPivot, TArgPivot &&>;
        }
      }
    }
  }

  template <template <typename...> class TResult, typename TInput, typename TArgInput, typename TOutput,
            typename TArgOutput, typename TState = void, typename TArgState = void, typename TState2 = void,
            typename TArgState2 = void, typename TPivot = void, typename TArgPivot = void>
  constexpr bool ResultTypeMoveConstraint() noexcept
  {
    if constexpr (Void<TState>)
    {
      return !SameType<TResult<TInput, TOutput>, TResult<TArgInput, TArgOutput>>
             && Constructible<TInput, TArgInput &&> && Constructible<TOutput, TArgOutput &&>;
    }
    else
    {
      if constexpr (Void<TState2>)
      {
        return !SameType<TResult<TInput, TOutput, TState>, TResult<TArgInput, TArgOutput, TArgState>>
               && Constructible<TInput, TArgInput &&> && Constructible<TOutput, TArgOutput &&>
               && Constructible<ReferenceWrapper<TState>, TArgState &>;
      }
      else
      {
        if constexpr (Void<TPivot>)
        {
          return !SameType<TResult<TInput, TOutput, TState, TState2>,
                           TResult<TArgInput, TArgOutput, TArgState, TArgState2>>
                 && Constructible<TInput, TArgInput &&> && Constructible<TOutput, TArgOutput &&>
                 && Constructible<ReferenceWrapper<TState>, TArgState &>
                 && Constructible<ReferenceWrapper<TState2>, TArgState2 &>;
        }
        else
        {
          return !SameType<TResult<TInput, TOutput, TState, TState2, TPivot>,
                           TResult<TArgInput, TArgOutput, TArgState, TArgState2, TArgPivot>>
                 && Constructible<TInput, TArgInput &&> && Constructible<TOutput, TArgOutput &&>
                 && Constructible<ReferenceWrapper<TState>, TArgState &>
                 && Constructible<ReferenceWrapper<TState2>, TArgState2 &>
                 && Constructible<TPivot, TArgPivot &&>;
        }
      }
    }
  }
}
