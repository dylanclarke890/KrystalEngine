#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Text/ErrorHandlers/AssumeValidHandler.hpp"
#include "Krystal.Text/ErrorHandlers/DefaultHandler.hpp"
#include "Krystal.Text/ErrorHandlers/IncompleteHandler.hpp"
#include "Krystal.Text/ErrorHandlers/PassHandler.hpp"
#include "Krystal.Text/ErrorHandlers/ReplacementHandler.hpp"
#include "Krystal.Text/ErrorHandlers/ThrowHandler.hpp"
#include "Krystal.Text/IsCodePointsReplaceable.hpp"
#include "Krystal.Text/IsCodeUnitsReplaceable.hpp"
#include "Krystal.Text/IsUnicodeCodePoint.hpp"

namespace Krys
{
  namespace Impl
  {
    template <typename T>
    concept HasDecodeAlwaysOK = requires { T::DecodeAlwaysOK; };

    template <typename T>
    concept HasEncodeAlwaysOK = requires { T::EncodeAlwaysOK; };

    template <typename TEncoding, typename TErrorHandler>
    struct DecodeErrorHandlerAlwaysReturnsOK : public IntegralConstant<bool, HasDecodeAlwaysOK<TErrorHandler>>
    {
    };

    template <typename TEncoding>
    struct DecodeErrorHandlerAlwaysReturnsOK<TEncoding, Handlers::ReplacementHandler>
        : public IntegralConstant<bool, (HasReplacementCodePoints<TEncoding>
                                         && !HasMaybeReplacementCodePoints<TEncoding>)
                                          || IsUnicodeCodePoint<code_point_t<TEncoding>>>
    {
    };

    template <typename TEncoding>
    struct DecodeErrorHandlerAlwaysReturnsOK<TEncoding, Handlers::ThrowHandler> : public std::true_type
    {
    };

    template <typename TEncoding, typename TErrorHandler>
    struct DecodeErrorHandlerAlwaysReturnsOK<TEncoding,
                                             Handlers::BasicIncompleteHandler<TEncoding, TErrorHandler>>
        : public DecodeErrorHandlerAlwaysReturnsOK<
            TEncoding, typename Handlers::BasicIncompleteHandler<TEncoding, TErrorHandler>::ErrorHandler>
    {
    };

    template <typename TEncoding>
    struct DecodeErrorHandlerAlwaysReturnsOK<TEncoding, Handlers::AssumeValidHandler>
        : public std::true_type
    {
    };

    template <typename TEncoding>
    struct DecodeErrorHandlerAlwaysReturnsOK<TEncoding, Handlers::DefaultHandler>
        : public DecodeErrorHandlerAlwaysReturnsOK<TEncoding, Handlers::DefaultHandler::ErrorHandler>
    {
    };

    template <typename TEncoding>
    struct DecodeErrorHandlerAlwaysReturnsOK<TEncoding, Handlers::UncheckedDefaultHandler>
        : public DecodeErrorHandlerAlwaysReturnsOK<TEncoding, Handlers::DefaultHandler::ErrorHandler>
    {
    };

    template <typename TEncoding, typename TErrorHandler>
    struct EncodeErrorHandlerAlwaysReturnsOK : public IntegralConstant<bool, HasEncodeAlwaysOK<TErrorHandler>>
    {
    };

    template <typename TEncoding>
    struct EncodeErrorHandlerAlwaysReturnsOK<TEncoding, Handlers::ReplacementHandler>
        : public IntegralConstant<bool, (HasReplacementCodeUnits<TEncoding>
                                         && !HasMaybeReplacementCodeUnits<TEncoding>)
                                          || IsUnicodeCodePoint<code_unit_t<TEncoding>>>
    {
    };

    template <typename TEncoding>
    struct EncodeErrorHandlerAlwaysReturnsOK<TEncoding, Handlers::ThrowHandler> : public std::true_type
    {
    };

    template <typename TEncoding, typename TErrorHandler>
    struct EncodeErrorHandlerAlwaysReturnsOK<TEncoding,
                                             Handlers::BasicIncompleteHandler<TEncoding, TErrorHandler>>
        : public EncodeErrorHandlerAlwaysReturnsOK<
            TEncoding, typename Handlers::BasicIncompleteHandler<TEncoding, TErrorHandler>::ErrorHandler>
    {
    };

    template <typename TEncoding>
    struct EncodeErrorHandlerAlwaysReturnsOK<TEncoding, Handlers::AssumeValidHandler> : std::true_type
    {
    };

    template <typename TEncoding>
    struct EncodeErrorHandlerAlwaysReturnsOK<TEncoding, Handlers::DefaultHandler>
        : public EncodeErrorHandlerAlwaysReturnsOK<TEncoding, Handlers::DefaultHandler::ErrorHandler>
    {
    };

    template <typename TEncoding>
    struct EncodeErrorHandlerAlwaysReturnsOK<TEncoding, Handlers::UncheckedDefaultHandler>
        : public EncodeErrorHandlerAlwaysReturnsOK<TEncoding, Handlers::DefaultHandler::ErrorHandler>
    {
    };
  }

  /// @brief Whether or not the given `TEncoding` and `TInput` with the provided `TErrorHandler` will always
  /// return EncodingError::OK for any failure that is not related to an output being too small
  /// (EncodingError::InsufficientOutputSpace).
  /// @tparam TEncoding The encoding type whose `DecodeOne` function will be used with the error handler.
  /// @tparam TInput The input range that will be used with the `DecodeOne` function of the encoding.
  /// @tparam TErrorHandler The error handler that will be called with the given `TEncoding` object and
  /// `TInput` range.
  /// @remarks This is a compile time assertion. If the encoding may exhibit different behavior at runtime
  /// based on runtime conditions, then this should return false. This is meant for cases where it is
  /// provable at compile time, this should return true. For example, if the Handlers::ReplacementHandler is
  /// used in conjunction with utf8, then this will return true.
  template <typename TEncoding, typename TErrorHandler>
  concept DecodeErrorHandlerAlwaysReturnsOK =
    Impl::DecodeErrorHandlerAlwaysReturnsOK<TEncoding, TErrorHandler>::value;

  /// @brief Whether or not the given `TEncoding` and `TInput` with the provided `TErrorHandler` will always
  /// return EncodingError::OK for any failure that is not related to an output being too small
  /// (EncodingError::InsufficientOutputSpace).
  /// @tparam TEncoding The encoding type whose `DecodeOne` function will be used with the error handler.
  /// @tparam TInput The input range that will be used with the `DecodeOne` function of the encoding.
  /// @tparam TErrorHandler The error handler that will be called with the given `TEncoding` object and
  /// `TInput` range.
  /// @remarks This is a compile time assertion. If the encoding may exhibit different behavior at runtime
  /// based on runtime conditions, then this should return false. This is meant for cases where it is
  /// provable at compile time, this should return true. For example, if the Handlers::ReplacementHandler is
  /// used in conjunction with utf8, then this will return true.
  template <typename TEncoding, typename TErrorHandler>
  concept EncodeErrorHandlerAlwaysReturnsOK =
    Impl::EncodeErrorHandlerAlwaysReturnsOK<TEncoding, TErrorHandler>::value;
}
