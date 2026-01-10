#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Utils/ReferenceWrapper.hpp"

namespace Krys::Text::detail_result
{
  template <template <typename...> class TResult, typename TInput, typename TArgInput, typename TOutput,
            typename TArgOutput, typename TState, typename TArgState, typename TState2, typename TArgState2,
            typename TPivot, typename TArgPivot>
  consteval bool IsResultTypeCopyable() noexcept
  {
    if constexpr (Void<TState>)
    {
      return !SameType<TResult<TInput, TOutput>, TResult<TArgInput, TArgOutput>>
             && Constructible<TInput, const TArgInput &> && Constructible<TOutput, const TArgOutput &>;
    }
    else if constexpr (Void<TState2>)
    {
      return !SameType<TResult<TInput, TOutput, TState>, TResult<TArgInput, TArgOutput, TArgState>>
             && Constructible<TInput, const TArgInput &> && Constructible<TOutput, const TArgOutput &>
             && Constructible<ReferenceWrapper<TState>, TArgState &>;
    }
    else if constexpr (Void<TPivot>)
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

  template <template <typename...> class TResult, typename TInput, typename TArgInput, typename TOutput,
            typename TArgOutput, typename TState, typename TArgState, typename TState2, typename TArgState2,
            typename TPivot, typename TArgPivot>
  consteval bool IsResultTypeCopyNoexcept() noexcept
  {
    if constexpr (Void<TState>)
    {
      return !NoThrowConstructible<TInput, const TArgInput &>
             && NoThrowConstructible<TOutput, const TArgOutput &>;
    }
    else if constexpr (Void<TState2>)
    {
      return NoThrowConstructible<TInput, const TArgInput &>
             && NoThrowConstructible<TOutput, const TArgOutput &>
             && NoThrowConstructible<ReferenceWrapper<TState>, TArgState &>;
    }
    else if constexpr (Void<TPivot>)
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

  template <template <typename...> class TResult, typename TInput, typename TArgInput, typename TOutput,
            typename TArgOutput, typename TState, typename TArgState, typename TState2, typename TArgState2,
            typename TPivot, typename TArgPivot>
  consteval bool IsResultTypeMovable() noexcept
  {
    if constexpr (Void<TState>)
    {
      return !SameType<TResult<TInput, TOutput>, TResult<TArgInput, TArgOutput>>
             && Constructible<TInput, TArgInput &&> && Constructible<TOutput, TArgOutput &&>;
    }
    else if constexpr (Void<TState2>)
    {
      return !SameType<TResult<TInput, TOutput, TState>, TResult<TArgInput, TArgOutput, TArgState>>
             && Constructible<TInput, TArgInput &&> && Constructible<TOutput, TArgOutput &&>
             && Constructible<ReferenceWrapper<TState>, TArgState &>;
    }
    else if constexpr (Void<TPivot>)
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
             && Constructible<ReferenceWrapper<TState2>, TArgState2 &> && Constructible<TPivot, TArgPivot &&>;
    }
  }

  template <template <typename...> class TResult, typename TInput, typename TArgInput, typename TOutput,
            typename TArgOutput, typename TState, typename TArgState, typename TState2, typename TArgState2,
            typename TPivot, typename TArgPivot>
  consteval bool IsResultTypeMoveNoexcept() noexcept
  {
    if constexpr (Void<TState>)
    {
      return NoThrowConstructible<TInput, TArgInput &&> && NoThrowConstructible<TOutput, TArgOutput &&>;
    }
    else if constexpr (Void<TState2>)
    {
      return NoThrowConstructible<TInput, TArgInput &&> && NoThrowConstructible<TOutput, TArgOutput &&>
             && NoThrowConstructible<ReferenceWrapper<TState>, TArgState &>;
    }
    else if constexpr (Void<TPivot>)
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

namespace Krys::Text
{
  template <template <typename...> class TResult, typename TInput, typename TArgInput, typename TOutput,
            typename TArgOutput, typename TState = void, typename TArgState = void, typename TState2 = void,
            typename TArgState2 = void, typename TPivot = void, typename TArgPivot = void>
  concept CopyableResultType =
    ::Krys::Text::detail_result::IsResultTypeCopyable<TResult, TInput, TArgInput, TOutput, TArgOutput, TState,
                                                      TArgState, TState2, TArgState2, TPivot, TArgPivot>();

  template <template <typename...> class TResult, typename TInput, typename TArgInput, typename TOutput,
            typename TArgOutput, typename TState = void, typename TArgState = void, typename TState2 = void,
            typename TArgState2 = void, typename TPivot = void, typename TArgPivot = void>
  concept NoThrowCopyableResultType =
    ::Krys::Text::detail_result::IsResultTypeCopyNoexcept<TResult, TInput, TArgInput, TOutput, TArgOutput,
                                                          TState, TArgState, TState2, TArgState2, TPivot,
                                                          TArgPivot>();

  template <template <typename...> class TResult, typename TInput, typename TArgInput, typename TOutput,
            typename TArgOutput, typename TState = void, typename TArgState = void, typename TState2 = void,
            typename TArgState2 = void, typename TPivot = void, typename TArgPivot = void>
  concept MovableResultType =
    ::Krys::Text::detail_result::IsResultTypeMovable<TResult, TInput, TArgInput, TOutput, TArgOutput, TState,
                                                     TArgState, TState2, TArgState2, TPivot, TArgPivot>();

  template <template <typename...> class TResult, typename TInput, typename TArgInput, typename TOutput,
            typename TArgOutput, typename TState = void, typename TArgState = void, typename TState2 = void,
            typename TArgState2 = void, typename TPivot = void, typename TArgPivot = void>
  concept NoThrowMovableResultType =
    ::Krys::Text::detail_result::IsResultTypeMoveNoexcept<TResult, TInput, TArgInput, TOutput, TArgOutput,
                                                          TState, TArgState, TState2, TArgState2, TPivot,
                                                          TArgPivot>();
}