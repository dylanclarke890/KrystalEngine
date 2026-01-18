#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/Range.hpp"
#include "Krystal.Lib/Ranges/Reconstruct.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/Concepts.hpp"
#include "Krystal.Text/Decode/DecodeResult.hpp"
#include "Krystal.Text/Encode/EncodeResult.hpp"
#include "Krystal.Text/EncodingError.hpp"
#include "Krystal.Text/Handlers/PassThroughHandler.hpp"
#include "Krystal.Text/IsUnicodeCodePoint.hpp"
#include "Krystal.Text/SkipInputError.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/Unicode.hpp"
#include <climits>
#include <cstddef>
#include <utility>

namespace Krys::Text::Handlers
{
  namespace Impl
  {
    template <typename TEncoding, typename TReplacement, typename TResult>
    constexpr TResult &&WriteDirect(const TEncoding &, TReplacement &&replacement, TResult &&result) noexcept
    {
      using TSubOutput = decltype(result.Output);

      auto outIt = Krys::Ranges::begin(result.Output);
      auto outLast = Krys::Ranges::end(result.Output);
      if (outIt == outLast) // BAIL
      {
        return std::forward<TResult>(result);
      }

      if (Krys::Ranges::empty(replacement)) // empty range, everything is okay
      {
        result.ErrorCode = EncodingError::OK;
        return std::forward<TResult>(result);
      }

      for (const auto &element : std::forward<TReplacement>(replacement))
      {
        if (outIt == outLast)
        {
          result.Output = TSubOutput(std::move(outIt), std::move(outLast));
          return std::forward<TResult>(result);
        }
        (*outIt) = element;
        ++outIt;
      }

      result.Output = TSubOutput(std::move(outIt), std::move(outLast));
      result.ErrorCode = EncodingError::OK;
      return std::forward<TResult>(result);
    }

    template <typename TEncoding, typename TResult>
    constexpr TResult &&WriteStaticCodePointsDirect(const TEncoding &encoding, TResult &&result) noexcept
    {
      using TInputCodePoint = code_point_t<TEncoding>;
      if constexpr (HasReplacementCodePoints<TEncoding, decltype(result.State) &>)
      {
        return Impl::WriteDirect(encoding, encoding.ReplacementCodePoints(result.State),
                                 std::forward<TResult>(result));
      }
      else if constexpr (HasReplacementCodePoints<TEncoding>)
      {
        return Impl::WriteDirect(encoding, encoding.ReplacementCodePoints(), std::forward<TResult>(result));
      }
      else if constexpr (HasMaybeReplacementCodePoints<TEncoding, decltype(result.State) &>)
      {
        decltype(auto) maybeCodePoints = encoding.MaybeReplacementCodePoints(result.State);
        if (maybeCodePoints)
        {
          return Impl::WriteDirect(encoding, *std::forward<decltype(maybeCodePoints)>(maybeCodePoints),
                                   std::forward<TResult>(result));
        }
        else
        {
          return std::forward<TResult>(result);
        }
      }
      else if constexpr (HasMaybeReplacementCodePoints<TEncoding>)
      {
        decltype(auto) maybeCodePoints = encoding.MaybeReplacementCodePoints();
        if (maybeCodePoints)
        {
          return Impl::WriteDirect(encoding, *std::forward<decltype(maybeCodePoints)>(maybeCodePoints),
                                   std::forward<TResult>(result));
        }
        else
        {
          return std::forward<TResult>(result);
        }
      }
      else if constexpr (IsUnicodeCodePoint<TInputCodePoint>)
      {
        constexpr auto replacements = ::Krys::Text::Unicode::ReplacementAsArray<TInputCodePoint>;
        return Impl::WriteDirect(encoding, replacements, std::forward<TResult>(result));
      }
      else
      {
        static_assert(DependentFalse<TEncoding>,
                      "There is no logical replacement code points to insert into the stream on failure for "
                      "the "
                      "specified encoding type.");
      }
    }

