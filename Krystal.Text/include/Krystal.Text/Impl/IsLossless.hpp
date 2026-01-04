#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Text/Impl/ForwardingHandler.hpp"
#include "Krystal.Text/Impl/SchemeHandler.hpp"
#include "Krystal.Text/IsFullRangeRepresentable.hpp"

namespace Krys::Impl
{
  template <typename TErrorHandler>
  class IsCarelessErrorHandler
      : public IntegralConstant<bool, SameType<remove_cvref_t<TErrorHandler>, Handlers::DefaultHandler>>
  {
  };

  template <typename TByte, typename TUInputRange, typename TUOutputRange, typename TErrorHandler>
  class IsCarelessErrorHandler<SchemeHandler<TByte, TUInputRange, TUOutputRange, TErrorHandler>>
      : public IntegralConstant<bool, IsCarelessErrorHandler<TErrorHandler>::value>
  {
  };

  template <typename TEncoding, typename TErrorHandler>
  class IsCarelessErrorHandler<ForwardingHandler<TEncoding, TErrorHandler>>
      : public IntegralConstant<bool, IsCarelessErrorHandler<TErrorHandler>::value>
  {
  };

  template <typename TErrorHandler>
  concept CarelessErrorHandler = IsCarelessErrorHandler<TErrorHandler>::value;

  template <typename TEncoding, typename TErrorHandler>
  class IsEncodeLosslessOrDeliberate
      : public IntegralConstant<bool, Impl::CarelessErrorHandler<remove_cvref_t<TErrorHandler>>
                                        ? IsEncodeInjective<remove_cvref_t<TEncoding>>
                                        : true>
  {
  };

  template <typename TEncoding, typename TErrorHandler>
  concept EncodeLosslessOrDeliberate = IsEncodeLosslessOrDeliberate<TEncoding, TErrorHandler>::value;

  template <typename TEncoding, typename TErrorHandler>
  class IsDecodeLosslessOrDeliberate
      : public IntegralConstant<bool, Impl::CarelessErrorHandler<remove_cvref_t<TErrorHandler>>
                                        ? IsDecodeInjective<remove_cvref_t<TEncoding>>
                                        : true>
  {
  };

  template <typename TEncoding, typename TErrorHandler>
  concept DecodeLosslessOrDeliberate = IsDecodeLosslessOrDeliberate<TEncoding, TErrorHandler>::value;

  template <typename TFromEncoding, typename TToEncoding, typename TFromErrorHandler,
            typename TToErrorHandler>
  class IsTranscodeLosslessOrDeliberate
      : public IntegralConstant<bool, (Impl::CarelessErrorHandler<remove_cvref_t<TFromErrorHandler>>
                                         ? IsDecodeInjective<remove_cvref_t<TFromEncoding>>
                                         : true)
                                        && (Impl::CarelessErrorHandler<remove_cvref_t<TToErrorHandler>>
                                              ? IsEncodeInjective<remove_cvref_t<TToEncoding>>
                                              : true)>
  {
  };

