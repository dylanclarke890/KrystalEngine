#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/ADL.hpp"
#include "Krystal.Lib/Ranges/Iterator.hpp"
#include "Krystal.Lib/Utils/Tag.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/State.hpp"
#include <utility>

namespace Krys::Impl
{
#pragma region Decode Validation
  template <typename TInput, typename TEncoding, typename TState, typename... TAdditionalStates>
  concept HasInternalTextValidateDecodableAs = requires {
    InternalTextValidateDecodableAs(Krys::Tag<remove_cvref_t<TEncoding>> {}, std::declval<TInput>(),
                                    std::declval<TEncoding>(), std::declval<TState &>(),
                                    std::declval<TAdditionalStates &>()...);
  };

  template <typename TInput, typename TEncoding, typename TState, typename... TAdditionalStates>
  concept HasInternalTextValidateDecodableAsOne = requires {
    InternalTextValidateDecodableAsOne(Krys::Tag<remove_cvref_t<TEncoding>> {}, std::declval<TInput>(),
                                       std::declval<TEncoding>(), std::declval<TState &>(),
                                       std::declval<TAdditionalStates &>()...);
  };

  template <typename TInput, typename TEncoding, typename TState, typename... TAdditionalStates>
  concept HasTextValidateDecodableAs = requires {
    TextValidateDecodableAs(Krys::Tag<remove_cvref_t<TEncoding>> {}, std::declval<TInput>(),
                            std::declval<TEncoding>(), std::declval<TState &>(),
                            std::declval<TAdditionalStates &>()...);
  };

  template <typename TInput, typename TEncoding, typename TState, typename... TAdditionalStates>
  concept HasTextValidateDecodableAsOne = requires {
    TextValidateDecodableAsOne(Krys::Tag<remove_cvref_t<TEncoding>> {}, std::declval<TInput>(),
                               std::declval<TEncoding>(), std::declval<TState &>(),
                               std::declval<TAdditionalStates &>()...);
  };
#pragma endregion

#pragma region Encode Validation
  template <typename TInput, typename TEncoding, typename TState, typename... TAdditionalStates>
  concept HasInternalTextValidateEncodableAs = requires {
    InternalTextValidateEncodableAs(Krys::Tag<remove_cvref_t<TEncoding>> {}, std::declval<TInput>(),
                                    std::declval<TEncoding>(), std::declval<TState &>(),
                                    std::declval<TAdditionalStates &>()...);
  };

  template <typename TInput, typename TEncoding, typename TState, typename... TAdditionalStates>
  concept HasInternalTextValidateEncodableAsOne = requires {
    InternalTextValidateEncodableAsOne(Krys::Tag<remove_cvref_t<TEncoding>> {}, std::declval<TInput>(),
                                       std::declval<TEncoding>(), std::declval<TState &>(),
                                       std::declval<TAdditionalStates &>()...);
  };

  template <typename TInput, typename TEncoding, typename TState, typename... TAdditionalStates>
  concept HasTextValidateEncodableAs = requires {
    TextValidateEncodableAs(Krys::Tag<remove_cvref_t<TEncoding>> {}, std::declval<TInput>(),
                            std::declval<TEncoding>(), std::declval<TState &>(),
                            std::declval<TAdditionalStates &>()...);
  };

  template <typename TInput, typename TEncoding, typename TState, typename... TAdditionalStates>
  concept HasTextValidateEncodableAsOne = requires {
    TextValidateEncodableAsOne(Krys::Tag<remove_cvref_t<TEncoding>> {}, std::declval<TInput>(),
                               std::declval<TEncoding>(), std::declval<TState &>(),
                               std::declval<TAdditionalStates &>()...);
  };
#pragma endregion

#pragma region Transcode Validation
  template <typename TInput, typename TFromEncoding, typename TToEncoding, typename TFromState,
            typename TToState, typename TPivot>
  concept HasInternalTextValidateTranscodableAs = requires {
    InternalTextValidateTranscodableAs(
      Krys::Tag<remove_cvref_t<TFromEncoding>, remove_cvref_t<TToEncoding>> {}, std::declval<TInput>(),
      std::declval<TFromEncoding>(), std::declval<TToEncoding>(), std::declval<TFromState &>(),
      std::declval<TToState &>(), std::declval<TPivot>());
  };