    template <typename TEncoding, typename TResult>
    constexpr TResult &&WriteStaticCodeUnitsDirect(const TEncoding &encoding, TResult &&result) noexcept
    {
      using TInputCodeUnit = code_unit_t<TEncoding>;
      if constexpr (HasReplacementCodeUnits<TEncoding, decltype(result.State) &>)
      {
        return Impl::WriteDirect(encoding, encoding.ReplacementCodeUnits(result.State),
                                 std::forward<TResult>(result));
      }
      else if constexpr (HasReplacementCodeUnits<TEncoding>)
      {
        return Impl::WriteDirect(encoding, encoding.ReplacementCodeUnits(), std::forward<TResult>(result));
      }
      else if constexpr (HasMaybeReplacementCodeUnits<TEncoding, decltype(result.State) &>)
      {
        decltype(auto) maybeCodeUnits = encoding.MaybeReplacementCodeUnits(result.State);
        if (maybeCodeUnits)
        {
          return Impl::WriteDirect(encoding, *std::forward<decltype(maybeCodeUnits)>(maybeCodeUnits),
                                   std::forward<TResult>(result));
        }
        else
        {
          return std::forward<TResult>(result);
        }
      }
      else if constexpr (HasMaybeReplacementCodeUnits<TEncoding>)
      {
        decltype(auto) maybeCodeUnits = encoding.MaybeReplacementCodeUnits();
        if (maybeCodeUnits)
        {
          return Impl::WriteDirect(encoding, *std::forward<decltype(maybeCodeUnits)>(maybeCodeUnits),
                                   std::forward<TResult>(result));
        }
        else
        {
          return std::forward<TResult>(result);
        }
      }
      else if constexpr (sizeof(TInputCodeUnit) >= sizeof(char))
      {
        constexpr auto replacements = ::Krys::Text::Unicode::ReplacementAsArray<TInputCodeUnit>;
        return Impl::WriteDirect(encoding, replacements, std::forward<TResult>(result));
      }
      else
      {
        static_assert(
          DependentFalse<TEncoding>,
          "There is no logical replacement code units to insert into the stream on failure for the "
          "specified encoding type.");
      }
    }

    template <typename TEncoding, typename TState>
    constexpr std::size_t FillReplacementCodePointStatic(
      const TEncoding &encoding, TState &state,
      code_point_t<TEncoding> (&replacementCodePoints)[MaxCodePoints<TEncoding>])
    {
      using TInputCodePoint = code_point_t<TEncoding>;
      if constexpr (HasReplacementCodePoints<TEncoding, TState &>)
      {
        std::size_t replacementIndex = 0;
        for (const auto &element : encoding.ReplacementCodePoints(state))
        {
          replacementCodePoints[replacementIndex] = static_cast<TInputCodePoint>(element);
          ++replacementIndex;
        }
        return replacementIndex;
      }
      else if constexpr (HasReplacementCodePoints<TEncoding>)
      {
        std::size_t replacementIndex = 0;
        for (const auto &element : encoding.ReplacementCodePoints())
        {
          replacementCodePoints[replacementIndex] = static_cast<TInputCodePoint>(element);
          ++replacementIndex;
        }
        return replacementIndex;
      }
      else if constexpr (HasMaybeReplacementCodePoints<TEncoding, TState &>)
      {
        std::size_t replacementIndex = 0;
        decltype(auto) maybeCodePoints = encoding.MaybeReplacementCodePoints(state);
        if (maybeCodePoints)
        {
          decltype(auto) codePoints = *std::forward<decltype(maybeCodePoints)>(maybeCodePoints);
          for (const auto &element : codePoints)
          {
            replacementCodePoints[replacementIndex] = static_cast<TInputCodePoint>(element);
            ++replacementIndex;
          }
        }
        return replacementIndex;
      }
      else if constexpr (HasMaybeReplacementCodePoints<TEncoding>)
      {
        std::size_t replacementIndex = 0;
        decltype(auto) maybeCodePoints = encoding.MaybeReplacementCodePoints();
        if (maybeCodePoints)
        {
          decltype(auto) codePoints = *std::forward<decltype(maybeCodePoints)>(maybeCodePoints);
          for (const auto &element : codePoints)
          {
            replacementCodePoints[replacementIndex] = static_cast<TInputCodePoint>(element);
            ++replacementIndex;
          }
        }
        return replacementIndex;
      }
      else if constexpr (IsUnicodeCodePoint<TInputCodePoint>)
      {
        replacementCodePoints[0] = ::Krys::Text::Unicode::Replacement<TInputCodePoint>;
        return 1;
      }
      else
      {
        static_assert(DependentFalse<TEncoding>,
                      "There is no logical replacement code points to insert into the stream on failure for "
                      "the "
                      "specified encoding type.");
      }
    }

