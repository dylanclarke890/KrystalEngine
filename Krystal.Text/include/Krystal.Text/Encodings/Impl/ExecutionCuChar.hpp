#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Detection/Compiler.hpp"
#include "Krystal.Lib/Detection/OS.hpp"
#include "Krystal.Lib/Ranges/Range.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Text/DecodeResult.hpp"
#include "Krystal.Text/EncodeResult.hpp"
#include "Krystal.Text/EncodingDetection.hpp"
#include "Krystal.Text/EncodingError.hpp"
#include "Krystal.Text/Encodings/UTF16.hpp"
#include "Krystal.Text/Encodings/UTF8.hpp"
#include "Krystal.Text/Impl/EncodingName.hpp"
#include "Krystal.Text/Impl/ProgressHandler.hpp"
#include "Krystal.Text/Impl/Windows.hpp"
#include "Krystal.Text/IsIgnorableErrorHandler.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

#if KRYS_COMPILER_INCLUDE(<cuchar>)
  #include <cstdint>
  #include <cuchar>
  #include <cwchar>

namespace Krys
{
  namespace detail
  {
    class ExecutionDecodeState
    {
    public:
      /// @brief The narrow mbstate_t from the standard library for multibyte conversion sequences.
      mbstate_t NarrowState;

      /// @brief Whether or not there might be some accumulated data in the state.
      bool OutputPending;

      /// @brief Zero-initializes to its initial state, which includes the initial conversion sequence.
      ExecutionDecodeState() noexcept : NarrowState(), OutputPending(false)
      {
        assert(std::mbsinit(&NarrowState) != 0);
      }

      /// @brief Finds out whether or not the state contains any unused data that needs to complete an
      /// indivisible unit of work.
      /// @returns Whether or not there are additional information stored in any part of the standard-based
      /// streams have accumulated information for a continual decode operation.
      bool IsComplete() const noexcept
      {
        return !this->OutputPending && (std::mbsinit(&NarrowState) != 0);
      }
    };

    class ExecutionEncodeState
    {
    public:
      /// @brief The narrow mbstate_t from the standard library for multibyte conversion sequences.
      mbstate_t NarrowState;

      /// @brief Whether or not there might be some accumulated data in the state.
      bool OutputPending;

      /// @brief Zero-initializes to its initial state, which includes the initial conversion sequence.
      ExecutionEncodeState() noexcept : NarrowState(), OutputPending(false)
      {
        assert(std::mbsinit(&this->NarrowState) != 0);
      }

      /// @brief Finds out whether or not the state contains any unused data that needs to complete an
      /// indivisible unit of work.
      /// @returns Whether or not there are additional information stored in any part of the standard-based
      /// streams have accumulated information for a continual encode operation.
      bool IsComplete() const noexcept
      {
        return !this->OutputPending && (std::mbsinit(&NarrowState) != 0);
      }
    };
  }

  namespace Impl
  {
    /// @brief The Encoding that represents the "Execution" (narrow locale-based) encoding. The execution
    /// encoding is typically associated with the locale, which is tied to the C standard library's
    /// setlocale function.
    /// @remarks Use of this type is subject to the C Standard Library or platform defaults. Some locales
    /// (such as the Big5 Hong King Supplementary Character Set (Big5-HKSCS)) are broken due to fundamental
    /// design issues in the C Standard Library and bugs in glibc/musl libc's current locale encoding
    /// support. On Apple, this is cuurrently assumed to be UTF-8 since they do not support the @c
    /// \<cuchar\> or @c \<uchar.h\> headers.
    class ExecutionCuChar
    {
    public:
      /// @brief The state of the execution encoding used between decode calls, which may potentially manage
      /// shift state.
      /// @remarks This type can potentially have lots of state due to the way the C API is specified. It is
      /// important it is preserved between calls, or text may become mangled / data may become lost.
      using decode_state = Krys::detail::ExecutionDecodeState;

      /// @brief The state of the execution encoding used between encode calls, which may potentially manage
      /// shift state.
      /// @remarks This type can potentially have lots of state due to the way the C API is specified. It is
      /// important it is preserved between calls, or text may become mangled / data may become lost.
      using encode_state = Krys::detail::ExecutionEncodeState;