  template <typename TInput, typename TFromEncoding, typename TToEncoding, typename TFromState,
            typename TToState, typename TPivot>
  concept HasInternalTextValidateTranscodableAsOne = requires {
    InternalTextValidateTranscodableAsOne(
      Krys::Tag<remove_cvref_t<TFromEncoding>, remove_cvref_t<TToEncoding>> {}, std::declval<TInput>(),
      std::declval<TFromEncoding>(), std::declval<TToEncoding>(), std::declval<TFromState &>(),
      std::declval<TToState &>(), std::declval<TPivot>());
  };

  template <typename TInput, typename TFromEncoding, typename TToEncoding, typename TFromState,
            typename TToState, typename TPivot>
  concept HasTextValidateTranscodableAs = requires {
    TextValidateTranscodableAs(Krys::Tag<remove_cvref_t<TFromEncoding>, remove_cvref_t<TToEncoding>> {},
                               std::declval<TInput>(), std::declval<TFromEncoding>(),
                               std::declval<TToEncoding>(), std::declval<TFromState &>(),
                               std::declval<TToState &>(), std::declval<TPivot>());
  };

  template <typename TInput, typename TFromEncoding, typename TToEncoding, typename TFromState,
            typename TToState, typename TPivot>
  concept HasTextValidateTranscodableAsOne = requires {
    TextValidateTranscodableAsOne(Krys::Tag<remove_cvref_t<TFromEncoding>, remove_cvref_t<TToEncoding>> {},
                                  std::declval<TInput>(), std::declval<TFromEncoding>(),
                                  std::declval<TToEncoding>(), std::declval<TFromState &>(),
                                  std::declval<TToState &>(), std::declval<TPivot>());
  };
#pragma endregion

#pragma region Counting Code Units
  template <typename TInput, typename TEncoding, typename THandler, typename TState>
  concept HasInternalTextCountAsDecoded = requires {
    InternalTextCountAsDecoded(Krys::Tag<remove_cvref_t<TEncoding>> {}, std::declval<TInput>(),
                               std::declval<TEncoding>(), std::declval<THandler>(), std::declval<TState &>());
  };

  template <typename TInput, typename TEncoding, typename THandler, typename TState>
  concept HasInternalTextCountAsDecodedOne = requires {
    InternalTextCountAsDecodedOne(Krys::Tag<remove_cvref_t<TEncoding>> {}, std::declval<TInput>(),
                                  std::declval<TEncoding>(), std::declval<THandler>(),
                                  std::declval<TState &>());
  };

  template <typename TInput, typename TEncoding, typename THandler, typename TState>
  concept HasTextCountAsDecoded = requires {
    TextCountAsDecoded(Krys::Tag<remove_cvref_t<TEncoding>> {}, std::declval<TInput>(),
                       std::declval<TEncoding>(), std::declval<THandler>(), std::declval<TState &>());
  };

  template <typename TInput, typename TEncoding, typename THandler, typename TState>
  concept HasTextCountAsDecodedOne = requires {
    TextCountAsDecodedOne(Krys::Tag<remove_cvref_t<TEncoding>> {}, std::declval<TInput>(),
                          std::declval<TEncoding>(), std::declval<THandler>(), std::declval<TState &>());
  };
#pragma endregion

#pragma region Counting Code Points
  template <typename TInput, typename TEncoding, typename THandler, typename TState>
  concept HasInternalTextCountAsEncoded = requires {
    InternalTextCountAsEncoded(std::declval<TInput>(), std::declval<TEncoding>(), std::declval<THandler>(),
                               std::declval<TState &>());
  };

  template <typename TInput, typename TEncoding, typename THandler, typename TState>
  concept HasInternalTextCountAsEncodedOne = requires {
    InternalTextCountAsEncodedOne(Krys::Tag<remove_cvref_t<TEncoding>> {}, std::declval<TEncoding>(),
                                  std::declval<TInput>(), std::declval<THandler>(), std::declval<TState &>());
  };

  template <typename TInput, typename TEncoding, typename THandler, typename TState>
  concept HasTextCountAsEncoded = requires {
    TextCountAsEncoded(Krys::Tag<remove_cvref_t<TEncoding>> {}, std::declval<TInput>(),
                       std::declval<TEncoding>(), std::declval<THandler>(), std::declval<TState &>());
  };

