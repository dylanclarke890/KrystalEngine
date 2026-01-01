#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Types/Span.hpp"

#include <type_traits>

namespace Krys
{
  namespace Impl
  {
    template <typename T, typename TInput, typename TOutput, typename TState, typename TErrorHandler>
    concept HasEncodeBackward = requires {
      std::declval<T>().EncodeOneBackward(std::declval<TInput &>(), std::declval<TOutput &>(),
                                          std::declval<TErrorHandler &>(), std::declval<TState &>());
    };

    template <typename T, typename TInput, typename TOutput, typename TState, typename TErrorHandler>
    concept HasDecodeBackward = requires {
      std::declval<T &>().DecodeOneBackward(std::declval<TInput &>(), std::declval<TOutput &>(),
                                            std::declval<TErrorHandler &>(), std::declval<TState &>());
    };
  }

  template <typename T, typename TInput, typename TOutput, typename TState, typename TErrorHandler>
  concept IsBidirectionalEncoding = Impl::HasDecodeBackward<T, TInput, TOutput, TState, TErrorHandler>
                                    && Impl::HasEncodeBackward<T, TInput, TOutput, TState, TErrorHandler>;
}