    template <typename TEncoding, typename TState>
    constexpr std::size_t
      FillReplacementCodeUnitStatic(const TEncoding &encoding, TState &state,
                                    code_unit_t<TEncoding> (&replacementCodeUnits)[MaxCodeUnits<TEncoding>])
    {
      using TInputCodeUnit = code_unit_t<TEncoding>;
      if constexpr (HasReplacementCodeUnits<TEncoding, TState &>)
      {
        std::size_t replacementIndex = 0;
        for (const auto &element : encoding.ReplacementCodeUnits(state))
        {
          replacementCodeUnits[replacementIndex] = static_cast<TInputCodeUnit>(element);
          ++replacementIndex;
        }
        return replacementIndex;
      }
      else if constexpr (HasReplacementCodeUnits<TEncoding>)
      {
        std::size_t replacementIndex = 0;
        for (const auto &element : encoding.ReplacementCodeUnits())
        {
          replacementCodeUnits[replacementIndex] = static_cast<TInputCodeUnit>(element);
          ++replacementIndex;
        }
        return replacementIndex;
      }
      else if constexpr (HasMaybeReplacementCodeUnits<TEncoding, TState &>)
      {
        std::size_t replacementIndex = 0;
        decltype(auto) maybeCodeUnits = encoding.MaybeReplacementCodeUnits(state);
        if (maybeCodeUnits)
        {
          decltype(auto) codeUnits = *std::forward<decltype(maybeCodeUnits)>(maybeCodeUnits);
          for (const auto &element : codeUnits)
          {
            replacementCodeUnits[replacementIndex] = static_cast<TInputCodeUnit>(element);
            ++replacementIndex;
          }
        }
        return replacementIndex;
      }
      else if constexpr (HasMaybeReplacementCodeUnits<TEncoding>)
      {
        std::size_t replacementIndex = 0;
        decltype(auto) maybeCodeUnits = encoding.MaybeReplacementCodeUnits();
        if (maybeCodeUnits)
        {
          decltype(auto) codeUnits = *std::forward<decltype(maybeCodeUnits)>(maybeCodeUnits);
          for (const auto &element : codeUnits)
          {
            replacementCodeUnits[replacementIndex] = static_cast<TInputCodeUnit>(element);
            ++replacementIndex;
          }
        }
        return replacementIndex;
      }
      else if constexpr (IsUnicodeCodePoint<TInputCodeUnit> || sizeof(TInputCodeUnit) >= sizeof(char))
      {
        replacementCodeUnits[0] = ::Krys::Text::Unicode::Replacement<TInputCodeUnit>;
        return 1;
      }
      else
      {
        static_assert(
          DependentFalse<TEncoding>,
          "There is no logical replacement code units to insert into the stream on failure for the "
          "specified encoding type.");
      }
    }
  }

  enum class ReplacementPolicy
  {
    Invalid = 1 << 0,
    Incomplete = 1 << 1,
  };
}

namespace Krys
{
  template <>
  inline constexpr bool EnableEnumFlags<::Krys::Text::Handlers::ReplacementPolicy> = true;
}

