#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Detection/Compiler.hpp"
#include "Krystal.Lib/Detection/OS.hpp"
#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Concepts.hpp"
#include "Krystal.Text/Decode/DecodeResult.hpp"
#include "Krystal.Text/Encode/EncodeResult.hpp"
#include "Krystal.Text/Encodings/detail/Windows.hpp"
#include "Krystal.Text/Encodings/UTF16.hpp"
#include "Krystal.Text/Handlers/ProgressHandler.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"
#include <optional>
#include <utility>
#include <xutility>

namespace Krys::Text
{
  namespace Impl
  {
    constexpr inline int32 CodePageActive = 0;
    constexpr inline int32 CodePageOEM = 1;
    constexpr inline int32 CodePageMac = 2;
    constexpr inline int32 CodePageActiveThread = 3;
  }

  template <typename TCodeUnit, typename TCodePoint>
  class basic_windows_code_page;

  /// @brief The encoding representing a Windows conversion using WideCharToMultiByte (encode) and
  /// MultiByteToWideChar (decode) conversion sequences.
  /// @remarks This is slow due to the bad design of WideCharToMultiByte/MultiByteToWideChar, but it does
  /// guarantee access to all of the wide variety of legacy encodings Windows supports. If possible, a
  /// different named encoding type should be used to avoid the performance penalties that comes from API
  /// limitations of this encoding object. Occasionally, for correct text, this API limitations can be
  /// mitigated when using the bulk APIs: for the single-conversion APIs that do not perform bulk
  /// conversion, the performance penalty is endemic.
  template <typename TCodeUnit, typename TCodePoint = UnicodeCodePoint>
  class basic_windows_code_page
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = {"Unknown"_s};
    constexpr static inline ::Krys::Array<::Krys::Text::ASCIILiteral, 0> Aliases = {};

    /// @brief The code point type that is decoded to, and encoded from.
    using code_point = TCodePoint;

    /// @brief The code unit type that is decoded from, and encoded to.
    using code_unit = TCodeUnit;

    /// @brief The associated state for decode operations.
    struct decode_state
    {
      std::optional<code_point> MaybeReplacementCodePoint;

      constexpr decode_state() : MaybeReplacementCodePoint()
      {
      }

      constexpr decode_state(code_point replacementCodePoint)
          : MaybeReplacementCodePoint(replacementCodePoint)
      {
      }

      constexpr decode_state(const basic_windows_code_page &encoding)
          : MaybeReplacementCodePoint(encoding._replacementCodePoint)
      {
      }
    };

    /// @brief The associated state for encode operations.
    struct encode_state
    {
      std::optional<code_unit> MaybeReplacementCodeUnit;

      constexpr encode_state() : MaybeReplacementCodeUnit()
      {
      }

      constexpr encode_state(code_unit replacementCodeUnit) : MaybeReplacementCodeUnit(replacementCodeUnit)
      {
      }

      constexpr encode_state(const basic_windows_code_page &encoding)
          : MaybeReplacementCodeUnit(encoding._replacementCodeUnit)
      {
      }
    };

    /// @brief Returns a potential view into the code points to use when an error happens and a replacement
    /// strategy is being used.
    /// @param[in] state The decode state of this particular conversion operation.
    /// @return A potential view of code points to do replacements with. This is influenced by either what
    /// the state for the conversion was initialized with, and it defaults to what was stored in the actual
    /// encoding object.
    constexpr std::optional<Span<const code_point>>
      MaybeReplacementCodePoints(const decode_state &state) noexcept
    {
      if (state.MaybeReplacementCodePoint.has_value())
      {
        return std::optional<Span<const code_point>>(
          std::in_place, std::addressof(state.MaybeReplacementCodePoint.value()), 1);
      }
      return std::nullopt;
    }

    /// @brief Returns a potential view into the code points to use when an error happens and a replacement
    /// strategy is being used.
    /// @param[in] state The encode state of this particular conversion operation.
    /// @return A potential view of code units to do replacements with. This is influenced by either what
    /// the state for the conversion was initialized with, and it defaults to what was stored in the actual
    /// encoding object.
    constexpr std::optional<Span<const code_unit>>
      MaybeReplacementCodeUnits(const encode_state &state) noexcept
    {
      if (state.MaybeReplacementCodeUnit.has_value())
      {
        return std::optional<Span<const code_unit>>(
          std::in_place, std::addressof(state.MaybeReplacementCodeUnit.value()), 1);
      }
      return std::nullopt;
    }