  template <typename TInput, typename TEncoding, typename THandler, typename TState>
  concept HasTextCountAsEncodedOne = requires {
    TextCountAsEncodedOne(Krys::Tag<remove_cvref_t<TEncoding>> {}, std::declval<TEncoding>(),
                          std::declval<TInput>(), std::declval<THandler>(), std::declval<TState &>());
  };
#pragma endregion

#pragma region Counting Transcode Code Units
  template <typename TInput, typename TFromEncoding, typename TToEncoding, typename TFromHandler,
            typename TToHandler, typename TFromState, typename TToState, typename TPivot>
  concept HasInternalTextCountAsTranscoded = requires {
    InternalTextCountAsTranscoded(Krys::Tag<remove_cvref_t<TFromEncoding>, remove_cvref_t<TToEncoding>> {},
                                  std::declval<TInput>(), std::declval<TFromEncoding>(),
                                  std::declval<TToEncoding>(), std::declval<TFromHandler>(),
                                  std::declval<TToHandler>(), std::declval<TFromState &>(),
                                  std::declval<TToState &>(), std::declval<TPivot>());
  };
  template <typename TInput, typename TFromEncoding, typename TToEncoding, typename TFromHandler,
            typename TToHandler, typename TFromState, typename TToState, typename TPivot>
  concept HasInternalTextCountAsTranscodedOne = requires {
    InternalTextCountAsTranscodedOne(Krys::Tag<remove_cvref_t<TFromEncoding>, remove_cvref_t<TToEncoding>> {},
                                     std::declval<TFromEncoding>(), std::declval<TToEncoding>(),
                                     std::declval<TInput>(), std::declval<TFromHandler>(),
                                     std::declval<TToHandler>(), std::declval<TFromState &>(),
                                     std::declval<TToState &>(), std::declval<TPivot>());
  };

  template <typename TInput, typename TFromEncoding, typename TToEncoding, typename TFromHandler,
            typename TToHandler, typename TFromState, typename TToState, typename TPivot>
  concept HasTextCountAsTranscoded = requires {
    TextCountAsTranscoded(Krys::Tag<remove_cvref_t<TFromEncoding>, remove_cvref_t<TToEncoding>> {},
                          std::declval<TInput>(), std::declval<TFromEncoding>(), std::declval<TToEncoding>(),
                          std::declval<TFromHandler>(), std::declval<TToHandler>(),
                          std::declval<TFromState &>(), std::declval<TToState &>(), std::declval<TPivot>());
  };

  template <typename TInput, typename TFromEncoding, typename TToEncoding, typename TFromHandler,
            typename TToHandler, typename TFromState, typename TToState, typename TPivot>
  concept HasTextCountAsTranscodedOne = requires {
    TextCountAsTranscodedOne(Krys::Tag<remove_cvref_t<TFromEncoding>, remove_cvref_t<TToEncoding>> {},
                             std::declval<TInput>(), std::declval<TFromEncoding>(),
                             std::declval<TToEncoding>(), std::declval<TFromHandler>(),
                             std::declval<TToHandler>(), std::declval<TFromState &>(),
                             std::declval<TToState &>(), std::declval<TPivot>());
  };
#pragma endregion

#pragma region Decode
  template <typename TEncoding, typename TInput, typename TOutput, typename THandler, typename TState>
  concept HasDecodeOneMember = requires {
    std::declval<TEncoding>().DecodeOne(std::declval<TInput>(), std::declval<TOutput>(),
                                        std::declval<THandler>(), std::declval<TState &>());
  };

  template <typename TEncoding, typename TInput, typename TOutput, typename THandler, typename TState>
  concept HasDecodeOneBackwardsMember = requires {
    std::declval<TEncoding>().DecodeOneBackwards(std::declval<TInput>(), std::declval<TOutput>(),
                                                 std::declval<THandler>(), std::declval<TState &>());
  };

  template <typename TInput, typename TEncoding, typename TOutput, typename THandler, typename TState>
  concept HasTextDecode = requires {
    TextDecode(Krys::Tag<remove_cvref_t<TEncoding>> {}, std::declval<TInput>(), std::declval<TEncoding>(),
               std::declval<TOutput>(), std::declval<THandler>(), std::declval<TState &>());
  };