namespace Krys::Text::Handlers
{
  /// @brief An error handler that replaces bad code points and code units with a chosen code point / code
  /// unit sequence.
  /// @remarks This class hooks into the encodings passed as the first parameter to the error handling
  /// functions to see if they define either `ReplacementCodePoints()` or `ReplacementCodeUnits()`
  /// function. If so, they will call them and use the returned contiguous range to isnert code points or
  /// code units into the function. If neither of these exist, then it checks for a definition of a
  /// `MaybeReplacementCodePoints()` or a @c MaybeReplacementCodeUnits() function. If either is
  /// present, they are expected to return a `std::optional` of a contiguous range. If it is engaged (the
  /// `std::optional` is filled) it will be used. Otherwise, if it is not engaged, then it will explicitly
  /// fall back to attempt to insert the default replacement character `U`+FFFD (<tt>U'�'</tt>) or <tt>?</tt>
  /// character. If the output is out of room for the desired object, then nothing will be inserted at all.
  template <ReplacementPolicy Policy = ReplacementPolicy::Invalid | ReplacementPolicy::Incomplete>
  class ReplacementHandler
  {
  private:
    template <typename TEncoding, typename TInput, typename TOutput, typename TState>
    constexpr static auto EncodeReplace(const TEncoding &encoding,
                                        EncodeResult<TInput, TOutput, TState> result) noexcept
    {
      using TCodeUnit = code_unit_t<TEncoding>;
      if constexpr (HasReplacementCodeUnits<TEncoding, TState &>)
      {
        return Impl::WriteDirect(encoding, encoding.ReplacementCodeUnits(result.State), std::move(result));
      }
      else if constexpr (HasReplacementCodeUnits<TEncoding>)
      {
        return Impl::WriteDirect(encoding, encoding.ReplacementCodeUnits(), std::move(result));
      }
      else if constexpr (!(HasMaybeReplacementCodePoints<TEncoding, TState &>
                           || HasMaybeReplacementCodePoints<TEncoding>)
                         && IsUnicodeCodePoint<TCodeUnit>)
      {
        constexpr auto replacements = ::Krys::Text::Unicode::ReplacementAsArray<TCodeUnit>;
        return Impl::WriteDirect(encoding, replacements, std::move(result));
      }
      else
      {
        if constexpr (HasMaybeReplacementCodeUnits<TEncoding, TState &>)
        {
          auto maybeDirectReplacement = encoding.MaybeReplacementCodeUnits(result.State);
          if (maybeDirectReplacement)
          {
            const auto &directReplacement = *maybeDirectReplacement;
            return Impl::WriteDirect(encoding, directReplacement, std::move(result));
          }
        }
        else if constexpr (HasMaybeReplacementCodeUnits<TEncoding>)
        {
          auto maybeDirectReplacement = encoding.MaybeReplacementCodeUnits();
          if (maybeDirectReplacement)
          {
            const auto &directReplacement = *maybeDirectReplacement;
            return Impl::WriteDirect(encoding, directReplacement, std::move(result));
          }
        }

        using TInputCodePoint = code_point_t<TEncoding>;

        TInputCodePoint replacement[MaxCodePoints<TEncoding>] {};
        std::size_t replacementSize = 0;

        if constexpr (HasReplacementCodePoints<TEncoding, TState &>)
        {
          auto replacementCodeUnits = encoding.ReplacementCodePoints(result.State);
          for (const auto &element : replacementCodeUnits)
          {
            replacement[replacementSize] = element;
            ++replacementSize;
          }
        }
        else if constexpr (HasReplacementCodePoints<TEncoding>)
        {
          auto replacementCodeUnits = encoding.ReplacementCodePoints();
          for (const auto &element : replacementCodeUnits)
          {
            replacement[replacementSize] = element;
            ++replacementSize;
          }
        }
        else
        {
          replacementSize = Impl::FillReplacementCodePointStatic(encoding, result.State, replacement);
        }

        const Span<const TInputCodePoint> replacementRange(replacement, replacementSize);

        ::Krys::Text::Handlers::PassThroughHandler handler {};
        encode_state_t<TEncoding> state = ::Krys::Text::CopyEncodeStateWith(encoding, result.State);

        auto encodingResult = encoding.EncodeOne(replacementRange, std::move(result.Output), handler, state);
        result.Output = std::move(encodingResult.Output);

        if (encodingResult.ErrorCode != EncodingError::OK)
        {
          // we can't even encode a single code unit into the stream... report error and bail
          return result;
        }

        result.ErrorCode = EncodingError::OK;
        return result;
      }
    }

