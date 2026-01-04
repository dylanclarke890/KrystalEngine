#pragma once

#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/Encodings/UTF8.hpp"
#include "Krystal.Text/Impl/DefaultCharView.hpp"
#include "Krystal.Text/RecodeIterator.hpp"
#include <string_view>

namespace Krys
{
  /// @brief A recoding iterator that takes an input of code units and provides an output over the code units
  /// of the desired `TToEncoding` after converting from the `TFromEncoding` in a fashion that will never
  /// produce a EncodingError::InsufficientOutput error.
  /// @tparam TFromEncoding The encoding to read the underlying range of code points as.
  /// @tparam TToEncoding The encoding to read the underlying range of code points as.
  /// @tparam TRange The range of input that will be fed into the TFromEncoding's decode operation.
  /// @tparam TFromErrorHandler The error handler for any decode-step failures.
  /// @tparam TToErrorHandler The error handler for any encode-step failures.
  /// @tparam TFromState The state type to use for the decode operations to intermediate code points.
  /// @tparam TToState The state type to use for the encode operations to intermediate code points.
  /// @remarks This type produces proxies as their reference type, and are only readable, not writable
  /// iterators. The type will also try many different shortcuts for decoding the input and encoding the
  /// intermediates, respectively, including invoking a few customization points for either `DecodeOne.` or @c
  /// EncodeOne . It may also call `RecodeOne` to bypass having to do the round-trip through
  /// two encodings, which an encoding pair that a developer is interested in can use to do the conversion
  /// more quickly. The view presents code units one at a time, regardless of how many code units are output
  /// by one decode operation. This means if, for example, one (1) UTF-16 code unit becomes two (2) UTF-8 code
  /// units, it will present each code unit one at a time. If you are looking to explicitly know each
  /// collection of characters, you will have to use lower-level interfaces.
  template <typename TFromEncoding, typename TToEncoding = utf8_t,
            typename TRange = Impl::default_char_view_t<code_unit_t<TFromEncoding>>,
            typename TFromErrorHandler = Handlers::DefaultHandler,
            typename TToErrorHandler = Handlers::DefaultHandler,
            typename TFromState = decode_state_t<TFromEncoding>,
            typename TToState = encode_state_t<TToEncoding>>
  class RecodeView : public Krys::Ranges::ViewBase
  {
  public:
    /// @brief The iterator type for this view.
    using iterator = RecodeIterator<TFromEncoding, TToEncoding, TRange, TFromErrorHandler, TToErrorHandler,
                                    TFromState, TToState>;

    /// @brief The sentinel type for this view.
    using sentinel = TRecodeSentinel;

    /// @brief The underlying range type.
    using range_type = TRange;

    /// @brief The encoding type used for decoding to intermediate code point storage.
    using from_encoding_type = TFromEncoding;

    /// @brief The encoding type used for encoding to the final code units storage.
    using to_encoding_type = TToEncoding;

    /// @brief The error handler when a decode operation fails.
    using from_error_handler_type = TFromErrorHandler;

    /// @brief The error handler when an encode operation fails.
    using to_error_handler_type = TToErrorHandler;

    /// @brief The state type used for decode operations.
    using from_state_type = TFromState;

    /// @brief The state type used for encode operations.
    using to_state_type = TToState;

    /// @brief Constructs a RecodeView from the underlying range.
    ///
    /// @param[in] range The input range to wrap and iterate over.
    constexpr RecodeView(range_type range) noexcept : RecodeView(std::move(range), to_encoding_type {})
    {
    }

    /// @brief Constructs a RecodeView from the underlying range.
    ///
    /// @param[in] range The input range to wrap and iterate over.
    /// @param[in] toEncoding The encoding object to call `EncodeOne` or equivalent functionality on.
    constexpr RecodeView(range_type range, to_encoding_type toEncoding) noexcept
        : RecodeView(std::move(range), from_encoding_type {}, std::move(toEncoding))
    {
    }

    /// @brief Constructs a RecodeView from the underlying range.
    ///
    /// @param[in] range The input range to wrap and iterate over.
    /// @param[in] fromEncoding The encoding object to call `DecodeOne` or equivalent functionality on.
    /// @param[in] toEncoding The encoding object to call `EncodeOne` or equivalent functionality on.
    constexpr RecodeView(range_type range, from_encoding_type fromEncoding,
                         to_encoding_type toEncoding) noexcept
        : RecodeView(std::move(range), std::move(fromEncoding), std::move(toEncoding),
                     from_error_handler_type {}, to_error_handler_type {})
    {
    }

    /// @brief Constructs a RecodeView from the underlying range.
    ///
    /// @param[in] range The input range to wrap and iterate over.
    /// @param[in] fromEncoding The encoding object to call `DecodeOne` or equivalent functionality on.
    /// @param[in] toEncoding The encoding object to call `EncodeOne` or equivalent functionality on.
    /// @param[in] fromErrorHandler The error handler for decode operations to store in this view.
    /// @param[in] toErrorHandler The error handler for encode operations to store in this view.
    constexpr RecodeView(range_type range, from_encoding_type fromEncoding, to_encoding_type toEncoding,
                         from_error_handler_type fromErrorHandler,
                         to_error_handler_type toErrorHandler) noexcept
        : _it(std::move(range), std::move(fromEncoding), std::move(toEncoding), std::move(fromErrorHandler),
              std::move(toErrorHandler))
    {
    }

    /// @brief Constructs a RecodeView from the underlying range.
    ///
    /// @param[in] range The input range to wrap and iterate over.
    /// @param[in] fromEncoding The encoding object to call `DecodeOne` or equivalent functionality on.
    /// @param[in] toEncoding The encoding object to call `EncodeOne` or equivalent functionality on.
    /// @param[in] fromErrorHandler The error handler for decode operations to store in this view.
    /// @param[in] toErrorHandler The error handler for encode operations to store in this view.
    /// @param[in] fromState The state to user for the decode operation.
    /// @param[in] toState The state to user for the decode operation.
    constexpr RecodeView(range_type range, from_encoding_type fromEncoding, to_encoding_type toEncoding,
                         from_error_handler_type fromErrorHandler, to_error_handler_type toErrorHandler,
                         from_state_type fromState, to_state_type toState) noexcept
        : _it(std::move(range), std::move(fromEncoding), std::move(toEncoding), std::move(fromErrorHandler),
              std::move(toErrorHandler), std::move(fromState), std::move(toState))
    {
    }

    /// @brief The beginning of the range.
    constexpr iterator begin() & noexcept
    {
      if constexpr (CopyConstructible<iterator>)
      {
        return this->_it;
      }
      else
      {
        return std::move(this->_it);
      }
    }

    /// @brief The beginning of the range.
    constexpr iterator begin() const & noexcept
    {
      return this->_it;
    }

    /// @brief The beginning of the range.
    constexpr iterator begin() && noexcept
    {
      return std::move(this->_it);
    }

    /// @brief The end of the range. Uses a sentinel type and not a special iterator.
    constexpr sentinel end() const noexcept
    {
      return sentinel();
    }

  private:
    iterator _it;
  };

}

namespace std::ranges
{

  template <typename TFromEncoding, typename TToEncoding, typename TRange, typename TFromErrorHandler,
            typename TToErrorHandler, typename TFromState, typename TToState>
  constexpr inline bool enable_borrowed_range<Krys::RecodeView<
    TFromEncoding, TToEncoding, TRange, TFromErrorHandler, TToErrorHandler, TFromState, TToState>> =
    Krys::Ranges::enable_borrowed_range<TRange>;

}
