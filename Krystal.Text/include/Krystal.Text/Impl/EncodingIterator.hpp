#pragma once

#include "Krystal.Text/IsIgnorableErrorHandler.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/Impl/EncodingIteratorStorage.hpp"
#include <ztd/text/detail/encoding_range.hpp>
#include <ztd/text/detail/transcode_routines.hpp>
#include <ztd/text/detail/update_input.hpp>
#include <ztd/text/error_handler_always_returns_ok.hpp>
#include <ztd/text/transcode_result.hpp>

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Utils/EBCO.hpp"
#include "Krystal.Lib/Utils/Unwrap.hpp"
#include "Krystal.Lib/Ranges/ADL.hpp"
#include "Krystal.Lib/Ranges/BlackholeIterator.hpp"
#include "Krystal.Lib/Ranges/DefaultSentinel.hpp"
#include "Krystal.Lib/Ranges/Subrange.hpp"
#include "Krystal.Lib/Ranges/Unbounded.hpp"
#include <array>

namespace Krys
{

  namespace Impl
  {

    template <typename TEncoding, typename TInput, typename TErrorHandler>
    inline constexpr bool __is_encoding_with_error_handler_never_returns_error_v = false;

    using __encoding_sentinel_t = ranges::default_sentinel_t;

    template <__transaction _EncodeOrDecode, typename TDerived, typename _Storage>
    class __encoding_iterator
        : private _Storage,
          private CursorCache<
            (_EncodeOrDecode == __transaction::Decode
               ? MaxCodePoints<remove_cvref_t<unwrap_t<typename _Storage::encoding_type>>>
               : MaxCodeUnits<remove_cvref_t<unwrap_t<typename _Storage::encoding_type>>>),
            ranges::is_range_input_or_output_range_exactly_v<
              remove_cvref_t<unwrap_t<typename _Storage::range_type>>>>,
          private ErrorCache<(_EncodeOrDecode == __transaction::Decode
                                   ? DecodeErrorHandlerAlwaysReturnsOK<
                                       remove_cvref_t<unwrap_t<typename _Storage::encoding_type>>,
                                       remove_cvref_t<unwrap_t<typename _Storage::error_handler_type>>>
                                   : EncodeErrorHandlerAlwaysReturnsOK<
                                       remove_cvref_t<unwrap_t<typename _Storage::encoding_type>>,
                                       remove_cvref_t<unwrap_t<typename _Storage::error_handler_type>>>)>
    {
    private:
      using TRange = typename _Storage::range_type;
      using TEncoding = typename _Storage::encoding_type;
      using TState = typename _Storage::state_type;
      using TErrorHandler = typename _Storage::error_handler_type;
      using TURange = unwrap_remove_cvref_t<TRange>;
      using TUEncoding = unwrap_remove_cvref_t<TEncoding>;
      using TUErrorHandler = unwrap_remove_cvref_t<TErrorHandler>;
      using TUState = unwrap_remove_cvref_t<TState>;
      using _BaseIterator = ranges::range_const_iterator_t<TURange>;
      inline static constexpr std::size_t _MaxValues =
        (_EncodeOrDecode == __transaction::Decode ? MaxCodePoints<unwrap_remove_cvref_t<TEncoding>>
                                                    : MaxCodeUnits<unwrap_remove_cvref_t<TEncoding>>);
      inline static constexpr bool _IsSingleValueType = _MaxValues == 1;
      inline static constexpr bool _IsInputOrOutput =
        ranges::is_range_input_or_output_range_exactly_v<TURange>;
      inline static constexpr bool _IsCursorless = _IsSingleValueType && !_IsInputOrOutput;
      inline static constexpr bool _IsErrorless =
        _EncodeOrDecode == __transaction::Decode
          ? DecodeErrorHandlerAlwaysReturnsOK<TUEncoding, TUErrorHandler>
          : EncodeErrorHandlerAlwaysReturnsOK<TUEncoding, TUErrorHandler>;
      using __base_cursor_cache_t = CursorCache<_MaxValues, _IsInputOrOutput>;
      using __base_cursor_cache_size_t = typename __base_cursor_cache_t::TSize;
      using __base_error_cache_t = ErrorCache<_IsErrorless>;
      using __base_storage_t = _Storage;

      inline static constexpr bool _IsBackwards =
        _EncodeOrDecode == __transaction::Encode
          ? is_detected_v<HasEncodeOneBackwardsMember, TUEncoding, TURange, TUErrorHandler, TUState>
          : is_detected_v<HasDecodeOneBackwardsMember, TUEncoding, TURange, TUErrorHandler, TUState>;

    public:
      /// @brief The underlying range type.
      using range_type = TRange;

      /// @brief The base iterator type.
      using iterator = _BaseIterator;

      /// @brief The encoding type used for transformations.
      using encoding_type = TEncoding;

      /// @brief The error handler when an encode operation fails.
      using error_handler_type = TErrorHandler;

      /// @brief The state type used for encode operations.
      using state_type = remove_cvref_t<TState>;

      /// @brief The strength of the iterator category, as defined in relation to the base.
      using iterator_category =
        conditional_t<ranges::is_iterator_concept_or_better_v<std::bidirectional_iterator_tag, _BaseIterator>,
                      conditional_t<_IsBackwards, std::bidirectional_iterator_tag, std::forward_iterator_tag>,
                      ranges::iterator_category_t<_BaseIterator>>;

      /// @brief The strength of the iterator concept, as defined in relation to the base.
      using iterator_concept =
        conditional_t<ranges::is_iterator_concept_or_better_v<std::bidirectional_iterator_tag, _BaseIterator>,
                      conditional_t<_IsBackwards, std::bidirectional_iterator_tag, std::forward_iterator_tag>,
                      ranges::iterator_concept_t<_BaseIterator>>;

      /// @brief The object type that gets output on every dereference.
      using value_type = conditional_t<_EncodeOrDecode == __transaction::Encode, code_unit_t<TEncoding>,
                                       code_point_t<TEncoding>>;

      /// @brief A pointer type to the value_type.
      using pointer = value_type *;

      /// @brief The value returned from derefencing the iterator.
      ///
      /// @remarks This is a proxy iterator, so the `reference` is a non-reference `value_type.`
      using reference = value_type;

      /// @brief The type returned when two of these pointers are subtracted from one another.
      ///
      /// @remarks It's not a very useful type...
      using difference_type = ranges::iterator_difference_type_t<_BaseIterator>;

      constexpr __encoding_iterator() = default;

      constexpr __encoding_iterator(const __encoding_iterator &) = default;
      constexpr __encoding_iterator(__encoding_iterator &&) = default;

      template <typename _ArgRange,
                enable_if_t<!SameType<remove_cvref_t<_ArgRange>, __encoding_iterator>> * = nullptr>
      constexpr __encoding_iterator(_ArgRange &&range) noexcept(noexcept(__encoding_iterator(
        std::declval<range_type>(), std::declval<encoding_type>(), std::declval<error_handler_type>())))
          : __encoding_iterator(std::forward<_ArgRange>(range), encoding_type {}, error_handler_type {})
      {
      }

      constexpr __encoding_iterator(range_type range, encoding_type encoding) noexcept(
        noexcept(__encoding_iterator(std::declval<range_type>(), std::declval<encoding_type>(),
                                     std::declval<error_handler_type>())))
          : __encoding_iterator(std::move(range), std::move(encoding), error_handler_type {})
      {
      }

      constexpr __encoding_iterator(range_type range, error_handler_type errorHandler) noexcept(
        noexcept(__encoding_iterator(std::declval<range_type>(), std::declval<encoding_type>(),
                                     std::declval<error_handler_type>())))
          : __encoding_iterator(std::move(range), encoding_type {}, std::move(errorHandler))
      {
      }

      constexpr __encoding_iterator(range_type range, encoding_type encoding,
                                    error_handler_type errorHandler) noexcept( // cf
        NoThrowConstructible<__base_storage_t, range_type, encoding_type, error_handler_type>)
          : __base_storage_t(std::move(range), std::move(encoding), std::move(errorHandler)),
            __base_cursor_cache_t(), __base_error_cache_t(), _M_cache()
      {
        this->_M_read_one();
      }

      constexpr __encoding_iterator(range_type range, encoding_type encoding,
                                    error_handler_type errorHandler, state_type state) noexcept( // cf
        NoThrowConstructible<__base_storage_t, range_type, encoding_type, error_handler_type,
                                        state_type>)
          : __base_storage_t(std::move(range), std::move(encoding), std::move(errorHandler),
                             std::move(state)),
            __base_cursor_cache_t(), __base_error_cache_t(), _M_cache()
      {
        this->_M_read_one();
      }

      // assignment
      constexpr __encoding_iterator &operator=(const __encoding_iterator &) = default;
      constexpr __encoding_iterator &operator=(__encoding_iterator &&) = default;

      /// @brief The encoding object.
      ///
      /// @returns A const l-value reference to the encoding object used to construct this iterator.
      constexpr const encoding_type &encoding() const noexcept
      {
        return this->__base_storage_t::GetEncoding();
      }

      /// @brief The encoding object.
      ///
      /// @returns An l-value reference to the encoding object used to construct this iterator.
      constexpr encoding_type &encoding() noexcept
      {
        return this->__base_storage_t::GetEncoding();
      }

      /// @brief The state object.
      ///
      /// @returns A const l-value reference to the state object used to construct this iterator.
      constexpr const state_type &state() const noexcept
      {
        return this->__base_storage_t::GetEncoding();
      }

      /// @brief The state object.
      ///
      /// @returns An l-value reference to the state object used to construct this iterator.
      constexpr state_type &state() noexcept
      {
        return this->__base_storage_t::GetState();
      }

      /// @brief The error handler object.
      ///
      /// @returns A const l-value reference to the error handler used to construct this iterator.
      constexpr const error_handler_type &error_handler() const & noexcept
      {
        return this->__base_storage_t::GetErrorHandler();
      }

      /// @brief The error handler object.
      ///
      /// @returns An l-value reference to the error handler used to construct this iterator.
      constexpr error_handler_type &error_handler() & noexcept
      {
        return this->__base_storage_t::GetErrorHandler();
      }

      /// @brief The error handler object.
      ///
      /// @returns An r-value reference to the error handler used to construct this iterator.
      constexpr error_handler_type &&error_handler() && noexcept
      {
        return this->__base_storage_t::GetErrorHandler();
      }

      /// @brief The input range used to construct this object.
      ///
      /// @returns A const l-value reference to the input range used to construct this iterator.
      constexpr range_type range() const noexcept(NoThrowMoveConstructible<range_type>)
      {
        return Krys::Ranges::reconstruct(std::in_place_type<range_type>,
                                          this->__base_storage_t::GetRange());
      }

      /// @brief Returns whether the last read operation had an encoding error or not.
      ///
      /// @returns The EncodingError that occurred. This can be EncodingError::OK for
      /// an operation that went just fine.
      ///
      /// @remarks If the error handler is identified as an error handler that, if given a suitably sized
      /// buffer, will never return an error. This is the case with specific encoding operations with
      /// Handlers::ReplacementHandler, or Handlers::ThrowHandler.
      constexpr EncodingError ErrorCode() const noexcept
      {
        if constexpr (_IsErrorless)
        {
          return EncodingError::OK;
        }
        else
        {
          return this->__base_error_cache_t::ToError();
        }
      }

      /// @brief Copy then increment the iterator.
      ///
      /// @returns A copy of iterator, before incrementing.
      constexpr TDerived operator++(int)
      {
        TDerived __copy = this->_M_derived();
        ++(*this);
        return __copy;
      }

      /// @brief Increment the iterator.
      ///
      /// @returns A reference to *this, after incrementing the iterator.
      constexpr TDerived &operator++()
      {
        if constexpr (_IsSingleValueType)
        {
          this->_M_read_one();
        }
        else
        {
          ++this->__base_cursor_cache_t::Position;
          if (this->__base_cursor_cache_t::Position == this->__base_cursor_cache_t::Size)
          {
            this->_M_read_one();
          }
        }
        return this->_M_derived();
      }

      /// @brief Dereference the iterator.
      ///
      /// @remarks This is a proxy iterator, and therefore only returns a const value_type& object and not a
      /// non-const reference object. This is deliberately to work around the standard not allowing
      /// stand-alone proxy iterators that do not return reference types. Encoding iterators are only
      /// readable, not writable.
      constexpr reference operator*() const noexcept
      {
        if constexpr (_IsSingleValueType)
        {
          return this->_M_cache[0];
        }
        else
        {
          return this->_M_cache[this->__base_cursor_cache_t::Position];
        }
      }

      // observers: comparison

      /// @brief Compares whether or not this iterator has truly reached the end.
      friend constexpr bool operator==(const TDerived &it, const __encoding_sentinel_t &)
      {
        if constexpr (_IsCursorless || (_IsInputOrOutput && _IsSingleValueType))
        {
          return it._M_base_is_empty()
                 && static_cast<__base_cursor_cache_size_t>(Impl::CursorlessSizeSentinel)
                      == it.__base_cursor_cache_t::Size;
        }
        else
        {
          return it._M_base_is_empty()
                 && it.__base_cursor_cache_t::Position == it.__base_cursor_cache_t::Size;
        }
      }

      /// @brief Compares whether or not this iterator has truly reached the end.
      friend constexpr bool operator==(const __encoding_sentinel_t &__sen, const TDerived &it)
      {
        return it == __sen;
      }

      /// @brief Compares whether or not this iterator has truly reached the end.
      template <
        typename _Concept = iterator_concept,
        enable_if_t<Krys::Ranges::is_concept_or_better_v<std::forward_iterator_tag, _Concept>> * = nullptr>
      friend constexpr bool operator==(const TDerived &it, const TDerived &__sen)
      {
        if constexpr (_IsCursorless || (_IsInputOrOutput && _IsSingleValueType))
        {
          return Krys::Ranges::begin(it.__base_storage_t::GetRange())
                 == Krys::Ranges::begin(__sen.__base_storage_t::GetRange());
        }
        else
        {
          return Krys::Ranges::begin(it.__base_storage_t::GetRange())
                   == Krys::Ranges::begin(__sen.__base_storage_t::GetRange())
                 && it.__base_cursor_cache_t::Position == __sen.__base_cursor_cache_t::Position;
        }
      }

      /// @brief Compares whether or not this iterator has truly reached the end.
      friend constexpr bool operator!=(const TDerived &it, const __encoding_sentinel_t &)
      {
        if constexpr (_IsCursorless || (_IsInputOrOutput && _IsSingleValueType))
        {
          return !it._M_base_is_empty()
                 || static_cast<__base_cursor_cache_size_t>(Impl::CursorlessSizeSentinel)
                      != it.__base_cursor_cache_t::Size;
        }
        else
        {
          return !it._M_base_is_empty()
                 || it.__base_cursor_cache_t::Position != it.__base_cursor_cache_t::Size;
        }
      }

      /// @brief Compares whether or not this iterator has truly reached the end.
      friend constexpr bool operator!=(const __encoding_sentinel_t &__sen, const TDerived &it) noexcept
      {
        return it != __sen;
      }

      /// @brief Compares whether or not this iterator has truly reached the end.
      template <
        typename _Concept = iterator_concept,
        enable_if_t<Krys::Ranges::is_concept_or_better_v<std::forward_iterator_tag, _Concept>> * = nullptr>
      friend constexpr bool operator!=(const TDerived &it, const TDerived &__sen)
      {
        return !(it == __sen);
      }

    private:
      constexpr bool _M_base_is_empty() const noexcept
      {
        if constexpr (is_detected_v<ranges::detect_adl_empty, TRange>)
        {
          return Krys::Ranges::empty(this->__base_storage_t::GetRange());
        }
        else
        {
          return Krys::Ranges::begin(this->__base_storage_t::GetRange())
                 == Krys::Ranges::end(this->__base_storage_t::GetRange());
        }
      }

      constexpr void _M_read_one()
      {
        if (this->_M_base_is_empty())
        {
          if constexpr (_IsCursorless || (_IsSingleValueType && _IsInputOrOutput))
          {
            this->__base_cursor_cache_t::Size =
              static_cast<__base_cursor_cache_size_t>(CursorlessSizeSentinel);
          }
          else
          {
            this->__base_cursor_cache_t::Size =
              static_cast<__base_cursor_cache_size_t>(this->_M_cache.size());
            this->__base_cursor_cache_t::Position =
              static_cast<__base_cursor_cache_size_t>(this->_M_cache.size());
          }
          return;
        }
        auto &__this_input_range = this->_M_range();
        auto __this_cache_begin = this->_M_cache.data();
        KRYS_MAYBE_UNUSED decltype(__this_cache_begin) __this_cache_end {};
        Span<value_type, _MaxValues> __cache_view(this->_M_cache);
        if constexpr (_IsInputOrOutput)
        {
          auto result = __basic_encode_or_decode_one<__consume::__no, _EncodeOrDecode>(
            std::move(__this_input_range), this->encoding(), __cache_view, this->error_handler(),
            this->state());
          __this_cache_end = std::to_address(Krys::Ranges::begin(result.Output));
          if constexpr (!_IsErrorless)
          {
            this->__base_error_cache_t::SetErrors(EncodingError::OK, result.ErrorCode);
          }
          this->__base_storage_t::GetRange() = std::move(result.Input);
        }
        else
        {
          auto result = __basic_encode_or_decode_one<__consume::__no, _EncodeOrDecode>(
            std::move(__this_input_range), this->encoding(), __cache_view, this->error_handler(),
            this->state());
          __this_cache_end = std::to_address(Krys::Ranges::begin(result.Output));
          if constexpr (!_IsErrorless)
          {
            this->__base_error_cache_t::SetErrors(EncodingError::OK, result.ErrorCode);
          }
          this->__base_storage_t::GetRange() = Impl::__update_input<TRange>(std::move(result.Input));
        }
        if constexpr (!_IsSingleValueType)
        {
          __base_cursor_cache_size_t __data_size =
            static_cast<__base_cursor_cache_size_t>(__this_cache_end - __this_cache_begin);
          ZTD_TEXT_ASSERT_MESSAGE_I_("size of produced value can never be bigger thanthe cache",
                                     static_cast<std::size_t>(__data_size) <= this->_M_cache.size());
          this->__base_cursor_cache_t::Position = static_cast<__base_cursor_cache_size_t>(0);
          this->__base_cursor_cache_t::Size = __data_size;
        }
      }

      constexpr TDerived &_M_derived() noexcept
      {
        return static_cast<TDerived &>(*this);
      }

      constexpr const TDerived &_M_derived() const noexcept
      {
        return static_cast<const TDerived &>(*this);
      }

      constexpr auto &_M_range() noexcept
      {
        return this->__base_storage_t::GetRange();
      }

      constexpr const auto &_M_range() const noexcept
      {
        return this->__base_storage_t::GetRange();
      }

      std::array<value_type, _MaxValues> _M_cache;
    };

  }

}

#endif