    template <typename TEncoding, typename TInput, typename TOutput, typename TState>
    constexpr static auto DecodeReplace(const TEncoding &encoding,
                                        DecodeResult<TInput, TOutput, TState> result) noexcept
    {
      using TCodePoint = code_point_t<TEncoding>;

      if constexpr (HasReplacementCodePoints<TEncoding, TState &>)
      {
        return Impl::WriteDirect(encoding, encoding.ReplacementCodePoints(result.State), std::move(result));
      }
      else if constexpr (HasReplacementCodePoints<TEncoding>)
      {
        return Impl::WriteDirect(encoding, encoding.ReplacementCodePoints(), std::move(result));
      }
      else if constexpr (!(HasMaybeReplacementCodePoints<TEncoding, TState &>
                           || HasMaybeReplacementCodePoints<TEncoding>)
                         && IsUnicodeCodePoint<TCodePoint>)
      {
        constexpr auto replacements = ::Krys::Text::Unicode::ReplacementAsArray<TCodePoint>;
        return Impl::WriteDirect(encoding, replacements, std::move(result));
      }
      else
      {
        if constexpr (HasMaybeReplacementCodePoints<TEncoding, TState &>)
        {
          auto maybeDirectReplacement = encoding.MaybeReplacementCodePoints(result.State);
          if (maybeDirectReplacement)
          {
            const auto &directReplacement = *maybeDirectReplacement;
            return Impl::WriteDirect(encoding, directReplacement, std::move(result));
          }
        }
        else if constexpr (HasMaybeReplacementCodePoints<TEncoding>)
        {
          auto maybeDirectReplacement = encoding.MaybeReplacementCodePoints();
          if (maybeDirectReplacement)
          {
            const auto &directReplacement = *maybeDirectReplacement;
            return Impl::WriteDirect(encoding, directReplacement, std::move(result));
          }
        }

        using TInputCodeUnit = code_unit_t<TEncoding>;
        TInputCodeUnit replacement[MaxCodeUnits<TEncoding>] {};
        std::size_t replacementSize = 0;

        if constexpr (HasReplacementCodeUnits<TEncoding, TState &>)
        {
          auto replacementCodeUnits = encoding.ReplacementCodeUnits(result.State);
          for (const auto &element : replacementCodeUnits)
          {
            replacement[replacementSize] = element;
            ++replacementSize;
          }
        }
        else if constexpr (HasReplacementCodeUnits<TEncoding>)
        {
          auto replacementCodeUnits = encoding.ReplacementCodeUnits();
          for (const auto &element : replacementCodeUnits)
          {
            replacement[replacementSize] = element;
            ++replacementSize;
          }
        }
        else
        {
          replacementSize = Impl::FillReplacementCodeUnitStatic(encoding, result.State, replacement);
        }

        const Span<const TInputCodeUnit> replacementRange(replacement, replacementSize);
        decode_state_t<TEncoding> state = ::Krys::Text::CopyDecodeStateWith(encoding, result.State);
        // TODO: We copy the above state but it is not actually used in the decode operation below?
        ::Krys::Text::Handlers::PassThroughHandler handler {};
        auto decodingResult =
          encoding.DecodeOne(replacementRange, std::move(result.Output), handler, result.State);
        result.Output = std::move(decodingResult.Output);
        if (decodingResult.ErrorCode != EncodingError::OK)
        {
          // we can't even decode a single code unit into the stream... report error and bail
          return result;
        }

        result.ErrorCode = EncodingError::OK;
        return result;
      }
    }