      /// @brief The individual units that result from an encode operation or are used as input to a decode
      /// operation.
      /// @remarks Please note that char can be either signed or unsigned, and so generally can result in
      /// bad results when promoted to a plain `int` when working with code units or working with the C
      /// Standard Library.
      using code_unit = char;

      /// @brief The individual units that result from a decode operation or as used as input to an encode
      /// operation. For most encodings, this is going to be a Unicode Code Point or a Unicode Scalar Value.
      using code_point = UnicodeCodePoint;

      /// @brief Whether or not the decode operation can process all forms of input into code point values.
      /// @remarks All known execution encodings can decode into Unicode just fine. However, someone may
      /// define a platform encoding on their machine that does not transform cleanly. Therefore, decoding
      /// is not marked as injective.
      using is_decode_injective = std::false_type;

      /// @brief Whether or not the encode operation can process all forms of input into code unit values.
      /// This is absolutely not true: many unicode code point values cannot be safely converted to a large
      /// number of existing (legacy) encodings.
      using is_encoding_injective = std::false_type;

      /// @brief The maximum code units a single complete operation of encoding can produce.
      /// @remarks There are encodings for which one input can produce 3 code points (some Tamil encodings)
      /// and there are rumours of an encoding that can produce 7 code points from a handful of input. We
      /// use a protective/conservative 8, here, to make sure ABI isn't broken later.
      inline static constexpr std::size_t MaxCodePoints = 8;

      /// @brief The maximum number of code points a single complete operation of decoding can produce.
      /// @remarks This is bounded by the platform's `MB_LEN_MAX` macro, which is an integral constant
      /// expression representing the maximum value of output all C locales can produce from a single
      /// complete operation.
      inline static constexpr std::size_t MaxCodeUnits = MB_LEN_MAX;

      /// @brief A range of code unit values that can be used as a replacement value, instead of the ones
      /// used in Krys::default_handler_t.
      /// @remarks The default replacement code point / code unit is U+FFFD (). This, obviously, does not
      /// fit in the majority of the (legacy) locale encodings in C and C++. '?' is a much more conservative
      /// option, here, and most (all?) locale encodings have some form of representation for it.
      inline static constexpr code_unit ReplacementCodeUnits[1] = {'?'};

      /// @brief Returns whether or not this encoding is a unicode encoding.
      /// @remarks This function operates at runtime and queries the existing locale through a variety of
      /// platform-specific means (such as `nl_langinfo` for POSIX, ACP probing on Windows, or fallin back
      /// to `std::setlocale` name checking otherwise).
      static bool ContainsUnicodeEncoding() noexcept
      {
        return Krys::is_execution_encoding_unicode();
      }

      /// @brief Encodes a single complete unit of information as code units and produces a result with the
      /// input and output ranges moved past what was successfully read and written; or, produces an error
      /// and returns the input and output ranges untouched.
      /// @param[in] input The input view to read code uunits from.
      /// @param[in] output The output view to write code points into.
      /// @param[in] errorHandler The error handler to invoke if encoding fails.
      /// @param[in, out] s The necessary state information. Most encodings have no state, but because
      /// this is effectively a runtime encoding and therefore it is important to preserve and manage this
      /// state.
      /// @returns A Krys::EncodeResult object that contains the reconstructed input range,
      /// reconstructed output range, error handler, and a reference to the passed-in state.
      /// @remarks Platform APIs and/or the C Standard Library may be used to properly decode one complete
      /// unit of information (alongside mbstate_t usage). Whether or not the state is used is based on the
      /// implementation and what it chooses.
      /// @remarks To the best ability of the implementation, the iterators will be
      /// returned untouched (e.g., the input models at least a view and a forward_range). If it is not
      /// possible, returned ranges may be incremented even if an error occurs due to the semantics of any
      /// view that models an input_range.
      template <typename TInput, typename TOutput, typename TErrorHandler>
      static constexpr auto EncodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler,
                                      encode_state &s)
      {
        using TUErrorHandler = remove_cvref_t<TErrorHandler>;
        using TSubInput = Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
        using TSubOutput = Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
        using TResult = EncodeResult<TSubInput, TSubOutput, encode_state>;
        constexpr bool CallErrorHandler = !IsIgnorableErrorHandler<TUErrorHandler>;

        if (Krys::is_execution_encoding_utf8())
        {
          // just go straight to UTF8
          using TExecutionUTF8 =
            Impl::UTF8With<ExecutionCuChar, code_unit, code_point, decode_state, encode_state>;
          TExecutionUTF8 baseEncoding {};
          return baseEncoding.EncodeOne(std::forward<TInput>(input), std::forward<TOutput>(output),
                                        std::forward<TErrorHandler>(errorHandler), s);
        }

        auto inIt = ::Krys::Ranges::cbegin(input);
        auto inLast = ::Krys::Ranges::cend(input);

        if (inIt == inLast)
        {
          // an exhausted sequence is fine
          return TResult(TSubInput(std::move(inIt), std::move(inLast)), std::forward<TOutput>(output), s,
                         EncodingError::OK);
        }

  #if KRYS_OS(WINDOWS)
        auto outIt = ::Krys::Ranges::begin(output);
        auto outLast = ::Krys::Ranges::end(output);

        if constexpr (CallErrorHandler)
        {
          if (outIt == outLast)
          {
            ExecutionCuChar self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self,
              TResult(std::forward<TInput>(input), TSubOutput(std::move(outIt), std::move(outLast)), s,
                      EncodingError::InsufficientOutputSpace),
              Span<code_point>(), Span<code_unit>());
          }
        }

