#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Text/Concepts.hpp"
#include "Krystal.Text/Handlers/DefaultHandler.hpp"
#include "Krystal.Text/Handlers/ForwardingHandler.hpp"
#include "Krystal.Text/Handlers/SchemeHandler.hpp"

namespace Krys::Text::detail
{
  template <typename TErrorHandler>
  class IsCarelessErrorHandler
      : public IntegralConstant<
          bool, SameType<remove_cvref_t<TErrorHandler>, ::Krys::Text::Handlers::DefaultHandler>>
  {
  };

  template <typename TByte, typename TUInputRange, typename TUOutputRange, typename TErrorHandler>
  class IsCarelessErrorHandler<
    ::Krys::Text::Handlers::SchemeHandler<TByte, TUInputRange, TUOutputRange, TErrorHandler>>
      : public IntegralConstant<bool, IsCarelessErrorHandler<TErrorHandler>::value>
  {
  };

  template <typename TEncoding, typename TErrorHandler>
  class IsCarelessErrorHandler<::Krys::Text::Handlers::ForwardingHandler<TEncoding, TErrorHandler>>
      : public IntegralConstant<bool, IsCarelessErrorHandler<TErrorHandler>::value>
  {
  };

  template <typename TErrorHandler>
  concept CarelessErrorHandler = IsCarelessErrorHandler<TErrorHandler>::value;

  template <typename TEncoding, typename TErrorHandler>
  class IsEncodeLosslessOrDeliberate
      : public IntegralConstant<bool, CarelessErrorHandler<remove_cvref_t<TErrorHandler>>
                                        ? IsEncodeInjective<remove_cvref_t<TEncoding>>
                                        : true>
  {
  };

  template <typename TEncoding, typename TErrorHandler>
  class IsDecodeLosslessOrDeliberate
      : public IntegralConstant<bool, CarelessErrorHandler<remove_cvref_t<TErrorHandler>>
                                        ? IsDecodeInjective<remove_cvref_t<TEncoding>>
                                        : true>
  {
  };

  template <typename TFromEncoding, typename TToEncoding, typename TFromErrorHandler,
            typename TToErrorHandler>
  class IsTranscodeLosslessOrDeliberate
      : public IntegralConstant<bool, (CarelessErrorHandler<remove_cvref_t<TFromErrorHandler>>
                                         ? IsDecodeInjective<remove_cvref_t<TFromEncoding>>
                                         : true)
                                        && (CarelessErrorHandler<remove_cvref_t<TToErrorHandler>>
                                              ? IsEncodeInjective<remove_cvref_t<TToEncoding>>
                                              : true)>
  {
  };
}
namespace Krys::Text
{
  template <typename TEncoding, typename TErrorHandler>
  concept EncodeLosslessOrDeliberate =
    ::Krys::Text::detail::IsEncodeLosslessOrDeliberate<TEncoding, TErrorHandler>::value;

  template <typename TEncoding, typename TErrorHandler>
  concept DecodeLosslessOrDeliberate =
    ::Krys::Text::detail::IsDecodeLosslessOrDeliberate<remove_cvref_t<TEncoding>,
                                                       remove_cvref_t<TErrorHandler>>::value;

  template <typename TFromEncoding, typename TToEncoding, typename TFromErrorHandler,
            typename TToErrorHandler>
  concept TranscodeLosslessOrDeliberate =
    ::Krys::Text::detail::IsTranscodeLosslessOrDeliberate<TFromEncoding, TToEncoding, TFromErrorHandler,
                                                          TToErrorHandler>::value;

#define KRYS_TEXT_LOSSY_DECODE_MESSAGE                                                                       \
  "This decode is a lossy, non-injective operation. This means you may lose data that you did not intend "   \
  "to lose; specify a 'handler' error handler parameter to Decode(input, encoding, handler, ...) or "        \
  "DecodeIntoRaw(input, encoding, output, handler, ...) explicitly in order to bypass this."

#define KRYS_TEXT_LOSSY_ENCODE_MESSAGE                                                                       \
  "This encode is a lossy, non-injective operation. This means you may lose data that you did not intend "   \
  "to lose; specify a 'handler' error handler parameter to Encode(input, encoding, handler, ...) or "        \
  "EncodeIntoRaw(input, encoding, output, handler, ...) explicitly in order to bypass this."

#define KRYS_TEXT_LOSSY_TRANSCODE_DECODE_MESSAGE                                                             \
  "The decode (input) portion of this transcode is a lossy, non-injective operation. This means you may "    \
  "lose data that you did not intend to lose; specify an 'in_handler' error handler parameter to "           \
  "Transcode(input, in_encoding, out_encoding, in_handler, ...) or TranscodeIntoRaw(input, in_encoding, "    \
  "output, out_encoding, in_handler, ...) explicitly in order to bypass this."
}