  public:
    /// @brief The function call for inserting replacement code units at the point of failure, before
    /// returning flow back to the caller of the encode operation.
    /// @param[in] encoding The Encoding that experienced the error.
    /// @param[in] result The current state of the encode operation.
    /// @param[in] inputProgress How much input was (potentially irreversibly) read from the input range
    /// before undergoing the attempted encode operation.
    /// @param[in] outputProgress How much output was (potentially irreversibly) written to the output
    /// range before undergoing the attempted encode operation.
    template <typename TEncoding, typename TInput, typename TOutput, typename TState, typename TInputProgress,
              typename TOutputProgress>
    constexpr auto operator()(const TEncoding &encoding, EncodeResult<TInput, TOutput, TState> result,
                              const TInputProgress &inputProgress,
                              const TOutputProgress &outputProgress) const
      noexcept(NoThrowSkipInputError<const TEncoding &, EncodeResult<TInput, TOutput, TState>,
                                     const TInputProgress &, const TOutputProgress &>)
    {
      if (result.ErrorCode == EncodingError::InsufficientOutputSpace)
      {
        // BAIL
        return result;
      }

      if constexpr (!HasFlag(Policy, ReplacementPolicy::Invalid))
      {
        if (result.ErrorCode == EncodingError::InvalidSequence)
        {
          return result; // BAIL
        }
      }

      if constexpr (!HasFlag(Policy, ReplacementPolicy::Incomplete))
      {
        if (result.ErrorCode == EncodingError::IncompleteSequence)
        {
          return result; // BAIL
        }
      }

      return ::Krys::Text::SkipInputError(encoding, this->EncodeReplace(encoding, std::move(result)),
                                          inputProgress, outputProgress);
    }

    /// @brief The function call for inserting replacement code points at the point of failure, before
    /// returning flow back to the caller of the decode operation.
    ///
    /// @param[in] encoding The Encoding that experienced the error.
    /// @param[in] result The current state of the encode operation.
    /// @param[in] inputProgress How much input was (potentially irreversibly) read from the input range
    /// before undergoing the attempted encode operation.
    /// @param[in] outputProgress How much output was (potentially irreversibly) written to the output
    /// range before undergoing the attempted encode operation.
    template <typename TEncoding, typename TInput, typename TOutput, typename TState, typename TInputProgress,
              typename TOutputProgress>
    constexpr auto operator()(const TEncoding &encoding, DecodeResult<TInput, TOutput, TState> result,
                              const TInputProgress &inputProgress,
                              const TOutputProgress &outputProgress) const
      noexcept(NoThrowSkipInputError<const TEncoding &, DecodeResult<TInput, TOutput, TState>,
                                     const TInputProgress &, const TOutputProgress &>)
    {
      if (result.ErrorCode == EncodingError::InsufficientOutputSpace)
      {
        return result; // BAIL
      }

      if constexpr (!HasFlag(Policy, ReplacementPolicy::Invalid))
      {
        if (result.ErrorCode == EncodingError::InvalidSequence)
        {
          return result; // BAIL
        }
      }

      if constexpr (!HasFlag(Policy, ReplacementPolicy::Incomplete))
      {
        if (result.ErrorCode == EncodingError::IncompleteSequence)
        {
          return result; // BAIL
        }
      }

      return ::Krys::Text::SkipInputError(encoding, this->DecodeReplace(encoding, std::move(result)),
                                          inputProgress, outputProgress);
    }
  };

  /// @brief A convenience variable for passing the ReplacementHandler handler to functions.
  constexpr inline ReplacementHandler Replacement = {};

  /// @brief An error handler that replaces bad code points and code units with a chosen code point / code
  /// unit sequence provided directly in the constructor rather than by the encoding object.
  /// @tparam TCodePointRange The code point range type to insert.
  /// @tparam TCodeUnitRange The code unit range type to insert.
  /// @remarks Unlike ReplacementHandler, this class does not hook into anything and will simply insert the
  /// desired sequences of code units or code points into the output with no attempt at negotiating anything.
  /// It will store the two sequences given as the range types provided in the template arguments.
  template <typename TCodePointRange, typename TCodeUnitRange>
  class ReplacementOfHandler
  {
  public:
    /// @brief The code units that will be inserted when the error handler is called for encoding
    /// operations.
    KRYS_NO_UNIQUE_ADDRESS TCodePointRange CodePointReplacement;

