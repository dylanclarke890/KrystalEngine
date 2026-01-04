#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/ADL.hpp"
#include "Krystal.Lib/Ranges/BlackholeIterator.hpp"
#include "Krystal.Lib/Ranges/DefaultSentinel.hpp"
#include "Krystal.Lib/Ranges/Subrange.hpp"
#include "Krystal.Lib/Ranges/Unbounded.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Utils/EBCO.hpp"
#include "Krystal.Lib/Utils/Unwrap.hpp"
#include "Krystal.Text/ErrorHandlerAlwaysReturnsOk.hpp"
#include "Krystal.Text/Impl/EncodingIteratorStorage.hpp"
#include "Krystal.Text/Impl/EncodingRange.hpp"
#include "Krystal.Text/Impl/TranscodeRoutines.hpp"
#include "Krystal.Text/Impl/UpdateInput.hpp"
#include "Krystal.Text/IsIgnorableErrorHandler.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/TranscodeResult.hpp"
#include <array>

namespace Krys
{
  namespace Impl
  {
    using TEncodingSentinel = Krys::Ranges::DefaultSentinel;

    template <Transaction TEncodeOrDecode, typename TDerived, typename TStorage>
    class EncodingIterator
        : private TStorage,
          private CursorCache<
            (TEncodeOrDecode == Transaction::Decode
               ? MaxCodePoints<remove_cvref_t<unwrap_t<typename TStorage::encoding_type>>>
               : MaxCodeUnits<remove_cvref_t<unwrap_t<typename TStorage::encoding_type>>>),
            Krys::Ranges::InputOrOutputRangeExact<remove_cvref_t<unwrap_t<typename TStorage::range_type>>>>,
          private ErrorCache<(TEncodeOrDecode == Transaction::Decode
                                ? DecodeErrorHandlerAlwaysReturnsOK<
                                    remove_cvref_t<unwrap_t<typename TStorage::encoding_type>>,
                                    remove_cvref_t<unwrap_t<typename TStorage::error_handler_type>>>
                                : EncodeErrorHandlerAlwaysReturnsOK<
                                    remove_cvref_t<unwrap_t<typename TStorage::encoding_type>>,
                                    remove_cvref_t<unwrap_t<typename TStorage::error_handler_type>>>)>
    {
    private:
      using TRange = typename TStorage::range_type;
      using TEncoding = typename TStorage::encoding_type;
      using TState = typename TStorage::state_type;
      using TErrorHandler = typename TStorage::error_handler_type;
      using TURange = unwrap_remove_cvref_t<TRange>;
      using TUEncoding = unwrap_remove_cvref_t<TEncoding>;
      using TUErrorHandler = unwrap_remove_cvref_t<TErrorHandler>;
      using TUState = unwrap_remove_cvref_t<TState>;
      using TBaseIterator = Krys::Ranges::range_const_iterator_t<TURange>;
      inline static constexpr std::size_t MaxValues =
        (TEncodeOrDecode == Transaction::Decode ? MaxCodePoints<unwrap_remove_cvref_t<TEncoding>>
                                                : MaxCodeUnits<unwrap_remove_cvref_t<TEncoding>>);
      inline static constexpr bool IsSingleValueType = MaxValues == 1;
      inline static constexpr bool IsInputOrOutput = Krys::Ranges::InputOrOutputRangeExact<TURange>;
      inline static constexpr bool IsCursorless = IsSingleValueType && !IsInputOrOutput;
      inline static constexpr bool IsErrorless =
        TEncodeOrDecode == Transaction::Decode
          ? DecodeErrorHandlerAlwaysReturnsOK<TUEncoding, TUErrorHandler>
          : EncodeErrorHandlerAlwaysReturnsOK<TUEncoding, TUErrorHandler>;
      using TBaseCursorCache = CursorCache<MaxValues, IsInputOrOutput>;
      using TBaseCursorCacheSize = typename TBaseCursorCache::TSize;
      using TBaseErrorCache = ErrorCache<IsErrorless>;
      using TBaseStorage = TStorage;

      inline static constexpr bool IsBackwards =
        TEncodeOrDecode == Transaction::Encode
          ? HasEncodeOneBackwardsMember<TUEncoding, TURange, TUErrorHandler, TUState>
          : HasDecodeOneBackwardsMember<TUEncoding, TURange, TUErrorHandler, TUState>;

    public:
      /// @brief The underlying range type.
      using range_type = TRange;

      /// @brief The base iterator type.
      using iterator = TBaseIterator;

      /// @brief The encoding type used for transformations.
      using encoding_type = TEncoding;

      /// @brief The error handler when an encode operation fails.
      using error_handler_type = TErrorHandler;

      /// @brief The state type used for encode operations.
      using state_type = remove_cvref_t<TState>;

      /// @brief The strength of the iterator category, as defined in relation to the base.
      using iterator_category =
        conditional_t<Krys::Ranges::IsIteratorConceptOrBetter<std::bidirectional_iterator_tag, TBaseIterator>,
                      conditional_t<IsBackwards, std::bidirectional_iterator_tag, std::forward_iterator_tag>,
                      Krys::Ranges::iterator_category_t<TBaseIterator>>;

      /// @brief The strength of the iterator concept, as defined in relation to the base.
      using iterator_concept =
        conditional_t<Krys::Ranges::IsIteratorConceptOrBetter<std::bidirectional_iterator_tag, TBaseIterator>,
                      conditional_t<IsBackwards, std::bidirectional_iterator_tag, std::forward_iterator_tag>,
                      Krys::Ranges::iterator_concept_t<TBaseIterator>>;

      /// @brief The object type that gets output on every dereference.
      using value_type = conditional_t<TEncodeOrDecode == Transaction::Encode, code_unit_t<TEncoding>,
                                       code_point_t<TEncoding>>;

      /// @brief A pointer type to the value_type.
      using pointer = value_type *;

      /// @brief The value returned from derefencing the iterator.
      /// @remarks This is a proxy iterator, so the `reference` is a non-reference `value_type.`
      using reference = value_type;

      /// @brief The type returned when two of these pointers are subtracted from one another.
      /// @remarks It's not a very useful type...
      using difference_type = Krys::Ranges::iterator_difference_type_t<TBaseIterator>;

      constexpr EncodingIterator() = default;

      constexpr EncodingIterator(const EncodingIterator &) = default;
      constexpr EncodingIterator(EncodingIterator &&) = default;

      template <typename TArgRange>
      requires(!SameType<remove_cvref_t<TArgRange>, EncodingIterator>)
      constexpr EncodingIterator(TArgRange &&range) noexcept(noexcept(EncodingIterator(
        std::declval<range_type>(), std::declval<encoding_type>(), std::declval<error_handler_type>())))
          : EncodingIterator(std::forward<TArgRange>(range), encoding_type {}, error_handler_type {})
      {
      }

      constexpr EncodingIterator(range_type range, encoding_type encoding) noexcept(noexcept(EncodingIterator(
        std::declval<range_type>(), std::declval<encoding_type>(), std::declval<error_handler_type>())))
          : EncodingIterator(std::move(range), std::move(encoding), error_handler_type {})
      {
      }

      constexpr EncodingIterator(range_type range, error_handler_type errorHandler) noexcept(
        noexcept(EncodingIterator(std::declval<range_type>(), std::declval<encoding_type>(),
                                  std::declval<error_handler_type>())))
          : EncodingIterator(std::move(range), encoding_type {}, std::move(errorHandler))
      {
      }

      constexpr EncodingIterator(range_type range, encoding_type encoding,
                                 error_handler_type errorHandler) noexcept( // cf
        NoThrowConstructible<TBaseStorage, range_type, encoding_type, error_handler_type>)
          : TBaseStorage(std::move(range), std::move(encoding), std::move(errorHandler)), TBaseCursorCache(),
            TBaseErrorCache(), _cache()
      {
        this->ReadOne();
      }

      constexpr EncodingIterator(range_type range, encoding_type encoding, error_handler_type errorHandler,
                                 state_type state) noexcept( // cf
        NoThrowConstructible<TBaseStorage, range_type, encoding_type, error_handler_type, state_type>)
          : TBaseStorage(std::move(range), std::move(encoding), std::move(errorHandler), std::move(state)),
            TBaseCursorCache(), TBaseErrorCache(), _cache()
      {
        this->ReadOne();
      }

      // assignment
      constexpr EncodingIterator &operator=(const EncodingIterator &) = default;
      constexpr EncodingIterator &operator=(EncodingIterator &&) = default;

      /// @brief The encoding object.
      /// @returns A const l-value reference to the encoding object used to construct this iterator.
      constexpr const encoding_type &encoding() const noexcept
      {
        return this->TBaseStorage::GetEncoding();
      }

      /// @brief The encoding object.
      /// @returns An l-value reference to the encoding object used to construct this iterator.
      constexpr encoding_type &encoding() noexcept
      {
        return this->TBaseStorage::GetEncoding();
      }

      /// @brief The state object.
      /// @returns A const l-value reference to the state object used to construct this iterator.
      constexpr const state_type &state() const noexcept
      {
        return this->TBaseStorage::GetEncoding();
      }

      /// @brief The state object.
      /// @returns An l-value reference to the state object used to construct this iterator.
      constexpr state_type &state() noexcept
      {
        return this->TBaseStorage::GetState();
      }

      /// @brief The error handler object.
      /// @returns A const l-value reference to the error handler used to construct this iterator.
      constexpr const error_handler_type &ErrorHandler() const & noexcept
      {
        return this->TBaseStorage::GetErrorHandler();
      }

      /// @brief The error handler object.
      /// @returns An l-value reference to the error handler used to construct this iterator.
      constexpr error_handler_type &ErrorHandler() & noexcept
      {
        return this->TBaseStorage::GetErrorHandler();
      }

      /// @brief The error handler object.
      /// @returns An r-value reference to the error handler used to construct this iterator.
      constexpr error_handler_type &&ErrorHandler() && noexcept
      {
        return this->TBaseStorage::GetErrorHandler();
      }

      /// @brief The input range used to construct this object.
      /// @returns A const l-value reference to the input range used to construct this iterator.
      constexpr range_type range() const noexcept(NoThrowMoveConstructible<range_type>)
      {
        return Krys::Ranges::reconstruct(std::in_place_type<range_type>, this->TBaseStorage::GetRange());
      }

      /// @brief Returns whether the last read operation had an encoding error or not.
      /// @returns The EncodingError that occurred. This can be EncodingError::OK for
      /// an operation that went just fine.
      /// @remarks If the error handler is identified as an error handler that, if given a suitably sized
      /// buffer, will never return an error. This is the case with specific encoding operations with
      /// Handlers::ReplacementHandler, or Handlers::ThrowHandler.
      constexpr EncodingError ErrorCode() const noexcept
      {
        if constexpr (IsErrorless)
        {
          return EncodingError::OK;
        }
        else
        {
          return this->TBaseErrorCache::ToError();
        }
      }

      /// @brief Copy then increment the iterator.
      /// @returns A copy of iterator, before incrementing.
      constexpr TDerived operator++(int)
      {
        TDerived _copy = this->_derived();
        ++(*this);
        return _copy;
      }

      /// @brief Increment the iterator.
      /// @returns A reference to *this, after incrementing the iterator.
      constexpr TDerived &operator++()
      {
        if constexpr (IsSingleValueType)
        {
          this->ReadOne();
        }
        else
        {
          ++this->TBaseCursorCache::Position;
          if (this->TBaseCursorCache::Position == this->TBaseCursorCache::Size)
          {
            this->ReadOne();
          }
        }
        return this->_derived();
      }

      /// @brief Dereference the iterator.
      /// @remarks This is a proxy iterator, and therefore only returns a const value_type& object and not a
      /// non-const reference object. This is deliberately to work around the standard not allowing
      /// stand-alone proxy iterators that do not return reference types. Encoding iterators are only
      /// readable, not writable.
      constexpr reference operator*() const noexcept
      {
        if constexpr (IsSingleValueType)
        {
          return this->_cache[0];
        }
        else
        {
          return this->_cache[this->TBaseCursorCache::Position];
        }
      }

      // observers: comparison

      /// @brief Compares whether or not this iterator has truly reached the end.
      friend constexpr bool operator==(const TDerived &it, const TEncodingSentinel &)
      {
        if constexpr (IsCursorless || (IsInputOrOutput && IsSingleValueType))
        {
          return it.IsBaseEmpty()
                 && static_cast<TBaseCursorCacheSize>(Impl::CursorlessSizeSentinel)
                      == it.TBaseCursorCache::Size;
        }
        else
        {
          return it.IsBaseEmpty() && it.TBaseCursorCache::Position == it.TBaseCursorCache::Size;
        }
      }

      /// @brief Compares whether or not this iterator has truly reached the end.
      friend constexpr bool operator==(const TEncodingSentinel &sentinel, const TDerived &it)
      {
        return it == sentinel;
      }

      /// @brief Compares whether or not this iterator has truly reached the end.
      template <typename TConcept = iterator_concept>
      requires(Krys::Ranges::IsConceptOrBetter<std::forward_iterator_tag, TConcept>)
      friend constexpr bool operator==(const TDerived &it, const TDerived &sentinel)
      {
        if constexpr (IsCursorless || (IsInputOrOutput && IsSingleValueType))
        {
          return Krys::Ranges::begin(it.TBaseStorage::GetRange())
                 == Krys::Ranges::begin(sentinel.TBaseStorage::GetRange());
        }
        else
        {
          return Krys::Ranges::begin(it.TBaseStorage::GetRange())
                   == Krys::Ranges::begin(sentinel.TBaseStorage::GetRange())
                 && it.TBaseCursorCache::Position == sentinel.TBaseCursorCache::Position;
        }
      }

      /// @brief Compares whether or not this iterator has truly reached the end.
      friend constexpr bool operator!=(const TDerived &it, const TEncodingSentinel &)
      {
        if constexpr (IsCursorless || (IsInputOrOutput && IsSingleValueType))
        {
          return !it.IsBaseEmpty()
                 || static_cast<TBaseCursorCacheSize>(Impl::CursorlessSizeSentinel)
                      != it.TBaseCursorCache::Size;
        }
        else
        {
          return !it.IsBaseEmpty() || it.TBaseCursorCache::Position != it.TBaseCursorCache::Size;
        }
      }

      /// @brief Compares whether or not this iterator has truly reached the end.
      friend constexpr bool operator!=(const TEncodingSentinel &sentinel, const TDerived &it) noexcept
      {
        return it != sentinel;
      }

      /// @brief Compares whether or not this iterator has truly reached the end.
      template <typename TConcept = iterator_concept>
      requires(Krys::Ranges::IsConceptOrBetter<std::forward_iterator_tag, TConcept>)
      friend constexpr bool operator!=(const TDerived &it, const TDerived &sentinel)
      {
        return !(it == sentinel);
      }

    private:
      constexpr bool IsBaseEmpty() const noexcept
      {
        if constexpr (Krys::Ranges::has_adl_empty<TRange>)
        {
          return Krys::Ranges::empty(this->TBaseStorage::GetRange());
        }
        else
        {
          return Krys::Ranges::begin(this->TBaseStorage::GetRange())
                 == Krys::Ranges::end(this->TBaseStorage::GetRange());
        }
      }

      constexpr void ReadOne()
      {
        if (this->IsBaseEmpty())
        {
          if constexpr (IsCursorless || (IsSingleValueType && IsInputOrOutput))
          {
            this->TBaseCursorCache::Size = static_cast<TBaseCursorCacheSize>(CursorlessSizeSentinel);
          }
          else
          {
            this->TBaseCursorCache::Size = static_cast<TBaseCursorCacheSize>(this->_cache.size());
            this->TBaseCursorCache::Position = static_cast<TBaseCursorCacheSize>(this->_cache.size());
          }
          return;
        }
        auto &thisInputRange = this->_range();
        auto thisCacheBegin = this->_cache.data();
        KRYS_MAYBE_UNUSED decltype(thisCacheBegin) thisCacheEnd {};
        Span<value_type, MaxValues> cacheView(this->_cache);
        if constexpr (IsInputOrOutput)
        {
          auto result = BasicEncodeOrDecodeOne<Consume::No, TEncodeOrDecode>(
            std::move(thisInputRange), this->encoding(), cacheView, this->ErrorHandler(), this->state());
          thisCacheEnd = std::to_address(Krys::Ranges::begin(result.Output));
          if constexpr (!IsErrorless)
          {
            this->TBaseErrorCache::SetErrors(EncodingError::OK, result.ErrorCode);
          }
          this->TBaseStorage::GetRange() = std::move(result.Input);
        }
        else
        {
          auto result = BasicEncodeOrDecodeOne<Consume::No, TEncodeOrDecode>(
            std::move(thisInputRange), this->encoding(), cacheView, this->ErrorHandler(), this->state());
          thisCacheEnd = std::to_address(Krys::Ranges::begin(result.Output));
          if constexpr (!IsErrorless)
          {
            this->TBaseErrorCache::SetErrors(EncodingError::OK, result.ErrorCode);
          }
          this->TBaseStorage::GetRange() = Impl::UpdateInput<TRange>(std::move(result.Input));
        }
        if constexpr (!IsSingleValueType)
        {
          TBaseCursorCacheSize dataSize = static_cast<TBaseCursorCacheSize>(thisCacheEnd - thisCacheBegin);
          assert(static_cast<std::size_t>(dataSize) <= this->_cache.size()
                 && "size of produced value can never be bigger than the cache");
          this->TBaseCursorCache::Position = static_cast<TBaseCursorCacheSize>(0);
          this->TBaseCursorCache::Size = dataSize;
        }
      }

      constexpr TDerived &_derived() noexcept
      {
        return static_cast<TDerived &>(*this);
      }

      constexpr const TDerived &_derived() const noexcept
      {
        return static_cast<const TDerived &>(*this);
      }

      constexpr auto &_range() noexcept
      {
        return this->TBaseStorage::GetRange();
      }

      constexpr const auto &_range() const noexcept
      {
        return this->TBaseStorage::GetRange();
      }

      std::array<value_type, MaxValues> _cache;
    };
  }
}
