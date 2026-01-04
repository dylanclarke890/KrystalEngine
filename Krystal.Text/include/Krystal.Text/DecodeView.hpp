#pragma once

#include "Krystal.Lib/Ranges/ADL.hpp"
#include "Krystal.Lib/Ranges/Reconstruct.hpp"
#include "Krystal.Lib/Utils/Unwrap.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/DecodeIterator.hpp"
#include "Krystal.Text/Impl/DefaultCharView.hpp"
#include <string_view>

namespace Krys
{
  /// @brief A view over a range of code points, presenting the code points as code units. Uses the
  /// `TEncoding` specified to do so.
  /// @tparam TEncoding The encoding to read the underlying range of code points as.
  /// @tparam TRange The range of input that will be fed into the TFromEncoding's decode operation.
  /// @tparam TErrorHandler The error handler for any encode-step failures.
  /// @tparam TState The state type to use for the decode operations to intermediate code points.
  /// @remarks The view presents code point one at a time, regardless of how many code points are output by
  /// one decode operation. This means if, for example, four (4) UTF-8 code units becomes two (2) UTF-16 code
  /// points, it will present one code point at a time. If you are looking to explicitly know what a single
  /// decode operation maps into as far as number of code points to code units (and vice-versa), you will have
  /// to use lower-level interfaces.
  template <typename TEncoding, typename TRange = Impl::default_char_view_t<code_unit_t<TEncoding>>,
            typename TErrorHandler = Handlers::DefaultHandler, typename TState = decode_state_t<TEncoding>>
  class DecodeView : public Krys::Ranges::ViewBase
  {
  private:
    using TCVRange = unwrap_remove_ref_t<TRange>;
    using TStoredRange = Krys::Ranges::range_reconstruct_t<const TCVRange>;

  public:
    /// @brief The iterator type for this view.
    using iterator = DecodeIterator<TEncoding, TStoredRange, TErrorHandler, TState>;

    /// @brief The sentinel type for this view.
    using sentinel = TEncodingSentinel;

    /// @brief The value type for this view.
    using value_type = Krys::Ranges::iterator_value_type_t<iterator>;

    /// @brief The underlying range type.
    using range_type = TRange;

    /// @brief The encoding type used for transformations.
    using encoding_type = TEncoding;

    /// @brief The error handler when a decode operation fails.
    using error_handler_type = TErrorHandler;

    /// @brief The state type used for decode operations.
    using state_type = decode_state_t<encoding_type>;

    /// @brief Constructs a DecodeView from the underlying range.
    /// @param[in] range The input range to wrap and iterate over.
    /// @remarks The stored encoding, error handler, and state type are default-constructed.
    template <typename TArgRange, enable_if_t<!SameType<remove_cvref_t<TArgRange>, DecodeView>
                                              && !SameType<remove_cvref_t<TArgRange>, iterator>> * = nullptr>
    constexpr DecodeView(TArgRange &&range) noexcept(NoThrowConstructible<iterator, TArgRange>)
        : _it(std::forward<TArgRange>(range))
    {
    }

    /// @brief Constructs a DecodeView from the underlying range.
    /// @param[in] range The input range to wrap and iterate over.
    /// @param[in] encoding The encoding object to call `.decode` or equivalent functionality on.
    constexpr DecodeView(range_type range, encoding_type encoding) noexcept(
      NoThrowConstructible<iterator, range_type, encoding_type>)
        : _it(std::move(range), std::move(encoding))
    {
    }

    /// @brief Constructs a DecodeView from the underlying range.
    /// @param[in] range The input range to wrap and iterate over.
    /// @param[in] encoding The encoding object to call `.decode` or equivalent functionality on.
    /// @param[in] errorHandler The error handler to store in this view.
    constexpr DecodeView(range_type range, encoding_type encoding, error_handler_type errorHandler) noexcept(
      NoThrowConstructible<iterator, range_type, encoding_type, error_handler_type>)
        : _it(std::move(range), std::move(encoding), std::move(errorHandler))
    {
    }

    /// @brief Constructs a DecodeView from the underlying range.
    /// @param[in] range The input range to wrap and iterate over.
    /// @param[in] encoding The encoding object to call `.decode` or equivalent functionality on.
    /// @param[in] errorHandler The error handler to store in this view.
    /// @param[in] state The state to user for the decode operation.
    constexpr DecodeView(range_type range, encoding_type encoding, error_handler_type errorHandler,
                         state_type state) noexcept(NoThrowConstructible<iterator, range_type, encoding_type,
                                                                         error_handler_type, state_type>)
        : _it(std::move(range), std::move(encoding), std::move(errorHandler), std::move(state))
    {
    }

    /// @brief Constructs an encoding_view from one of its iterators, reconstituting the range.
    /// @param[in] it A previously-made DecodeView iterator.
    constexpr DecodeView(iterator it) noexcept(NoThrowMoveConstructible<iterator>) : _it(std::move(it))
    {
    }

    /// @brief Default constructor. Defaulted.
    constexpr DecodeView() = default;

    /// @brief Copy constructor. Defaulted.
    constexpr DecodeView(const DecodeView &) = default;

    /// @brief Move constructor. Defaulted.
    constexpr DecodeView(DecodeView &&) = default;

    /// @brief Copy assignment operator. Defaulted.
    constexpr DecodeView &operator=(const DecodeView &) = default;

    /// @brief Move assignment operator. Defaulted.
    constexpr DecodeView &operator=(DecodeView &&) = default;

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

  /// @brief The reconstruct extension point for rebuilding an encoding view from its iterator and sentinel
  /// type.
  template <typename TEncoding, typename TRange, typename TErrorHandler, typename TState>
  constexpr DecodeView<TEncoding, TRange, TErrorHandler, TState> reconstruct(
    std::in_place_type_t<DecodeView<TEncoding, TRange, TErrorHandler, TState>>,
    typename DecodeView<TEncoding, TRange, TErrorHandler, TState>::iterator it,
    typename DecodeView<TEncoding, TRange, TErrorHandler, TState>::
      sentinel) noexcept(NoThrowConstructible<DecodeView<TEncoding, TRange, TErrorHandler, TState>,
                                              typename DecodeView<TEncoding, TRange, TErrorHandler,
                                                                  TState>::iterator &&>)
  {
    return DecodeView<TEncoding, TRange, TErrorHandler, TState>(std::move(it));
  }

}

namespace std::ranges
{
  template <typename TEncoding, typename TRange, typename TErrorHandler, typename TState>
  constexpr inline bool enable_borrowed_range<Krys::DecodeView<TEncoding, TRange, TErrorHandler, TState>> =
    Krys::Ranges::enable_borrowed_range<TRange>;
}