    /// @brief The code units that will be inserted when the error handler is called for decoding
    /// operations.
    KRYS_NO_UNIQUE_ADDRESS TCodeUnitRange CodeUnitReplacement;

    template <typename TCodePoints, typename TCodeUnits>
    constexpr ReplacementOfHandler(TCodePoints &&codePoints, TCodeUnits &&codeUnits) noexcept(
      NoThrowConstructible<TCodePointRange, TCodePoints> && NoThrowConstructible<TCodeUnitRange, TCodeUnits>)
        : CodePointReplacement(std::forward<TCodePoints>(codePoints)),
          CodeUnitReplacement(std::forward<TCodeUnits>(codeUnits))
    {
    }

    /// @brief The function call for inserting replacement code units at the point of failure, before
    /// returning flow back to the caller of the encode operation.
    /// @param[in] encoding The Encoding that experienced the error.
    /// @param[in] result The current state of the encode operation.
    /// @param[in] inputProgress How much input was (potentially irreversibly) read from the input range
    /// before undergoing the attempted encode operation.
    /// @param[in] outputProgress How much output was (potentially irreversibly) written to the output
    /// range before undergoing the attempted encode operation.
    template <typename TEncoding, typename TInput, typename TOutput, typename TState, typename TInputProgress,
              typename TOutputProgress>
    constexpr auto operator()(const TEncoding &encoding, EncodeResult<TInput, TOutput, TState> result,
                              const TInputProgress &inputProgress,
                              const TOutputProgress &outputProgress) const
      noexcept(NoThrowSkipInputError<const TEncoding &, EncodeResult<TInput, TOutput, TState>,
                                     const TInputProgress &, const TOutputProgress &>)
    {
      if (result.ErrorCode == EncodingError::InsufficientOutputSpace)
      {
        return result; // BAIL
      }
      auto copyResult = ::Krys::Ranges::Impl::Copy(CodeUnitReplacement, std::move(result.Output));
      result.Output = std::move(copyResult.Out);
      return ::Krys::Text::SkipInputError(encoding, std::move(result), inputProgress, outputProgress);
    }

    /// @brief The function call for inserting replacement code points at the point of failure, before
    /// returning flow back to the caller of the decode operation.
    /// @param[in] encoding The Encoding that experienced the error.
    /// @param[in] result The current state of the encode operation.
    /// @param[in] inputProgress How much input was (potentially irreversibly) read from the input range
    /// before undergoing the attempted encode operation.
    /// @param[in] outputProgress How much output was (potentially irreversibly) written to the output
    /// range before undergoing the attempted encode operation.
    template <typename TEncoding, typename TInput, typename TOutput, typename TState, typename TInputProgress,
              typename TOutputProgress>
    constexpr auto operator()(const TEncoding &encoding, DecodeResult<TInput, TOutput, TState> result,
                              const TInputProgress &inputProgress,
                              const TOutputProgress &outputProgress) const
      noexcept(NoThrowSkipInputError<const TEncoding &, DecodeResult<TInput, TOutput, TState>,
                                     const TInputProgress &, const TOutputProgress &>)
    {
      if (result.ErrorCode == EncodingError::InsufficientOutputSpace)
      {
        return result; // BAIL
      }

      auto copyResult = ::Krys::Ranges::Impl::Copy(CodePointReplacement, std::move(result.Output));
      result.Output = std::move(copyResult.Out);
      return ::Krys::Text::SkipInputError(encoding, std::move(result), inputProgress, outputProgress);
    }
  };

  template <typename TCodePoints, typename TCodeUnits>
  ReplacementOfHandler(TCodePoints &&, TCodeUnits &&)
    -> ReplacementOfHandler<remove_ref_t<TCodePoints>, remove_ref_t<TCodeUnits>>;
}
