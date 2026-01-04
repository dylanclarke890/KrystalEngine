#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Ranges/ADL.hpp"
#include "Krystal.Lib/Ranges/Algorithm.hpp"
#include "Krystal.Lib/Ranges/BlackholeIterator.hpp"
#include "Krystal.Lib/Ranges/Subrange.hpp"
#include "Krystal.Lib/Ranges/Unbounded.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Utils/Tag.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/DecodeResult.hpp"
#include "Krystal.Text/EncodeResult.hpp"
#include "Krystal.Text/Impl/EncodingRange.hpp"
#include "Krystal.Text/Impl/IsLossless.hpp"
#include "Krystal.Text/IsIgnorableErrorHandler.hpp"
#include "Krystal.Text/State.hpp"
#include <algorithm>
#include <array>

namespace Krys::Impl
{
  enum class Consume : uchar
  {
    No = 0,
    EmbraceTheVoid = 1
  };

  enum class Transaction : uchar
  {
    Encode = 0,
    Decode = 1
  };

  template <Consume ConsumeIntoTheNothingness, typename TEncoding, typename TInput, typename TOutput,
            typename TErrorHandler, typename TState>
  constexpr auto BasicDecodeOne(TInput &&input, TEncoding &&encoding, TOutput &&output,
                                TErrorHandler &errorHandler, TState &state)
  {
    using TUOutput = remove_cvref_t<TOutput>;
    using TUEncoding = remove_cvref_t<TEncoding>;

    if constexpr (ConsumeIntoTheNothingness == Consume::EmbraceTheVoid
                  && DecodeRangeCategoryOutput<TUEncoding>)
    {
      (void)output;
      using TBlackhole = Krys::Ranges::UnboundedView<Krys::Ranges::BlackholeIterator>;
      TBlackhole outputRange(Krys::Ranges::BlackholeIterator {});
      return encoding.DecodeOne(std::forward<TInput>(input), outputRange, errorHandler, state);
    }
    else if constexpr ((!Krys::Ranges::ContiguousRange<TUOutput>
                        || Krys::Ranges::InputOrOutputRangeExact<TUOutput>)
                       && ContiguousDecodeRangeCategory<TUEncoding>)
    {
      code_point_t<TUEncoding> intermediateOutputStorage[MaxCodePoints<TUEncoding>];
      Span<code_point_t<TUEncoding>, MaxCodePoints<TUEncoding>> intermediateOutput(intermediateOutputStorage);
      auto intermediateResult =
        encoding.DecodeOne(std::forward<TInput>(input), intermediateOutput, errorHandler, state);
      using TReconstructedResult =
        DecodeResult<decltype(intermediateResult.Input), Krys::Ranges::range_reconstruct_t<TOutput>, TState>;
      if (intermediateResult.ErrorCode != EncodingError::OK)
      {
        auto intermediateLast = intermediateResult.Output.data();
        auto outIt = Krys::Ranges::begin(std::move(output));
        auto outLast = Krys::Ranges::end(std::move(output));
        for (auto intermediateIt = intermediateOutput.data(); intermediateIt != intermediateLast;)
        {
          *outIt = *intermediateIt;
          ++intermediateIt;
          ++outIt;
        }
        return TReconstructedResult(std::move(intermediateResult.Input),
                                    Krys::Ranges::reconstruct(std::in_place_type<TUOutput>,
                                                              std::forward<TOutput>(output), std::move(outIt),
                                                              std::move(outLast)),
                                    state, intermediateResult.ErrorCode, intermediateResult.ErrorCount);
      }
      return TReconstructedResult(std::move(intermediateResult.Input), std::forward<TOutput>(output), state,
                                  intermediateResult.ErrorCode, intermediateResult.ErrorCount);
    }
    else
    {
      return encoding.DecodeOne(std::forward<TInput>(input), std::forward<TOutput>(output), errorHandler,
                                state);
    }
  }

  template <Consume ConsumeIntoTheNothingness, typename TEncoding, typename TInput, typename TOutput,
            typename TErrorHandler, typename TState>
  constexpr auto BasicEncodeOne(TInput &&input, TEncoding &&encoding, TOutput &&output,
                                TErrorHandler &errorHandler, TState &state)
  {
    using TUOutput = remove_cvref_t<TOutput>;
    using TUEncoding = remove_cvref_t<TEncoding>;

    if constexpr (ConsumeIntoTheNothingness == Consume::EmbraceTheVoid
                  && EncodeRangeCategoryOutput<TUEncoding>)
    {
      (void)output;
      using TBlackhole = Krys::Ranges::UnboundedView<Krys::Ranges::BlackholeIterator>;
      TBlackhole outputRange(Krys::Ranges::BlackholeIterator {});
      return encoding.EncodeOne(std::forward<TInput>(input), outputRange, errorHandler, state);
    }
    else if constexpr ((!Krys::Ranges::ContiguousRange<TUOutput>
                        || Krys::Ranges::InputOrOutputRangeExact<TUOutput>)
                       && ContiguousEncodeRangeCategory<TUEncoding>)
    {
      constexpr std::size_t intermediateMax = MaxCodeUnits<TUEncoding>;
      code_unit_t<TUEncoding> intermediateOutputStorage[intermediateMax];
      Span<code_unit_t<TUEncoding>, intermediateMax> intermediateOutput(intermediateOutputStorage);
      auto intermediateResult =
        encoding.EncodeOne(std::forward<TInput>(input), intermediateOutput, errorHandler, state);
      using TReconstructedResult =
        EncodeResult<decltype(intermediateResult.Input), Krys::Ranges::range_reconstruct_t<TOutput>, TState>;
      if (intermediateResult.ErrorCode != EncodingError::OK)
      {
        auto intermediateLast = intermediateResult.Output.data();
        auto outIt = Krys::Ranges::begin(std::move(output));
        auto outLast = Krys::Ranges::end(std::move(output));
        for (auto intermediateIt = intermediateOutput.data(); intermediateIt != intermediateLast;)
        {
          *outIt = *intermediateIt;
          ++intermediateIt;
          ++outIt;
        }
        return TReconstructedResult(std::move(intermediateResult.Input),
                                    Krys::Ranges::reconstruct(std::in_place_type<TUOutput>,
                                                              std::forward<TOutput>(output), std::move(outIt),
                                                              std::move(outLast)),
                                    state, intermediateResult.ErrorCode, intermediateResult.ErrorCount);
      }
      return TReconstructedResult(std::move(intermediateResult.Input), std::forward<TOutput>(output), state,
                                  intermediateResult.ErrorCode, intermediateResult.ErrorCount);
    }
    else
    {
      return encoding.EncodeOne(std::forward<TInput>(input), std::forward<TOutput>(output), errorHandler,
                                state);
    }
  }

  template <Consume ConsumeIntoTheNothingness, Transaction TEncodeOrDecode, typename TEncoding,
            typename TInput, typename TOutputContainer, typename TErrorHandler, typename TState>
  constexpr auto BasicEncodeOrDecodeOne(TInput &&input, TEncoding &&encoding, TOutputContainer &output,
                                        TErrorHandler &errorHandler, TState &state)
  {
    if constexpr (TEncodeOrDecode == Transaction::Decode)
    {
      return BasicDecodeOne<ConsumeIntoTheNothingness>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding), output, errorHandler, state);
    }
    else
    {
      return BasicEncodeOne<ConsumeIntoTheNothingness>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding), output, errorHandler, state);
    }
  }
}