    constexpr inline static const std::size_t MaxCodeUnits = 8;

    constexpr inline static const std::size_t MaxCodePoints = 8;

    /// @brief Default constructor: assumes the code page is the default (thread) code page with the value
    /// `CP_THREAD_ACP`.
    constexpr basic_windows_code_page() noexcept
        : basic_windows_code_page(::Krys::Text::Windows::CodePageActiveThread)
    {
    }

    /// @brief Constructor that applies the given code page to the underlying conversions.
    /// @param win32CodePage The code page. It is one of the allowed code page values
    /// (https://learn.microsoft.com/en-us/windows/win32/intl/code-page-identifiers) that are blessed by the
    /// WideCharToMultiByte and MultiByteToWideChar.
    constexpr basic_windows_code_page(uint32_t win32CodePage) noexcept
        : _codePage(win32CodePage), _replacementCodePoint(), _replacementCodeUnit(), _codepageInfo()
    {
#if KRYS_OS(WINDOWS)
      CPINFOEXW *codepageInfo = nullptr;
      if (::Krys::Text::Windows::GetCodepageDescriptor(this->_codePage, &codepageInfo))
      {
        this->_codepageInfo = static_cast<void *>(codepageInfo);
      }
#endif
    }

    /// @brief Constructor that applies the given code page to the conversions, and sets the default-used
    /// replacement code point and replacement code unit for the underlying algorithm where possible.
    /// @param win32CodePage The code page. It is one of the allowed code page values
    /// (https://learn.microsoft.com/en-us/windows/win32/intl/code-page-identifiers) that are blessed by the
    /// WideCharToMultiByte and MultiByteToWideChar.
    /// @param[in] replacementCodePoint The replacement code point (a single 2-byte UTF-16 code unit) to
    /// be used with the underlying algorithm, where permissible.
    /// @param[in] replacementCodeUnit The replacement code unit (a single 1-byte
    /// `win32CodePage`-encoded code unit) to be used with the underlying algorithm, where permissible.
    constexpr basic_windows_code_page(uint32_t win32CodePage, uint16_t replacementCodePoint,
                                      int8_t replacementCodeUnit) noexcept
        : _codePage(win32CodePage), _replacementCodePoint(static_cast<code_point>(replacementCodePoint)),
          _replacementCodeUnit(static_cast<code_unit>(replacementCodeUnit)), _codepageInfo()
    {
#if KRYS_OS(WINDOWS)
      CPINFOEXW *codepageInfo = nullptr;
      if (::Krys::Text::Windows::GetCodepageDescriptor(this->_codePage, &codepageInfo))
      {
        this->_codepageInfo = static_cast<void *>(codepageInfo);
      }
#endif
    }

