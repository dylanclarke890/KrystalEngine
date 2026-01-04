#pragma once

#include "Krystal.Text/Impl/EncodingIterator.hpp"
#include "Krystal.Text/State.hpp"

namespace Krys
{
  /// @brief The encode sentinel to use as the `end` value for a EncodeIterator.
  using TEncodeSentinel = Impl::TEncodingSentinel;

  /// @brief An iterator over an iterator of code points, presenting the code points as code units. Uses the
  /// @p TEncoding specified to do so.
  /// @tparam TEncoding The encoding to read the underlying range of code points as.
  /// @tparam TRange The range of input that will be fed into the TFromEncoding's decode operation.
  /// @tparam TErrorHandler The error handler for any encode-step failures.
  /// @tparam TState The state type to use for the encode operations to intermediate code points.
  /// @remarks This type produces proxies as their reference type, and are only readable, not writable
  /// iterators. The iterator presents code units one at a time, regardless of how many code units are output
  /// by one decode operation. This means if, for example, one (1) UTF-32 code point becomes four (4) UTF-8
  /// code units, it will present each code unit one at a time. If you are looking to explicitly know what a
  /// single encode operation maps into as far as number of code points to code units (and vice-versa), you
  /// will have to use lower-level interfaces.
  template <typename TEncoding, typename TRange, typename TErrorHandler = Handlers::DefaultHandler,
            typename TState = encode_state_t<TEncoding>>
  class EncodeIterator
      : public Impl::EncodingIterator<Impl::Transaction::Encode,
                                      EncodeIterator<TEncoding, TRange, TErrorHandler, TState>,
                                      Impl::IteratorStorage<TEncoding, TRange, TErrorHandler, TState>>
  {
  private:
    using TIteratorBaseIterator =
      Impl::EncodingIterator<Impl::Transaction::Encode,
                             EncodeIterator<TEncoding, TRange, TErrorHandler, TState>,
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
    ///
    /// @remarks It's not a very useful type...
    using difference_type = typename TIteratorBaseIterator::difference_type;

    /// @brief Default constructor. Defaulted.
    constexpr EncodeIterator() = default;

    /// @brief Copy constructor. Defaulted.
    constexpr EncodeIterator(const EncodeIterator &) = default;

    /// @brief Move constructor. Defaulted.
    constexpr EncodeIterator(EncodeIterator &&) = default;

    /// @brief Constructs a DecodeIterator from the explicitly given `range`.
    ///
    /// @param[in] range The range value that will be read from.
    ///
    /// @remarks Each argument is moved/forwarded in.
    template <typename TArgRange>
    requires(!SameType<remove_cvref_t<TArgRange>, EncodeIterator>)
    constexpr EncodeIterator(TArgRange &&range) noexcept(
      NoThrowConstructible<TIteratorBaseIterator, range_type>)
        : TIteratorBaseIterator(std::forward<TArgRange>(range))
    {
    }

    /// @brief Constructs a EncodeIterator from the explicitly given `range`, and `encoding`.
    ///
    /// @param[in] range The range value that will be read from.
    /// @param[in] encoding The encoding object to use.
    ///
    /// @remarks Each argument is moved in.
    constexpr EncodeIterator(range_type range, encoding_type encoding) noexcept(
      NoThrowConstructible<TIteratorBaseIterator, range_type, encoding_type>)
        : TIteratorBaseIterator(std::move(range), std::move(encoding), error_handler_type {})
    {
    }

    /// @brief Constructs a EncodeIterator from the explicitly given `range`, and @p
    /// errorHandler.
    ///
    /// @param[in] range The range value that will be read from.
    /// @param[in] errorHandler The error handler to use for reporting errors.
    ///
    /// @remarks Each argument is moved in.
    constexpr EncodeIterator(range_type range, error_handler_type errorHandler) noexcept(
      NoThrowConstructible<TIteratorBaseIterator, range_type, error_handler_type>)
        : TIteratorBaseIterator(std::move(range), encoding_type {}, std::move(errorHandler))
    {
    }

    /// @brief Constructs a EncodeIterator from the explicitly given `range`, `encoding`, and
    /// `errorHandler`.
    ///
    /// @param[in] range The range value that will be read from.
    /// @param[in] encoding The encoding object to use.
    /// @param[in] errorHandler The error handler to use for reporting errors.
    ///
    /// @remarks Each argument is moved in.
    constexpr EncodeIterator(
      range_type range, encoding_type encoding,
      error_handler_type errorHandler) noexcept(NoThrowConstructible<TIteratorBaseIterator, range_type,
                                                                     encoding_type, error_handler_type>)
        : TIteratorBaseIterator(std::move(range), std::move(encoding), std::move(errorHandler))
    {
    }

    /// @brief Constructs a EncodeIterator from the explicitly given `range`, `encoding`, @p
    /// errorHandler and `state`.
    ///
    /// @param[in] range The range value that will be read from.
    /// @param[in] encoding The encoding object to use.
    /// @param[in] errorHandler The error handler to use for reporting errors.
    /// @param[in] state The current state.
    ///
    /// @remarks Each argument is moved in.
    constexpr EncodeIterator(
      range_type range, encoding_type encoding, error_handler_type errorHandler,
      state_type state) noexcept(NoThrowConstructible<TIteratorBaseIterator, range_type, encoding_type,
                                                      error_handler_type, state_type>)
        : TIteratorBaseIterator(std::move(range), std::move(encoding), std::move(errorHandler),
                                std::move(state))
    {
    }

    /// @brief Copy assignment operator. Defaulted.
    constexpr EncodeIterator &operator=(const EncodeIterator &) = default;

    /// @brief Move assignment operator. Defaulted.
    constexpr EncodeIterator &operator=(EncodeIterator &&) = default;
  };
}