  template <typename TFromEncoding, typename TToEncoding, typename TFromErrorHandler,
            typename TToErrorHandler>
  concept TranscodeLosslessOrDeliberate =
    IsTranscodeLosslessOrDeliberate<TFromEncoding, TToEncoding, TFromErrorHandler, TToErrorHandler>::value;

#define ZTD_TEXT_LOSSY_DECODE_MESSAGE_I_                                                                     \
  "This decode is a lossy, non-injective operation. This means you may lose data that you did not intend "   \
  "to lose; "                                                                                                \
  "specify a 'handler' error handler parameter to decode(in, encoding, handler, ...) or "                    \
  "decode_into_raw(in, "                                                                                     \
  "encoding, "                                                                                               \
  "out, handler, ...) explicitly in order to bypass this."

#define ZTD_TEXT_LOSSY_ENCODE_MESSAGE_I_                                                                     \
  "This encode is a lossy, non-injective operation. This means you may lose data that you did not intend "   \
  "to lose; "                                                                                                \
  "specify a 'handler' error handler parameter to encode(in, encoding, handler, ...) or "                    \
  "encode_into_raw(in, "                                                                                     \
  "encoding, "                                                                                               \
  "out, handler, ...) explicitly in order to bypass this."

#define ZTD_TEXT_LOSSY_TRANSCODE_DECODE_MESSAGE_I_                                                           \
  "The decode (input) portion of this transcode is a lossy, non-injective operation. This means you may "    \
  "lose data "                                                                                               \
  "that you did not intend to lose; specify an 'in_handler' error handler parameter to transcode[_to](in, "  \
  "in_encoding, out_encoding, in_handler, ...) or transcode_into_raw(in, in_encoding, out, out_encoding, "   \
  "in_handler, "                                                                                             \
  "...) explicitly in order to bypass this."

#define ZTD_TEXT_LOSSY_TRANSCODE_ENCODE_MESSAGE_I_                                                           \
  "The encode (output) portion of this transcode is a lossy, non-injective operation. This means you may "   \
  "lose "                                                                                                    \
  "data that you did not intend to lose; specify an 'out_handler' error handler parameter to "               \
  "transcode[_to](in, "                                                                                      \
  "in_encoding, out_encoding, in_handler, out_handler, ...) or transcode_into_raw(in, in_encoding, out, "    \
  "out_encoding, "

#define ZTD_TEXT_LOSSY_RECODE_DECODE_MESSAGE_I_                                                              \
  "The encode (input) portion of this recode is a lossy, non-injective operation. This means you may lose "  \
  "data "                                                                                                    \
  "that you did not intend to lose; specify an 'in_handler' error handler parameter to recode[_to](in, "     \
  "in_encoding, out_encoding, in_handler, ...) or recode_into_raw(in, in_encoding, out, out_encoding, "      \
  "in_handler, "                                                                                             \
  "...) explicitly in order to bypass this."

#define ZTD_TEXT_LOSSY_RECODE_ENCODE_MESSAGE_I_                                                              \
  "The decode (output) portion of this recode is a lossy, non-injective operation. This means you may lose " \
  "data that you did not intend to lose; specify an 'out_handler' error handler parameter to "               \
  "recode[_to](in, "                                                                                         \
  "in_encoding, out_encoding, in_handler, out_handler, ...) or recode_into_raw(in, in_encoding, out, "       \
  "out_encoding, "                                                                                           \
  "in_handler, out_handler, ...) explicitly in order to bypass this."

#define ZTD_TEXT_UNKNOWN_LITERAL_ENCODING_MESSAGE_I_(_LITERAL_NAME, _LITERAL_TYPE_NAME, _LITERAL_PREFIX)     \
  "[[ PLEASE. READ. ]] Your compiler does not implement any known way of getting the " _LITERAL_NAME         \
  " encoding from the machine at compile time and you are trying to use the " _LITERAL_TYPE_NAME             \
  " encoding type, somewhere. If you are part of the C++ Standards Committee, please make sure p1885 "       \
  "(https://wg21.link/p1885) gets passed so we don't need to keep doing this nonsense. MSVC (VS2022 "        \
  "v17.0+), "                                                                                                \
  "Clang (v12.0+), and GCC (v11.1+) all have features to detect this. If you are on some other "             \
  "compiler,\n\n"                                                                                            \
                                                                                                             \
  "God's Speed.\n\n"                                                                                         \
                                                                                                             \
  "This could mangle all of your compile time string literal_t conversions you are performing. To prevent "  \
  "that "                                                                                                    \
  "from happening, this error is being printed out, so that you have the explicit understanding that "       \
  "your life may indeed be terrible after going ahead with the below advice.\n\n"                            \
                                                                                                             \
  "If you know what the literal encoding is, pass it through your command line build files so ztd.text "     \
  "understands it with ZTD_COMPILE_TIME_" _LITERAL_PREFIX                                                    \
  "ENCODING_NAME=\"NAME-HERE\". If you do not care to figure it out and what this error to go away, then "   \
  "add "                                                                                                     \
  "ZTD_TEXT_YES_PLEASE_DESTROY_MY_" _LITERAL_PREFIX                                                          \
  "LITERALS_UTTERLY_I_MEAN_IT to your command line to ignore this error and we will get right on doing "     \
  "exactly "                                                                                                 \
  "that for you!"

#define ZTD_TEXT_UNIMPLEMENTED_LITERAL_ENCODING_MESSAGE_I_(_ENCODING_DESCRIPTION, _LITERAL_PREFIX)           \
  "[[ PLEASE. READ. ]] This text encoding (" _ENCODING_DESCRIPTION                                           \
  "), while recognized, is not supported because it hasn't yet been implemented or tuned! You can see all "  \
  "the "                                                                                                     \
  "encodings we have support for in our documentation "                                                      \
  "(https://ztdtext.rtfd.io/en/latest/encodings.html).\n\n"                                                  \
                                                                                                             \
  "If you need this to be implemented, please reach out at the repository or to the contact addresses in "   \
  "the "                                                                                                     \
  "repository. If you absolutely don't give a damn, specify please add "                                     \
  "ZTD_TEXT_YES_PLEASE_DESTROY_MY_" _LITERAL_PREFIX                                                          \
  "LITERALS_UTTERLY_I_MEAN_IT to your command line to ignore this error and we will get right on doing "     \
  "exactly "                                                                                                 \
  "that for you."

#define ZTD_TEXT_RVALUE_NON_RANGE_INPUT_MESSAGE_I_                                                           \
  "Passing an r-value, non-view range into the lower-level *_into functions (e.g. decode_into_raw, "         \
  "transcode_into_raw, "                                                                                     \
  "and more) can result in wholly undesirable behavior and excessive copying that can result in extremely "  \
  "poor "                                                                                                    \
  "performance. If you would like to pass in a container, please use the top-level non-suffixed or *_to "    \
  "functions "                                                                                               \
  "so that an appropriate output container can be made and natural view-like, mostly-cheap ranges can be "   \
  "copied "                                                                                                  \
  "into the *_into functions to do the bulk of the hard work."
}