    /// @brief Decodes a single complete unit of information as code points and produces a result with the
    /// input and output ranges moved past what was successfully read and written; or, produces an error and
    /// returns the input and output ranges untouched.
    /// @param[in] input The input view to read code units from.
    /// @param[in] output The output view to write code points into.
    /// @param[in] errorHandler The error handler to invoke if encoding fails.
    /// @param[in, out] s The necessary state information. Most encodings have no state, but because this
    /// is effectively a runtime encoding and therefore it is important to preserve and manage this state.
    /// @returns A DecodeResult object that contains the reconstructed input range,
    /// reconstructed output range, error handler, and a reference to the passed-in state.
    template <typename TInput, typename TOutput, typename TErrorHandler>
    auto DecodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler, decode_state &s) const
    {
#if KRYS_OS(WINDOWS)
      using TSubInput = ::Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
      using TSubOutput = ::Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
      using TResult = DecodeResult<TSubInput, TSubOutput, decode_state>;

      auto inIt = std::ranges::cbegin(input);
      auto inLast = std::ranges::cend(input);
      if (inIt == inLast)
      {
        // an exhausted sequence is fine
        return TResult(TSubInput(std::move(inIt), std::move(inLast)), std::forward<TOutput>(output), s,
                       EncodingError::OK);
      }

      auto outIt = std::ranges::begin(output);
      auto outLast = std::ranges::end(output);

      WCHAR intermediateData[17] = {};
      const size_t initialIntermediateDataSize = std::ranges::size(intermediateData);
      CHAR inputReadData[17] = {*inIt};
      size_t inputReadSize = 1;
      const size_t initialInputReadDataSize = std::ranges::size(inputReadData);
      std::uint32_t codePageId = this->CodePage();
      auto Flags = ::Krys::Text::Windows::MultiByteToWidecharFlags(codePageId);
      size_t intermediateInputSize = 0;

      constexpr bool CallErrorHandler = !IsIgnorableErrorHandler<TErrorHandler>;
      for (; inIt != inLast; ++inputReadSize, std::ranges::advance(inIt))
      {
        if constexpr (CallErrorHandler)
        {
          if (inputReadSize > initialInputReadDataSize)
          {
            return std::forward<TErrorHandler>(errorHandler)(
              *this,
              TResult(TSubInput(std::move(inIt), std::move(inLast)),
                      TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::InvalidSequence),
              Span<code_unit>(inputReadData, inputReadSize - 1), Span<code_point>());
          }
        }
        int win32Error = ::MultiByteToWideChar(static_cast<UINT>(codePageId), Flags, inputReadData,
                                               static_cast<int>(inputReadSize), intermediateData,
                                               static_cast<int>(initialIntermediateDataSize));
        if constexpr (CallErrorHandler)
        {
          if (win32Error == 0)
          {
            DWORD lastWin32Error = ::GetLastError();
            if (lastWin32Error == ERROR_INSUFFICIENT_BUFFER)
            {
              return std::forward<TErrorHandler>(errorHandler)(
                *this,
                TResult(TSubInput(std::move(inIt), std::move(inLast)),
                        TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::InvalidSequence),
                Span<code_unit>(inputReadData, inputReadSize - 1), Span<code_point>());
            }
          }
          else
          {
            CPINFOEXW *info = static_cast<CPINFOEXW *>(this->_codepageInfo);
            if (::Krys::Text::Windows::MultiByteToWidecharConversionFailed(inputReadSize, inputReadData,
                                                                           intermediateData, info))
            {
              return std::forward<TErrorHandler>(errorHandler)(
                *this,
                TResult(TSubInput(std::move(inIt), std::move(inLast)),
                        TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::InvalidSequence),
                Span<code_unit>(inputReadData, inputReadSize - 1), Span<code_point>());
            }
            // we've finally completed one indivisible unit of work! Move to the next part.
            intermediateInputSize += static_cast<size_t>(win32Error);
            break;
          }
        }
        inputReadData[inputReadSize] = static_cast<CHAR>(*inIt);
      }

      using wutf16 = ::Krys::Text::Impl::UTF16With<void, WCHAR, code_point, false>;
      using TIntermediateState = decode_state_t<wutf16>;

      wutf16 intermediateEncoding {};
      TIntermediateState intermediateState {};
      ::Krys::Text::Handlers::ProgressHandler<!CallErrorHandler, wutf16> intermediateHandler {};
      code_point intermediateOutput[17] {};
      Span<code_point> outputBuffer(intermediateOutput);
      Span<const WCHAR> intermediateInputBuffer(intermediateData, intermediateInputSize);
      for (;;)
      {
        auto intermediateResult = intermediateEncoding.DecodeOne(intermediateInputBuffer, outputBuffer,
                                                                 intermediateHandler, intermediateState);
        if constexpr (CallErrorHandler)
        {
          if (intermediateResult.ErrorCode != EncodingError::OK)
          {
            return std::forward<TErrorHandler>(errorHandler)(
              *this,
              TResult(TSubInput(std::move(inIt), std::move(inLast)),
                      TSubOutput(std::move(outIt), std::move(outLast)), s, intermediateResult.ErrorCode),
              Span<code_unit>(), intermediateHandler.CodePointsProgress());
          }
        }
        if (!std::ranges::empty(intermediateResult.Input))
        {
          intermediateInputBuffer = Span<const WCHAR>(std::to_address(intermediateResult.Input.cbegin()),
                                                      std::to_address(intermediateResult.Input.cend()));
          outputBuffer = Span<code_point>(std::to_address(intermediateResult.Output.cbegin()),
                                          std::to_address(intermediateResult.Output.cend()));
          continue;
        }
      }
      auto intermediateOutputBegin = &intermediateOutput[0];
      auto intermediateOutputEnd = outputBuffer.data();
      for (auto intermediateOutputIt = intermediateOutputBegin;
           intermediateOutputIt != intermediateOutputEnd;)
      {
        if constexpr (CallErrorHandler)
        {
          if (outIt == outLast)
          {
            Span<code_unit> codePointLeftover(intermediateOutputIt, intermediateOutputEnd);
            return std::forward<TErrorHandler>(errorHandler)(
              *this,
              TResult(TSubInput(std::move(inIt), std::move(inLast)),
                      TSubOutput(std::move(outIt), std::move(outLast)), s,
                      EncodingError::InsufficientOutputSpace),
              Span<code_unit>(inputReadData, inputReadSize - 1), codePointLeftover);
          }
        }
        *outIt = *intermediateOutputIt;
        std::ranges::advance(outIt);
      }

      return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                     TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
#else
      (void)input;
      (void)output;
      (void)errorHandler;
      (void)s;
      static_assert(
        DependentFalse<TInput>,
        "This encoding type cannot be used on non-Windows platforms; please consider swapping this out for "
        "a different encoding type for a non-Windows platform.");
#endif
    }

    /// @brief Encodes a single complete unit of information as code units and produces a result with the
    /// input and output ranges moved past what was successfully read and written; or, produces an error and
    /// returns the input and output ranges untouched.
    /// @param[in] input The input view to read code units from.
    /// @param[in] output The output view to write code points into.
    /// @param[in] errorHandler The error handler to invoke if encoding fails.
    /// @param[in, out] s The necessary state information. Most encodings have no state, but because this
    /// is effectively a runtime encoding and therefore it is important to preserve and manage this state.
    /// @returns A EncodeResult object that contains the reconstructed input range,
    /// reconstructed output range, error handler, and a reference to the passed-in state.
    template <typename TInput, typename TOutput, typename TErrorHandler>
    auto EncodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler, encode_state &s) const
    {
#if KRYS_OS(WINDOWS)
      using TSubInput = ::Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
      using TSubOutput = ::Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
      using TResult = EncodeResult<TSubInput, TSubOutput, encode_state>;

      auto inIt = std::ranges::cbegin(input);
      auto inLast = std::ranges::cend(input);
      if (inIt == inLast)
      {
        // an exhausted sequence is fine
        return TResult(TSubInput(std::move(inIt), std::move(inLast)), std::forward<TOutput>(output), s,
                       EncodingError::OK);
      }

      auto outIt = std::ranges::begin(output);
      auto outLast = std::ranges::end(output);

      using wutf16 = ::Krys::Text::Impl::UTF16With<void, wchar_t, code_point, false>;
      using TIntermediateState = encode_state_t<wutf16>;

      wutf16 intermediateEncoding {};
      TIntermediateState intermediateState {};
      ::Krys::Text::Handlers::ProgressHandler<!CallErrorHandler, wutf16> intermediateHandler {};
      wchar_t wideIntermediate[8] {};
      Span<wchar_t> wideWriteBuffer(wideIntermediate);
      auto intermediateResult =
        intermediateEncoding.EncodeOne(TSubInput(std::move(inIt), std::move(inLast)), wideWriteBuffer,
                                       intermediateHandler, intermediateState);
      constexpr bool CallErrorHandler = !IsIgnorableErrorHandler<TErrorHandler>;
      if constexpr (CallErrorHandler)
      {
        if (intermediateResult.ErrorCode != EncodingError::OK)
        {
          return std::forward<TErrorHandler>(errorHandler)(
            *this,
            TResult(std::move(intermediateResult.Input), std::forward<TOutput>(output), s,
                    intermediateResult.ErrorCode),
            intermediateHandler.CodePointsProgress(), Span<code_unit>());
        }
      }
      constexpr const std::size_t stateCountMax = 17;
      CHAR intermediateOutput[stateCountMax] {};
      BOOL defaultCharUsed = false;
      CHAR defaultChar = '?';
      Span<const wchar_t> wideReadBuffer(wideIntermediate, intermediateResult.Output.data());
      std::uint32_t codePageId = this->CodePage();
      auto usedDefaults =
        ::Krys::Text::Windows::WidecharToMultiByteUsedChar(codePageId, &defaultChar, &defaultCharUsed);
      int win32Error =
        ::WideCharToMultiByte(static_cast<UINT>(codePageId), usedDefaults.Flags, wideReadBuffer.data(),
                              static_cast<int>(wideReadBuffer.size()), intermediateOutput, stateCountMax,
                              usedDefaults.DefaultChar, usedDefaults.DefaultCharUsed);
      if constexpr (CallErrorHandler)
      {
        if (win32Error == 0)
        {
          DWORD lastWin32Error = ::GetLastError();
          if (lastWin32Error == ERROR_INSUFFICIENT_BUFFER)
          {
            // technically, this should never happen.
            return std::forward<TErrorHandler>(errorHandler)(
              *this,
              TResult(std::move(intermediateResult.Input), std::forward<TOutput>(output), s,
                      EncodingError::InvalidSequence),
              intermediateHandler.CodePointsProgress(), Span<code_unit>());
          }
        }
        else
        {
          CPINFOEXW *info = static_cast<CPINFOEXW *>(this->_codepageInfo);
          if (::Krys::Text::Windows::WidecharToMultiByteConversionFailed(
                wideIntermediate, static_cast<size_t>(win32Error), intermediateOutput, info))
          {
            return std::forward<TErrorHandler>(errorHandler)(
              *this,
              TResult(std::move(intermediateResult.Input), std::forward<TOutput>(output), s,
                      EncodingError::InvalidSequence),
              intermediateHandler.CodePointsProgress(), Span<code_unit>());
          }
        }
      }
      auto intermediateOutputBegin = intermediateOutput;
      auto intermediateOutputEnd = intermediateOutputBegin + win32Error;
      for (auto intermediateOutputIt = intermediateOutputBegin;
           intermediateOutputIt != intermediateOutputEnd;)
      {
        if constexpr (CallErrorHandler)
        {
          if (outIt == outLast)
          {
            Span<code_unit> codeUnitProgress(intermediateOutputIt, intermediateOutputEnd);
            return std::forward<TErrorHandler>(errorHandler)(
              *this,
              TResult(std::move(intermediateResult.Input), TSubOutput(std::move(outIt), std::move(outLast)),
                      s, EncodingError::InsufficientOutputSpace),
              intermediateHandler.CodePointsProgress(), codeUnitProgress);
          }
        }
        *outIt = *intermediateOutputIt;
        std::ranges::advance(outIt);
      }
      return TResult(std::move(intermediateResult.Input), TSubOutput(std::move(outIt), std::move(outLast)), s,
                     intermediateResult.ErrorCode);
#else
      (void)input;
      (void)output;
      (void)errorHandler;
      (void)s;
      static_assert(
        DependentFalse<TInput>,
        "This encoding type cannot be used on non-Windows platforms; please consider swapping this out for "
        "a different encoding type for a non-Windows platform.");
#endif
    }

    constexpr uint32_t CodePage() const noexcept
    {
      return this->_codePage;
    }

    constexpr std::optional<code_point> MaybeDefaultReplacementCodePoint() const noexcept
    {
      return this->_replacementCodePoint;
    }

    constexpr std::optional<code_unit> MaybeDefaultReplacementCodeUnit() const noexcept
    {
      return this->_replacementCodeUnit;
    }

  private:
    std::optional<code_point> _replacementCodePoint;
    std::optional<code_unit> _replacementCodeUnit;
    void *_codepageInfo;
    uint32 _codePage;
  };

  /// @brief The encoding representing a Windows conversion using WideCharToMultiByte (encode) and
  /// MultiByteToWideChar (decode) conversion sequences.
  /// @remarks This is slow due to the bad design of WideCharToMultiByte/MultiByteToWideChar, but it does
  /// guarantee access to all of the wide variety of legacy encodings Windows supports. If possible, a
  /// different named encoding type should be used to avoid the performance penalties that comes from API
  /// limitations of this encoding object. Occasionally, for correct text, this API limitations can be
  /// mitigated when using the bulk APIs: for the single-conversion APIs that do not perform bulk
  /// conversion, the performance penalty is endemic.
  using windows_code_page = basic_windows_code_page<char, UnicodeCodePoint>;
}