        using TWUT16 = Impl::UTF16With<void, wchar_t, code_point, false>;
        using TIntermediateState = encode_state_t<TWUT16>;

        TWUT16 intermediateEncoding {};
        TIntermediateState intermediateState {};
        Impl::ProgressHandler<!CallErrorHandler, TWUT16> intermediateHandler {};
        wchar_t wideIntermediary[8] {};
        Span<wchar_t> wideWriteBuffer(wideIntermediary);
        auto intermediateResult = intermediateEncoding.EncodeOne(std::forward<TInput>(input), wideWriteBuffer,
                                                                 intermediateHandler, intermediateState);
        if constexpr (CallErrorHandler)
        {
          if (intermediateResult.ErrorCode != EncodingError::OK)
          {
            ExecutionCuChar self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self,
              TResult(std::move(intermediateResult.Input), std::forward<TOutput>(output), s,
                      intermediateResult.ErrorCode),
              intermediateHandler.CodePointsProgress(), Span<code_unit>());
          }
        }
        constexpr const std::size_t stateCountMax = 12;
        code_unit intermediaryOutput[stateCountMax] {};
        int usedDefaultChar = false;
        Span<const wchar_t> wideReadBuffer(wideIntermediary, intermediateResult.Output.data());
        int res = ::WideCharToMultiByte(
          static_cast<UINT>(Krys::Windows::DetermineActiveCodePage()), WC_ERR_INVALID_CHARS,
          wideReadBuffer.data(), static_cast<int>(wideReadBuffer.size()), intermediaryOutput, stateCountMax,
          std::addressof(ReplacementCodeUnits[0]), std::addressof(usedDefaultChar));
        if constexpr (CallErrorHandler)
        {
          if (res == 0)
          {
            ExecutionCuChar self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self,
              TResult(std::move(intermediateResult.Input), std::forward<TOutput>(output), s,
                      ::GetLastError() == ERROR_INSUFFICIENT_BUFFER ? EncodingError::InsufficientOutputSpace
                                                                    : EncodingError::InvalidSequence),
              intermediateHandler.CodePointsProgress(), Span<code_unit>());
          }
        }
        for (auto intermediaryIt = intermediaryOutput; res-- > 0;)
        {
          if constexpr (CallErrorHandler)
          {
            if (outIt == outLast)
            {
              ExecutionCuChar self {};
              Span<code_unit> codeUnitProgress(intermediaryIt, static_cast<std::size_t>(res + 1));
              return std::forward<TErrorHandler>(errorHandler)(
                self,
                TResult(std::move(intermediateResult.Input), TSubOutput(std::move(outIt), std::move(outLast)),
                        s, EncodingError::InsufficientOutputSpace),
                intermediateHandler.CodePointsProgress(), codeUnitProgress);
            }
          }
          *outIt = *intermediaryIt;
          ::Krys::Ranges::iter_advance(outIt);
        }
        return TResult(std::move(intermediateResult.Input), TSubOutput(std::move(outIt), std::move(outLast)),
                       s, intermediateResult.ErrorCode);
  #else

        auto outIt = ::Krys::Ranges::begin(output);
        auto outLast = ::Krys::Ranges::end(output);

        if constexpr (CallErrorHandler)
        {
          if (outIt == outLast)
          {
            ExecutionCuChar self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self,
              TResult(TSubInput(std::move(inIt), std::move(inLast)),
                      TSubOutput(std::move(outIt), std::move(outLast)), s,
                      EncodingError::InsufficientOutputSpace),
              Span<code_point>(), Span<code_unit>());
          }
        }

        code_point codepoint = *inIt;
        ::Krys::Ranges::iter_advance(inIt);
        code_unit intermediaryOutput[(MB_LEN_MAX)] {};
        std::size_t res = std::c32rtomb(intermediaryOutput, codepoint, std::addressof(s.NarrowState));
        if constexpr (CallErrorHandler)
        {
          if (res == static_cast<std::size_t>(-1))
          {
            ExecutionCuChar self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self,
              TResult(TSubInput(std::move(inIt), std::move(inLast)),
                      TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::InvalidSequence),
              Span<code_point>(&codepoint, 1), Span<code_unit>());
          }
        }

        for (auto intermediaryIt = intermediaryOutput; res-- > 0; ++intermediaryIt)
        {
          if constexpr (CallErrorHandler)
          {
            if (outIt == outLast)
            {
              ExecutionCuChar self {};
              Span<code_unit> codeUnitProgress(intermediaryIt, static_cast<std::size_t>(res + 1));
              return std::forward<TErrorHandler>(errorHandler)(
                self,
                TResult(TSubInput(std::move(inIt), std::move(inLast)),
                        TSubOutput(std::move(outIt), std::move(outLast)), s,
                        EncodingError::InsufficientOutputSpace),
                Span<code_point>(&codepoint, 1), codeUnitProgress);
            }
          }
          *outIt = *intermediaryIt;
          ::Krys::Ranges::iter_advance(outIt);
        }

        return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                       TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
  #endif // Windows is hell
      }

      /// @brief Decodes a single complete unit of information as code points and produces a result with the
      /// input and output ranges moved past what was successfully read and written; or, produces an error
      /// and returns the input and output ranges untouched.
      ///
      /// @param[in] input The input view to read code uunits from.
      /// @param[in] output The output view to write code points into.
      /// @param[in] errorHandler The error handler to invoke if encoding fails.
      /// @param[in, out] s The necessary state information. Most encodings have no state, but because
      /// this is effectively a runtime encoding and therefore it is important to preserve and manage this
      /// state.
      ///
      /// @returns A Krys::DecodeResult object that contains the reconstructed input range,
      /// reconstructed output range, error handler, and a reference to the passed-in state.
      ///
      /// @remarks Platform APIs and/or the C Standard Library may be used to properly decode one complete
      /// unit of information (alongside mbstate_t usage). Whether or not the state is used is based on the
      /// implementation and what it chooses.
      ///
      /// @remarks To the best ability of the implementation, the iterators will be
      /// returned untouched (e.g., the input models at least a view and a forward_range). If it is not
      /// possible, returned ranges may be incremented even if an error occurs due to the semantics of any
      /// view that models an input_range.
      template <typename TInput, typename TOutput, typename TErrorHandler>
      static constexpr auto DecodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler,
                                      decode_state &s)
      {
        using TUErrorHandler = remove_cvref_t<TErrorHandler>;
        using TSubInput = Krys::Ranges::csubrange_for_t<std::remove_reference_t<TInput>>;
        using TSubOutput = Krys::Ranges::subrange_for_t<std::remove_reference_t<TOutput>>;
        using TResult = DecodeResult<TSubInput, TSubOutput, decode_state>;
        constexpr bool CallErrorHandler = !IsIgnorableErrorHandler<TUErrorHandler>;

        if (Krys::is_execution_encoding_utf8())
        {
          // just go straight to UTF8
          using TExecutionUTF8 =
            Impl::UTF8With<ExecutionCuChar, code_unit, code_point, decode_state, encode_state>;
          TExecutionUTF8 baseEncoding {};
          return baseEncoding.DecodeOne(std::forward<TInput>(input), std::forward<TOutput>(output),
                                        std::forward<TErrorHandler>(errorHandler), s);
        }

        auto inIt = ::Krys::Ranges::cbegin(input);
        auto inLast = ::Krys::Ranges::cend(input);

        if (inIt == inLast)
        {
          // an exhausted sequence is fine
          return TResult(TSubInput(std::move(inIt), std::move(inLast)), std::forward<TOutput>(output), s,
                         EncodingError::OK);
        }

        auto outIt = ::Krys::Ranges::begin(output);
        auto outLast = ::Krys::Ranges::end(output);

        if constexpr (CallErrorHandler)
        {
          if (outIt == outLast)
          {
            ExecutionCuChar self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self,
              TResult(TSubInput(std::move(inIt), std::move(inLast)),
                      TSubOutput(std::move(outIt), std::move(outLast)), s,
                      EncodingError::InsufficientOutputSpace),
              Span<code_unit>(), Span<code_point>());
          }
        }

        code_unit intermediaryInput[MaxCodeUnits] {};
  #if KRYS_OS(WINDOWS)
        intermediaryInput[0] = *inIt;
        ::Krys::Ranges::iter_advance(inIt);
        std::size_t stateCount = 1;
        for (; stateCount < MaxCodeUnits; ++stateCount)
        {
          using TWUT16 = Impl::UTF16With<void, wchar_t, code_point, false>;
          using TIntermediateState = decode_state_t<TWUT16>;

          constexpr const int wideIntermediarySize = 4;
          wchar_t wideIntermediary[wideIntermediarySize] {};
          int res = ::MultiByteToWideChar(
            static_cast<UINT>(Krys::Windows::DetermineActiveCodePage()), MB_ERR_INVALID_CHARS,
            intermediaryInput, static_cast<int>(stateCount), wideIntermediary, wideIntermediarySize);
          if (res == 0)
          {
            if (::GetLastError() == ERROR_NO_UNICODE_TRANSLATION)
            {
              // loopback; we might just not have enough code units
              if constexpr (CallErrorHandler)
              {
                if (inIt == inLast)
                {
                  ExecutionCuChar self {};
                  return std::forward<TErrorHandler>(errorHandler)(
                    self,
                    TResult(TSubInput(std::move(inIt), std::move(inLast)),
                            TSubOutput(std::move(outIt), std::move(outLast)), s,
                            EncodingError::IncompleteSequence),
                    Span<code_unit>(intermediaryInput, stateCount), Span<code_point>());
                }
              }
              intermediaryInput[stateCount] = *inIt;
              ::Krys::Ranges::iter_advance(inIt);
              continue;
            }
            if constexpr (CallErrorHandler)
            {
              ExecutionCuChar self {};
              return std::forward<TErrorHandler>(errorHandler)(
                self,
                TResult(TSubInput(std::move(inIt), std::move(inLast)),
                        TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::InvalidSequence),
                Span<code_unit>(intermediaryInput, stateCount), Span<code_point>());
            }
          }

          TWUT16 intermediateEncoding {};
          TIntermediateState intermediateState {};
          Impl::ProgressHandler<!CallErrorHandler, TWUT16> intermediateHandler {};
          Span<wchar_t, sizeof(wideIntermediary) / sizeof(wchar_t)> wideIntermediaryView(wideIntermediary);
          auto intermediateResult = intermediateEncoding.EncodeOne(
            wideIntermediaryView, std::forward<TOutput>(output), intermediateHandler, intermediateState);
          if constexpr (CallErrorHandler)
          {
            if (intermediateResult.ErrorCode != EncodingError::OK)
            {
              ExecutionCuChar self {};
              return std::forward<TErrorHandler>(errorHandler)(
                self,
                TResult(TSubInput(std::move(inIt), std::move(inLast)), std::move(intermediateResult.Output),
                        s, intermediateResult.ErrorCode),
                Span<code_unit>(intermediaryInput, stateCount), intermediateHandler.CodePointsProgress());
            }
          }
          return TResult(TSubInput(std::move(inIt), std::move(inLast)), std::move(intermediateResult.Output),
                         s, intermediateResult.ErrorCode);
  #else
        if (s.OutputPending)
        {
          // need to drain potential mbstate_t of any leftover code points?
          char32 intermediaryOutput[MaxCodePoints] {};
          std::size_t res =
            std::mbrtoc32(std::addressof(intermediaryOutput[0]), nullptr, 0, std::addressof(s.NarrowState));
          if constexpr (CallErrorHandler)
          {
            if (res == static_cast<std::size_t>(-1))
            {
              ExecutionCuChar self {};
              return std::forward<TErrorHandler>(errorHandler)(
                self,
                TResult(TSubInput(std::move(inIt), std::move(inLast)),
                        TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::InvalidSequence),
                Span<code_unit>(), Span<code_point>());
            }
          }
          *outIt = intermediaryOutput[0];
          ::Krys::Ranges::iter_advance(outIt);
          s.OutputPending = res == static_cast<std::size_t>(-3);
          return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                         TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
        }

        std::size_t stateOffset = 0;
        std::size_t stateCount = 1;
        for (; stateOffset < MaxCodeUnits; (void)++stateOffset, (void)++stateCount)
        {
          mbstate_t preservedState = s.NarrowState;
          intermediaryInput[stateOffset] = *inIt;
          ::Krys::Ranges::iter_advance(inIt);
          char32 intermediaryOutput[1] {};
          std::size_t res =
            std::mbrtoc32(std::addressof(intermediaryOutput[0]), std::addressof(intermediaryInput[0]),
                          stateCount, std::addressof(preservedState));

          switch (res)
          {
            case static_cast<std::size_t>(-2):
              // cycle around and continue
              if constexpr (CallErrorHandler)
              {
                if (inIt == inLast)
                {
                  ExecutionCuChar self {};
                  return std::forward<TErrorHandler>(errorHandler)(
                    self,
                    TResult(TSubInput(std::move(inIt), std::move(inLast)),
                            TSubOutput(std::move(outIt), std::move(outLast)), s,
                            EncodingError::IncompleteSequence),
                    Span<code_unit>(intermediaryInput, stateCount), Span<code_point>());
                }
              }
              break;
            case static_cast<std::size_t>(-3):
              *outIt = intermediaryOutput[0];
              ::Krys::Ranges::iter_advance(outIt);
              s.NarrowState = preservedState;
              s.OutputPending = true;
              stateOffset = stateCount;
              return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                             TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
            case static_cast<std::size_t>(-1):
              if constexpr (CallErrorHandler)
              {
                // OH GOD PANIC AAAAAAH
                // seriously we're out of spec here:
                // everything has gone to shit
                // even the NarrowState is unspecified ;;
                ExecutionCuChar self {};
                return std::forward<TErrorHandler>(errorHandler)(
                  self,
                  TResult(TSubInput(std::move(inIt), std::move(inLast)),
                          TSubOutput(std::move(outIt), std::move(outLast)), s,
                          EncodingError::InvalidSequence),
                  Span<code_unit>(std::addressof(intermediaryInput[0]), stateCount), Span<code_point>());
              }
              else
              {
                break;
              }
            case static_cast<std::size_t>(0):
              // 0 means null character; ok
              *outIt = intermediaryOutput[0];
              ::Krys::Ranges::iter_advance(outIt);
              return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                             TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
            default:
              *outIt = intermediaryOutput[0];
              ::Krys::Ranges::iter_advance(outIt);
              s.NarrowState = preservedState;
              return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                             TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
          }
  #endif
        }
        if constexpr (CallErrorHandler)
        {
          // if it was invalid, we would have caught it before
          // this is for incomplete sequences only
          ExecutionCuChar self {};
          return std::forward<TErrorHandler>(errorHandler)(
            self,
            TResult(TSubInput(std::move(inIt), std::move(inLast)),
                    TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::IncompleteSequence),
            Span<code_unit>(std::addressof(intermediaryInput[0]), stateCount), Span<code_point>());
        }
        else
        {
          // ... I mean.
          // You asked for it???
          TResult(TSubInput(std::move(inIt), std::move(inLast)),
                  TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
        }
      }
    };
  }
}
#endif
