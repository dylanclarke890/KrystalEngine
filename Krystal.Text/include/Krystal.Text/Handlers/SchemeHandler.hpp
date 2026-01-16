#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Ranges/Range.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Utils/ReferenceWrapper.hpp"
#include "Krystal.Text/Decode/DecodeResult.hpp"
#include "Krystal.Text/Encode/EncodeResult.hpp"
#include <type_traits>

namespace Krys::Text::Handlers
{
  template <typename TByte, typename TUInputRange, typename TUOutputRange, typename TErrorHandler>
  class SchemeHandler
  {
  private:
    ReferenceWrapper<TErrorHandler> _handler;

    template <typename TEncoding, typename TInput, typename TOutput, typename TState, typename TInputProgress,
              typename TOutputProgress>
    constexpr static bool DecodeCallOperatorNoexcept() noexcept
    {
      if constexpr (ConvertibleTo<::Krys::Ranges::range_value_type_t<TInputProgress>, TByte>)
      {
        if constexpr (Callable<TErrorHandler &, const TEncoding &, DecodeResult<TInput, TOutput, TState>,
                               const TInputProgress &, const TOutputProgress &>)
        {
          return NoThrowInvocable<TErrorHandler &, const TEncoding &, DecodeResult<TInput, TOutput, TState>,
                                  const TInputProgress &, const TOutputProgress &>;
        }
        else
        {
          return NoThrowInvocable<TErrorHandler &, const TEncoding &, DecodeResult<TInput, TOutput, TState>,
                                  Span<TByte>, const TOutputProgress &>;
        }
      }
      else
      {
        return NoThrowInvocable<TErrorHandler &, const TEncoding &, DecodeResult<TInput, TOutput, TState>,
                                Span<TByte>, const TOutputProgress &>;
      }
    }

    template <typename TEncoding, typename TInput, typename TOutput, typename TState, typename TInputProgress,
              typename TOutputProgress>
    constexpr static bool EncodeCallOperatorNoexcept() noexcept
    {
      if constexpr (ConvertibleTo<::Krys::Ranges::range_value_type_t<TOutputProgress>, TByte>)
      {
        if constexpr (Callable<TErrorHandler &, const TEncoding &, EncodeResult<TInput, TOutput, TState>,
                               const TInputProgress &, const TOutputProgress &>)
        {
          return NoThrowInvocable<TErrorHandler &, const TEncoding &, EncodeResult<TInput, TOutput, TState>,
                                  const TInputProgress &, const TOutputProgress &>;
        }
        else
        {
          return NoThrowInvocable<TErrorHandler &, const TEncoding &, EncodeResult<TInput, TOutput, TState>,
                                  const TInputProgress &, Span<TByte>>;
        }
      }
      else
      {
        return NoThrowInvocable<TErrorHandler &, const TEncoding &, EncodeResult<TInput, TOutput, TState>,
                                const TInputProgress &, Span<TByte>>;
      }
    }

    template <typename TEncoding, typename TInput, typename TOutput, typename TState, typename TInputProgress,
              typename TOutputProgress>
    constexpr auto BasicEncode(const TEncoding &encoding, EncodeResult<TInput, TOutput, TState> &&result,
                               const TInputProgress &inputProgress,
                               const TOutputProgress &outputProgress) const
      noexcept(NoThrowInvocable<TErrorHandler &, const TEncoding &, EncodeResult<TInput, TOutput, TState>,
                                const TInputProgress &, Span<TByte>>)
    {
      using TProgressPointer = ::Krys::Ranges::range_pointer_t<TOutputProgress>;
      using TProgressWord = ::Krys::Ranges::range_value_type_t<TOutputProgress>;
      TByte *byteProgressData =
        reinterpret_cast<TByte *>(const_cast<TProgressPointer>(outputProgress.data()));
      auto byteProgressSize =
        (::Krys::Ranges::size(outputProgress) * sizeof(TProgressWord)) / (sizeof(TByte));
      Span<TByte> byteProgress(byteProgressData, byteProgressSize);
      return this->_handler.get()(encoding, std::move(result), inputProgress, byteProgress);
    }

    template <typename TEncoding, typename TInput, typename TOutput, typename TState, typename TInputProgress,
              typename TOutputProgress>
    constexpr auto BasicDecode(const TEncoding &encoding, DecodeResult<TInput, TOutput, TState> &&result,
                               const TInputProgress &inputProgress,
                               const TOutputProgress &outputProgress) const
      noexcept(NoThrowInvocable<TErrorHandler &, const TEncoding &, DecodeResult<TInput, TOutput, TState>,
                                Span<TByte>, const TOutputProgress &>)
    {
      using TProgressPointer = ::Krys::Ranges::range_pointer_t<TInputProgress>;
      using TProgressWord = ::Krys::Ranges::range_value_type_t<TInputProgress>;
      TByte *byteProgressData = reinterpret_cast<TByte *>(const_cast<TProgressPointer>(inputProgress.data()));
      auto byteProgressSize = (::Krys::Ranges::size(inputProgress) * sizeof(TProgressWord)) / (sizeof(TByte));
      Span<TByte> byteProgress(byteProgressData, byteProgressSize);
      return this->_handler.get()(encoding, std::move(result), byteProgress, outputProgress);
    }

  public:
    constexpr SchemeHandler(TErrorHandler &handler) noexcept : _handler(handler)
    {
    }

    template <typename TEncoding, typename TInput, typename TOutput, typename TState, typename TInputProgress,
              typename TOutputProgress>
    constexpr auto operator()(const TEncoding &encoding, DecodeResult<TInput, TOutput, TState> result,
                              const TInputProgress &inputProgress,
                              const TOutputProgress &outputProgress) const
      noexcept(
        DecodeCallOperatorNoexcept<TEncoding, TInput, TOutput, TState, TInputProgress, TOutputProgress>())
    {
      if constexpr (ConvertibleTo<::Krys::Ranges::range_value_type_t<TInputProgress>, TByte>)
      {
        if constexpr (Callable<TErrorHandler &, const TEncoding &, DecodeResult<TInput, TOutput, TState>,
                               const TInputProgress &, const TOutputProgress &>)
        {
          return this->_handler.get()(encoding, std::move(result), inputProgress, outputProgress);
        }
        else
        {
          return this->BasicDecode(encoding, std::move(result), inputProgress, outputProgress);
        }
      }
      else
      {
        return this->BasicDecode(encoding, std::move(result), inputProgress, outputProgress);
      }
    }

    template <typename TEncoding, typename TInput, typename TOutput, typename TState, typename TInputProgress,
              typename TOutputProgress>
    constexpr auto operator()(const TEncoding &encoding, EncodeResult<TInput, TOutput, TState> result,
                              const TInputProgress &inputProgress,
                              const TOutputProgress &outputProgress) const
      noexcept(
        EncodeCallOperatorNoexcept<TEncoding, TInput, TOutput, TState, TInputProgress, TOutputProgress>())
    {
      if constexpr (ConvertibleTo<::Krys::Ranges::range_value_type_t<TInputProgress>, TByte>)
      {
        if constexpr (Callable<TErrorHandler &, const TEncoding &, EncodeResult<TInput, TOutput, TState>,
                               const TInputProgress &, const TOutputProgress &>)
        {
          return this->_handler.get()(encoding, std::move(result), inputProgress, outputProgress);
        }
        else
        {
          return this->BasicEncode(encoding, std::move(result), inputProgress, outputProgress);
        }
      }
      else
      {
        return this->BasicEncode(encoding, std::move(result), inputProgress, outputProgress);
      }
    }
  };
}