  template <typename TInput, typename TEncoding, typename TOutput, typename THandler, typename TState>
  concept HasInternalTextDecode = requires {
    InternalTextDecode(Krys::Tag<remove_cvref_t<TEncoding>> {}, std::declval<TInput>(),
                       std::declval<TEncoding>(), std::declval<TOutput>(), std::declval<THandler>(),
                       std::declval<TState &>());
  };
#pragma endregion

#pragma region Encode
  template <typename TEncoding, typename TInput, typename TOutput, typename THandler, typename TState>
  concept HasEncodeOneMember = requires {
    std::declval<TEncoding>().EncodeOne(std::declval<TInput>(), std::declval<TOutput>(),
                                        std::declval<THandler>(), std::declval<TState &>());
  };

  template <typename TEncoding, typename TInput, typename TOutput, typename THandler, typename TState>
  concept HasEncodeOneBackwardsMember = requires {
    std::declval<TEncoding>().EncodeOneBackwards(std::declval<TInput>(), std::declval<TOutput>(),
                                                 std::declval<THandler>(), std::declval<TState &>());
  };

  template <typename TInput, typename TEncoding, typename TOutput, typename THandler, typename TState>
  concept HasTextEncode = requires {
    TextEncode(Krys::Tag<remove_cvref_t<TEncoding>> {}, std::declval<TInput>(), std::declval<TEncoding>(),
               std::declval<TOutput>(), std::declval<THandler>(), std::declval<TState &>());
  };

  template <typename TInput, typename TEncoding, typename TOutput, typename THandler, typename TState>
  concept HasInternalTextEncode = requires {
    InternalTextEncode(Krys::Tag<remove_cvref_t<TEncoding>> {}, std::declval<TInput>(),
                       std::declval<TEncoding>(), std::declval<TOutput>(), std::declval<THandler>(),
                       std::declval<TState &>());
  };
#pragma endregion

#pragma region Transcode
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromHandler, typename TToHandler, typename TFromState, typename TToState,
            typename TPivot>
  concept HasTextTranscode = requires {
    TextTranscode(Krys::Tag<remove_cvref_t<TFromEncoding>, remove_cvref_t<TToEncoding>>(),
                  std::declval<TInput>(), std::declval<TFromEncoding>(), std::declval<TOutput>(),
                  std::declval<TToEncoding>(), std::declval<TFromHandler>(), std::declval<TToHandler>(),
                  std::declval<TFromState &>(), std::declval<TToState &>(), std::declval<TPivot>());
  };

  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromHandler, typename TToHandler, typename TFromState, typename TToState,
            typename TPivot>
  concept HasInternalTextTranscode = requires {
    InternalTextTranscode(Krys::Tag<remove_cvref_t<TFromEncoding>, remove_cvref_t<TToEncoding>> {},
                          std::declval<TInput>(), std::declval<TFromEncoding>(), std::declval<TOutput>(),
                          std::declval<TToEncoding>(), std::declval<TFromHandler>(),
                          std::declval<TToHandler>(), std::declval<TFromState &>(),
                          std::declval<TToState &>(), std::declval<TPivot>());
  };

  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromHandler, typename TToHandler, typename TFromState, typename TToState,
            typename TPivot>
  concept HasInternalTextTranscodeOne = requires {
    InternalTextTranscodeOne(Krys::Tag<remove_cvref_t<TFromEncoding>, remove_cvref_t<TToEncoding>> {},
                             std::declval<TInput>(), std::declval<TFromEncoding>(), std::declval<TOutput>(),
                             std::declval<TToEncoding>(), std::declval<TFromHandler>(),
                             std::declval<TToHandler>(), std::declval<TFromState &>(),
                             std::declval<TToState &>(), std::declval<TPivot>());
  };

  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromHandler, typename TToHandler, typename TFromState, typename TToState,
            typename TPivot>
  concept HasTextTranscodeOne = requires {
    TextTranscodeOne(Krys::Tag<remove_cvref_t<TFromEncoding>, remove_cvref_t<TToEncoding>> {},
                     std::declval<TInput>(), std::declval<TFromEncoding>(), std::declval<TOutput>(),
                     std::declval<TToEncoding>(), std::declval<TFromHandler>(), std::declval<TToHandler>(),
                     std::declval<TFromState &>(), std::declval<TToState &>(), std::declval<TPivot>());
  };
#pragma endregion

#pragma region Recode
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromHandler, typename TToHandler, typename TFromState, typename TToState,
            typename TPivot>
  concept HasTextRecode = requires {
    TextRecode(Krys::Tag<remove_cvref_t<TFromEncoding>, remove_cvref_t<TToEncoding>>(),
               std::declval<TInput>(), std::declval<TFromEncoding>(), std::declval<TOutput>(),
               std::declval<TToEncoding>(), std::declval<TFromHandler>(), std::declval<TToHandler>(),
               std::declval<TFromState &>(), std::declval<TToState &>(), std::declval<TPivot>());
  };

  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromHandler, typename TToHandler, typename TFromState, typename TToState,
            typename TPivot>
  concept HasInternalTextRecode = requires {
    InternalTextRecode(Krys::Tag<remove_cvref_t<TFromEncoding>, remove_cvref_t<TToEncoding>> {},
                       std::declval<TInput>(), std::declval<TFromEncoding>(), std::declval<TOutput>(),
                       std::declval<TToEncoding>(), std::declval<TFromHandler>(), std::declval<TToHandler>(),
                       std::declval<TFromState &>(), std::declval<TToState &>(), std::declval<TPivot>());
  };

  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromHandler, typename TToHandler, typename TFromState, typename TToState,
            typename TPivot>
  concept HasInternalTextRecodeOne = requires {
    InternalTextRecodeOne(Krys::Tag<remove_cvref_t<TFromEncoding>, remove_cvref_t<TToEncoding>> {},
                          std::declval<TInput>(), std::declval<TFromEncoding>(), std::declval<TOutput>(),
                          std::declval<TToEncoding>(), std::declval<TFromHandler>(),
                          std::declval<TToHandler>(), std::declval<TFromState &>(),
                          std::declval<TToState &>(), std::declval<TPivot>());
  };

  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromHandler, typename TToHandler, typename TFromState, typename TToState,
            typename TPivot>
  concept HasTextRecodeOne = requires {
    TextRecodeOne(Krys::Tag<remove_cvref_t<TFromEncoding>, remove_cvref_t<TToEncoding>> {},
                  std::declval<TInput>(), std::declval<TFromEncoding>(), std::declval<TOutput>(),
                  std::declval<TToEncoding>(), std::declval<TFromHandler>(), std::declval<TToHandler>(),
                  std::declval<TFromState &>(), std::declval<TToState &>(), std::declval<TPivot>());
  };
#pragma endregion

