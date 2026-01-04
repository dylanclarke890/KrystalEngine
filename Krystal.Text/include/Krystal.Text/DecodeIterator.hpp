#pragma once

#include "Krystal.Text/Impl/EncodingIterator.hpp"
#include "Krystal.Text/State.hpp"

namespace Krys
{
  /// @brief The decode sentinel to use as the `end` value for a DecodeIterator.
  using TEncodingSentinel = Impl::TEncodingSentinel;

  /// @brief An iterator over a range of code points, presented as a range of code units, using the
  /// `TEncoding` specified to do so.
  /// @tparam TEncoding The encoding to read the underlying range of code points as.
  /// @tparam TRange The range of input that will be fed into the TFromEncoding's decode operation.
  /// @tparam TErrorHandler The error handler for any encode-step failures.
  /// @tparam TState The state type to use for the encode operations to intermediate code points.
  /// @remarks This type produces proxies as their reference type, and are only readable, not writable
  /// iterators. The iterator presents code point one at a time, regardless of how many code points are output
  /// by one decode operation. This means if, for example, four (4) UTF-8 code units becomes two (2) UTF-16
  /// code points, it will present one code point at a time. If you are looking to explicitly know what a
  /// single decode operation maps into as far as number of code points to code units (and vice-versa), you
  /// will have to use lower-level interfaces.
  template <typename TEncoding, typename TRange, typename TErrorHandler = Handlers::DefaultHandler,
            typename TState = decode_state_t<TEncoding>>
  class DecodeIterator
      : public Impl::EncodingIterator<Impl::Transaction::Decode,
                                      DecodeIterator<TEncoding, TRange, TErrorHandler, TState>,
                                      Impl::IteratorStorage<TEncoding, TRange, TErrorHandler, TState>>
  {
  private:
    using TIteratorBaseIterator =
      Impl::EncodingIterator<Impl::Transaction::Decode,
                             DecodeIterator<TEncoding, TRange, TErrorHandler, TState>,
                             Impl::IteratorStorage<TEncoding, TRange, TErrorHandler, TState>>;

  public:
    /// @brief The underlying range type.
    using range_type = typename TIteratorBaseIterator::range_type;

    /// @brief The base iterator type.
    using iterator = typename TIteratorBaseIterator::iterator;

    /// @brief The encoding type used for transformations.
    using encoding_type = typename TIteratorBaseIterator::encoding_type;

    /// @brief The error handler when an encode operation fails.
    using error_handler_type = typename TIteratorBaseIterator::error_handler_type;

    /// @brief The state type used for encode operations.
    using state_type = typename TIteratorBaseIterator::state_type;

    /// @brief The strength of the iterator category, as defined in relation to the base.
    using iterator_category = typename TIteratorBaseIterator::iterator_category;

    /// @brief The strength of the iterator concept, as defined in relation to the base.
    using iterator_concept = typename TIteratorBaseIterator::iterator_concept;

    /// @brief The object type that gets output on every dereference.
    using value_type = typename TIteratorBaseIterator::value_type;

    /// @brief A pointer type to the value_type.
    using pointer = typename TIteratorBaseIterator::pointer;

    /// @brief The value returned from derefencing the iterator.
    ///
    /// @remarks This is a proxy iterator, so the `reference` is a non-reference `value_type.`
    using reference = typename TIteratorBaseIterator::value_type;

    /// @brief The type returned when two of these pointers are subtracted from one another.
    /// @remarks It's not a very useful type...
    using difference_type = typename TIteratorBaseIterator::difference_type;

    /// @brief Default constructor. Defaulted.
    constexpr DecodeIterator() = default;

    /// @brief Copy constructor. Defaulted.
    constexpr DecodeIterator(const DecodeIterator &) = default;

    /// @brief Move constructor. Defaulted.
    constexpr DecodeIterator(DecodeIterator &&) = default;

    /// @brief Constructs a DecodeIterator from the explicitly given `range`.
    /// @param[in] range The range value that will be read from.
    /// @remarks Each argument is moved/forwarded in.
    template <typename TArgRange>
    requires(!SameType<remove_cvref_t<TArgRange>, DecodeIterator>)
    constexpr DecodeIterator(TArgRange &&range) noexcept(
      NoThrowConstructible<TIteratorBaseIterator, range_type>)
        : TIteratorBaseIterator(std::forward<TArgRange>(range))
    {
    }

    /// @brief Constructs a DecodeIterator from the explicitly given `range`, and `encoding`.
    /// @param[in] range The range value that will be read from.
    /// @param[in] encoding The encoding object to use.
    /// @remarks Each argument is moved in.
    constexpr DecodeIterator(range_type range, encoding_type encoding) noexcept(
      NoThrowConstructible<TIteratorBaseIterator, range_type, encoding_type>)
        : TIteratorBaseIterator(std::move(range), std::move(encoding))
    {
    }

    /// @brief Constructs a DecodeIterator from the explicitly given `range`, and @p
    /// errorHandler.
    /// @param[in] range The range value that will be read from.
    /// @param[in] errorHandler The error handler to use for reporting errors.
    /// @remarks Each argument is moved in.
    constexpr DecodeIterator(range_type range, error_handler_type errorHandler) noexcept(
      NoThrowConstructible<TIteratorBaseIterator, range_type, error_handler_type>)
        : TIteratorBaseIterator(std::move(range), std::move(errorHandler))
    {
    }

    /// @brief Constructs a DecodeIterator from the explicitly given `range`, `encoding`, and
    /// `errorHandler`.
    /// @param[in] range The range value that will be read from.
    /// @param[in] encoding The encoding object to use.
    /// @param[in] errorHandler The error handler to use for reporting errors.
    /// @remarks Each argument is moved in.
    constexpr DecodeIterator(
      range_type range, encoding_type encoding,
      error_handler_type errorHandler) noexcept(NoThrowConstructible<TIteratorBaseIterator, range_type,
                                                                     encoding_type, error_handler_type>)
        : TIteratorBaseIterator(std::move(range), std::move(encoding), std::move(errorHandler))
    {
    }

    /// @brief Constructs a DecodeIterator from the explicitly given `range`, `encoding`, @p
    /// errorHandler and `state`.
    /// @param[in] range The range value that will be read from.
    /// @param[in] encoding The encoding object to use.
    /// @param[in] errorHandler The error handler to use for reporting errors.
    /// @param[in] state The current state.
    /// @remarks Each argument is moved in.
    constexpr DecodeIterator(
      range_type range, encoding_type encoding, error_handler_type errorHandler,
      state_type state) noexcept(NoThrowConstructible<TIteratorBaseIterator, range_type, encoding_type,
                                                      error_handler_type, state_type>)
        : TIteratorBaseIterator(std::move(range), std::move(encoding), std::move(errorHandler),
                                std::move(state))
    {
    }

    /// @brief Copy assignment operator. Defaulted.
    constexpr DecodeIterator &operator=(const DecodeIterator &) = default;

    /// @brief Move assignment operator. Defaulted.
    constexpr DecodeIterator &operator=(DecodeIterator &&) = default;
  };
}