  template <typename THandler, typename TEncoding, typename TResult, typename _Progress>
  concept HasCallableHandler = requires {
    std::declval<THandler>()(std::declval<const TEncoding &>(), std::declval<TResult>(),
                             std::declval<_Progress>());
  };

  template <typename TEncoding, typename = void>
  struct RangeCategory
  {
    using type = void;
  };

  template <typename TEncoding>
  struct RangeCategory<TEncoding, std::void_t<typename remove_cvref_t<TEncoding>::range_category>>
  {
    using type = typename remove_cvref_t<TEncoding>::range_category;
  };

  template <typename TEncoding, typename = void>
  struct DecodeRangeCategory : public RangeCategory<TEncoding>
  {
  };

  template <typename TEncoding>
  struct DecodeRangeCategory<TEncoding,
                             std::void_t<typename remove_cvref_t<TEncoding>::decode_range_category>>
  {
    using type = typename remove_cvref_t<TEncoding>::decode_range_category;
  };

  template <typename TEncoding, typename = void>
  struct EncodeRangeCategory : public RangeCategory<TEncoding>
  {
  };

  template <typename TEncoding>
  struct EncodeRangeCategory<TEncoding,
                             std::void_t<typename remove_cvref_t<TEncoding>::encode_range_category>>
  {
    using type = typename remove_cvref_t<TEncoding>::encode_range_category;
  };

  template <typename TEncoding>
  using decode_range_category_t = typename DecodeRangeCategory<TEncoding>::type;

  template <typename TEncoding>
  using encode_range_category_t = typename EncodeRangeCategory<TEncoding>::type;

  template <typename TEncoding>
  concept DecodeRangeCategoryOutput =
    DerivedFrom<decode_range_category_t<TEncoding>, std::output_iterator_tag>;

  template <typename TEncoding>
  concept EncodeRangeCategoryOutput =
    DerivedFrom<encode_range_category_t<TEncoding>, std::output_iterator_tag>;

  template <typename TEncoding>
  concept ContiguousEncodeRangeCategory =
    DerivedFrom<encode_range_category_t<TEncoding>, Krys::contiguous_iterator_tag>;

  template <typename TEncoding>
  concept ContiguousDecodeRangeCategory =
    DerivedFrom<decode_range_category_t<TEncoding>, Krys::contiguous_iterator_tag>;
}
