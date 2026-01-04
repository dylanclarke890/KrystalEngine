#pragma once

#include "Krystal.Lib/Ranges/ADL.hpp"
#include "Krystal.Lib/Ranges/Range.hpp"
#include "Krystal.Lib/Utils/EBCO.hpp"
#include "Krystal.Lib/Utils/Unwrap.hpp"
#include "Krystal.Text/ErrorHandlerAlwaysReturnsOK.hpp"
#include "Krystal.Text/Impl/EncodingIterator.hpp"
#include "Krystal.Text/Impl/EncodingIteratorStorage.hpp"
#include "Krystal.Text/Impl/EncodingRange.hpp"
#include "Krystal.Text/Impl/SpanReconstruct.hpp"
#include "Krystal.Text/Impl/UpdateInput.hpp"
#include "Krystal.Text/IsIgnorableErrorHandler.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/TranscodeOne.hpp"

namespace Krys
{
  /// @brief A sentinel type that can be used to compare with a TranscodeIterator.
  using TTranscodeSentinel = Impl::TEncodingSentinel;

  /// @brief A transcoding iterator that takes an input of code units and provides an output over the code
  /// units of the desired `TToEncoding` after converting from the `TFromEncoding` in a fashion that will
  /// never produce a EncodingError::InsufficientOutput error.
  /// @tparam TFromEncoding The encoding to read the underlying range of code points as.
  /// @tparam TToEncoding The encoding to read the underlying range of code points as.
  /// @tparam TRange The range of input that will be fed into the TFromEncoding's decode operation.
  /// @tparam TFromErrorHandler The error handler for any decode-step failures.
  /// @tparam TToErrorHandler The error handler for any encode-step failures.
  /// @tparam TFromState The state type to use for the decode operations to intermediate code points.
  /// @tparam TToState The state type to use for the encode operations to intermediate code points.
  /// @remarks This type produces proxies as their reference type, and are only readable, not writable
  /// iterators. The type will also try many different shortcuts for decoding the input and encoding the
  /// intermediates, respectively, including invoking a few customization points for either `DecodeOne` or
  /// `EncodeOne` . It may also call `TranscodeOne` to bypass having to do the round-trip through two
  /// encodings, which an encoding pair that a developer is interested in can use to do the conversion more
  /// quickly. The view presents code units one at a time, regardless of how many code units are output by one
  /// decode operation. This means if, for example, one (1) UTF-16 code unit becomes two (2) UTF-8 code units,
  /// it will present each code unit one at a time. If you are looking to explicitly know each collection of
  /// characters, you will have to use lower-level interfaces.
  template <typename TFromEncoding, typename TToEncoding, typename TRange, typename TFromErrorHandler,
            typename TToErrorHandler, typename TFromState, typename TToState>
  class TranscodeIterator
      : private EBCO<remove_cvref_t<TFromEncoding>, 1>,
        private EBCO<remove_cvref_t<TToEncoding>, 2>,
        private EBCO<remove_cvref_t<TFromErrorHandler>, 3>,
        private EBCO<remove_cvref_t<TToErrorHandler>, 4>,
        private Impl::StateStorage<remove_cvref_t<TFromEncoding>, remove_cvref_t<TFromState>, 0>,
        private Impl::StateStorage<remove_cvref_t<TToEncoding>, remove_cvref_t<TToState>, 1>,
        private Impl::CursorCache<MaxCodeUnits<unwrap_remove_cvref_t<TToEncoding>>,
                                  Krys::Ranges::InputOrOutputRangeExact<unwrap_remove_cvref_t<TRange>>>,
        private Impl::ErrorCache<
          DecodeErrorHandlerAlwaysReturnsOK<unwrap_remove_cvref_t<TFromEncoding>,
                                            unwrap_remove_cvref_t<TFromErrorHandler>> // cf
          && EncodeErrorHandlerAlwaysReturnsOK<unwrap_remove_cvref_t<TToEncoding>,
                                               unwrap_remove_cvref_t<TToErrorHandler>>>,
        private EBCO<Impl::span_reconstruct_t<unwrap_remove_cvref_t<TRange>, TRange>, 5>
  {
  private:
    using TUNonRRange = unwrap_remove_cvref_t<TRange>;
    using TURange = Impl::span_reconstruct_t<TUNonRRange, TRange>;
    using TUFromEncoding = unwrap_remove_cvref_t<TFromEncoding>;
    using TUToEncoding = unwrap_remove_cvref_t<TToEncoding>;
    using TUFromErrorHandler = unwrap_remove_cvref_t<TFromErrorHandler>;
    using TUToErrorHandler = unwrap_remove_cvref_t<TToErrorHandler>;
    using TUFromState = unwrap_remove_cvref_t<TFromState>;
    using TUToState = unwrap_remove_cvref_t<TToState>;
    using TBaseIterator = Krys::Ranges::range_iterator_t<TURange>;
    using TIntermediateCodePoint = code_point_t<TUToEncoding>;
    inline static constexpr std::size_t MaxValues = MaxCodeUnits<TUToEncoding>;
    inline static constexpr bool IsSingleValueType = MaxValues == 1;
    inline static constexpr bool IsInputOrOutput = Krys::Ranges::InputOrOutputRangeExact<TURange>;
    inline static constexpr bool IsCursorless = IsSingleValueType && !IsInputOrOutput;
    inline static constexpr bool IsErrorless =
      DecodeErrorHandlerAlwaysReturnsOK<TUFromEncoding, TUFromErrorHandler>
      && EncodeErrorHandlerAlwaysReturnsOK<TUToEncoding, TUToErrorHandler>;
    using TBaseCursorCache = Impl::CursorCache<MaxValues, IsInputOrOutput>;
    using TBaseCursorCacheSize = typename TBaseCursorCache::TSize;
    using TBaseErrorCache = Impl::ErrorCache<IsErrorless>;
    using TBaseFromEncoding = EBCO<remove_cvref_t<TFromEncoding>, 1>;
    using TBaseToEncoding = EBCO<remove_cvref_t<TToEncoding>, 2>;
    using TBaseFromErrorHandler = EBCO<remove_cvref_t<TFromErrorHandler>, 3>;
    using TBaseToErrorHandler = EBCO<remove_cvref_t<TToErrorHandler>, 4>;
    using TBaseFromState = Impl::StateStorage<remove_cvref_t<TFromEncoding>, remove_cvref_t<TFromState>, 0>;
    using TBaseToState = Impl::StateStorage<remove_cvref_t<TToEncoding>, remove_cvref_t<TToState>, 1>;
    using TRangeBase = EBCO<TURange, 5>;

    inline static constexpr bool IsBackwards =
      Impl::HasEncodeOneBackwardsMember<TUFromEncoding, TURange, TUFromErrorHandler, TUFromState>;

  public:
    /// @brief The underlying range type.
    using range_type = TRange;

    /// @brief The base iterator type.
    using iterator_type = TBaseIterator;

    /// @brief The encoding type used for decoding to intermediate code point storage.
    using from_encoding_type = TFromEncoding;

    /// @brief The encoding type used for encoding to the final code units storage.
    using to_encoding_type = TToEncoding;

    /// @brief The error handler when a decode operation fails.
    using from_error_handler_type = TFromErrorHandler;

    /// @brief The error handler when an encode operation fails.
    using to_error_handler_type = TToErrorHandler;

    /// @brief The state type used for decode operations.
    using from_state_type = remove_cvref_t<TFromState>;

    /// @brief The state type used for encode operations.
    using to_state_type = remove_cvref_t<TToState>;

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
    using value_type = code_unit_t<TToEncoding>;

    /// @brief A pointer type to the value_type.
    using pointer = value_type *;

    /// @brief The value returned from derefencing the iterator.
    ///
    /// @remarks This is a proxy iterator, so the `reference` is a non-reference `value_type.`
    using reference = value_type;

    /// @brief The type returned when two of these pointers are subtracted from one another.
    ///
    /// @remarks It's not a very useful type...
    using difference_type = Krys::Ranges::iterator_difference_type_t<TBaseIterator>;

    /// @brief Default constructs a TranscodeIterator.
    ///
    /// @remarks This can only work if the underlying encodings, error handlers, and states can handle default
    /// construction.
    constexpr TranscodeIterator() noexcept(NoThrowDefaultConstructible<TBaseFromEncoding>
                                           && NoThrowDefaultConstructible<TBaseToEncoding>
                                           && NoThrowDefaultConstructible<TBaseFromErrorHandler>
                                           && NoThrowDefaultConstructible<TBaseToErrorHandler>
                                           && NoThrowConstructible<TBaseFromState, TFromEncoding>
                                           && NoThrowConstructible<TBaseToState, TToEncoding>
                                           && DefaultConstructible<TRangeBase>)
        : TBaseFromEncoding(), TBaseToEncoding(), TBaseFromErrorHandler(), TBaseToErrorHandler(),
          TBaseFromState(this->FromEncoding()), TBaseToState(this->ToEncoding()), TBaseCursorCache(),
          TRangeBase(), _cache()
    {
      this->ReadOne();
    }

    /// @brief Copy constructs a TranscodeIterator.
    constexpr TranscodeIterator(const TranscodeIterator &) = default;

    /// @brief Move constructs a TranscodeIterator.
    constexpr TranscodeIterator(TranscodeIterator &&) = default;

    /// @brief Constructs a TranscodeIterator from the underlying range.
    ///
    /// @param[in] range The input range to wrap and iterate over.
    template <typename TArgRange,
              enable_if_t<!SameType<remove_cvref_t<TArgRange>, TranscodeIterator>> * = nullptr>
    constexpr TranscodeIterator(TArgRange &&range) noexcept(
      noexcept(TranscodeIterator(std::forward<TArgRange>(range), to_encoding_type {})))
        : TranscodeIterator(std::forward<TArgRange>(range), to_encoding_type {})
    {
    }

    /// @brief Constructs a TranscodeIterator from the underlying range.
    ///
    /// @param[in] range The input range to wrap and iterate over.
    /// @param[in] toEncoding The encoding object to call `EncodeOne` or equivalent functionality on.
    constexpr TranscodeIterator(range_type range, to_encoding_type toEncoding)
        : TranscodeIterator(std::move(range), from_encoding_type {}, std::move(toEncoding))
    {
    }

    /// @brief Constructs a TranscodeIterator from the underlying range.
    ///
    /// @param[in] range The input range to wrap and iterate over.
    /// @param[in] fromEncoding The encoding object to call `DecodeOne` or equivalent functionality on.
    /// @param[in] toEncoding The encoding object to call `EncodeOne` or equivalent functionality on.
    constexpr TranscodeIterator(range_type range, from_encoding_type fromEncoding,
                                to_encoding_type toEncoding)
        : TranscodeIterator(std::move(range), std::move(fromEncoding), std::move(toEncoding),
                            from_error_handler_type {}, to_error_handler_type {})
    {
    }

    /// @brief Constructs a TranscodeIterator from the underlying range.
    ///
    /// @param[in] range The input range to wrap and iterate over.
    /// @param[in] fromEncoding The encoding object to call `DecodeOne` or equivalent functionality on.
    /// @param[in] toEncoding The encoding object to call `EncodeOne` or equivalent functionality on.
    /// @param[in] fromErrorHandler The error handler for decode operations to store in this view.
    /// @param[in] toErrorHandler The error handler for encode operations to store in this view.
    constexpr TranscodeIterator(range_type range, from_encoding_type fromEncoding,
                                to_encoding_type toEncoding, from_error_handler_type fromErrorHandler,
                                to_error_handler_type toErrorHandler)
        : TranscodeIterator(std::move(range), std::move(fromEncoding), std::move(toEncoding),
                            std::move(fromErrorHandler), std::move(toErrorHandler), from_state_type {},
                            to_state_type {})
    {
    }

    /// @brief Constructs a TranscodeIterator from the underlying range.
    ///
    /// @param[in] range The input range to wrap and iterate over.
    /// @param[in] fromEncoding The encoding object to call `DecodeOne` or equivalent functionality on.
    /// @param[in] toEncoding The encoding object to call `EncodeOne` or equivalent functionality on.
    /// @param[in] fromErrorHandler The error handler for decode operations to store in this view.
    /// @param[in] toErrorHandler The error handler for encode operations to store in this view.
    /// @param[in] fromState The state to user for the decode operation.
    /// @param[in] toState The state to user for the decode operation.
    constexpr TranscodeIterator(range_type range, from_encoding_type fromEncoding,
                                to_encoding_type toEncoding, from_error_handler_type fromErrorHandler,
                                to_error_handler_type toErrorHandler, from_state_type fromState,
                                to_state_type toState)
        : TBaseFromEncoding(std::move(fromEncoding)), TBaseToEncoding(std::move(toEncoding)),
          TBaseFromErrorHandler(std::move(fromErrorHandler)), TBaseToErrorHandler(std::move(toErrorHandler)),
          TBaseFromState(this->FromEncoding(), std::move(fromState)),
          TBaseToState(this->ToEncoding(), std::move(toState)), TBaseCursorCache(),
          TRangeBase(Krys::Ranges::Reconstruct(std::in_place_type<TUNonRRange>, std::move(range))), _cache()
    {
      this->ReadOne();
    }

    /// @brief Copy assigns- a TranscodeIterator.
    constexpr TranscodeIterator &operator=(const TranscodeIterator &) = default;

    /// @brief Move assigns a TranscodeIterator.
    constexpr TranscodeIterator &operator=(TranscodeIterator &&) = default;

    // observers

    /// @brief The decoding ("from") encoding object.
    ///
    /// @returns A const l-value reference to the encoding object used to construct this iterator.
    constexpr const from_encoding_type &FromEncoding() const
    {
      return this->TBaseFromEncoding::GetValue();
    }

    /// @brief The decoding ("from") encoding object.
    ///
    /// @returns An l-value reference to the encoding object used to construct this iterator.
    constexpr from_encoding_type &FromEncoding()
    {
      return this->TBaseFromEncoding::GetValue();
    }

    /// @brief The encoding ("to") encoding object.
    ///
    /// @returns A const l-value reference to the encoding object used to construct this iterator.
    constexpr const to_encoding_type &ToEncoding() const
    {
      return this->TBaseToEncoding::GetValue();
    }

    /// @brief The encoding ("to") encoding object.
    ///
    /// @returns An l-value reference to the encoding object used to construct this iterator.
    constexpr to_encoding_type &ToEncoding()
    {
      return this->TBaseToEncoding::GetValue();
    }

    /// @brief The decoding ("from") state object.
    constexpr const from_state_type &FromState() const
    {
      return this->TBaseFromState::GetState();
    }

    /// @brief The decoding ("from") state object.
    constexpr from_state_type &FromState()
    {
      return this->TBaseFromState::GetState();
    }

    /// @brief The encoding ("to") state object.
    constexpr const to_state_type &ToState() const
    {
      return this->TBaseToState::GetState();
    }

    /// @brief The encoding ("to") state object.
    constexpr to_state_type &ToState()
    {
      return this->TBaseToState::GetState();
    }

    /// @brief The error handler object.
    constexpr const from_error_handler_type &FromHandler() const
    {
      return this->TBaseFromErrorHandler::GetValue();
    }

    /// @brief The error handler object.
    constexpr from_error_handler_type &FromHandler()
    {
      return this->TBaseFromErrorHandler::GetValue();
    }

    /// @brief The error handler object.
    constexpr const to_error_handler_type &ToHandler() const & noexcept
    {
      return this->TBaseToErrorHandler::GetValue();
    }

    /// @brief The error handler object.
    constexpr to_error_handler_type &ToHandler() & noexcept
    {
      return this->TBaseToErrorHandler::GetValue();
    }

    /// @brief The error handler object.
    constexpr to_error_handler_type &&ToHandler() && noexcept
    {
      return std::move(this->TBaseToErrorHandler::GetValue());
    }

    /// @brief The input range used to construct this object.
    constexpr range_type range() & noexcept(CopyConstructible<range_type>
                                              ? NoThrowCopyConstructible<range_type>
                                              : (NoThrowMoveConstructible<range_type>))
    {
      if constexpr (CopyConstructible<range_type>)
      {
        return this->TRangeBase::GetValue();
      }
      else
      {
        return std::move(this->TRangeBase::GetValue());
      }
    }

    /// @brief The input range used to construct this object.
    constexpr range_type range() const & noexcept(NoThrowCopyConstructible<range_type>)
    {
      return this->TRangeBase::GetValue();
    }

    /// @brief The input range used to construct this object.
    constexpr range_type range() && noexcept(NoThrowMoveConstructible<range_type>)
    {
      return std::move(this->TRangeBase::GetValue());
    }

    /// @brief Returns whether the last read operation had an encoding error or not.
    ///
    /// @returns The EncodingError that occurred. This can be EncodingError::OK for
    /// an operation that went just fine.
    ///
    /// @remarks If the error handler is identified as an error handler that, if given a suitably sized
    /// buffer, will never return an error. This is the case with specific encoding operations with
    /// Handlers::ReplacementHandler, or Handlers::ThrowHandler.
    constexpr EncodingError PivotErrorCode() const noexcept
    {
      if constexpr (IsErrorless)
      {
        return EncodingError::OK;
      }
      else
      {
        return this->TBaseErrorCache::FromError();
      }
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
      if constexpr (IsErrorless)
      {
        return EncodingError::OK;
      }
      else
      {
        return this->TBaseErrorCache::ToError();
      }
    }

    // observers and modifiers: iteration

    /// @brief Copy then increment the iterator.
    ///
    /// @returns A copy of iterator, before incrementing.
    constexpr TranscodeIterator operator++(int)
    {
      TranscodeIterator _copy = *this;
      ++(*this);
      return _copy;
    }

    /// @brief Increment the iterator.
    ///
    /// @returns A reference to *this, after incrementing the iterator.
    constexpr TranscodeIterator &operator++()
    {
      if constexpr (IsSingleValueType)
      {
        this->ReadOne();
      }
      else
      {
        ++this->Position;
        if (this->Position == this->TBaseCursorCache::Size)
        {
          this->ReadOne();
        }
      }
      return *this;
    }

    /// @brief Dereference the iterator.
    ///
    /// @returns A value_type (NOT a reference) of the iterator.
    ///
    /// @remarks This is a proxy iterator, and therefore only returns a value_type object and not a reference
    /// object. Encoding iterators are only readable, not writable.
    constexpr value_type operator*() const
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
    friend constexpr bool operator==(const TranscodeIterator &it, const TTranscodeSentinel &)
    {
      if constexpr (it.IsCursorless || (it.IsInputOrOutput && it.IsSingleValueType))
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
    friend constexpr bool operator==(const TTranscodeSentinel &sentinel, const TranscodeIterator &it)
    {
      return it == sentinel;
    }

    /// @brief Compares whether or not this iterator has truly reached the end.
    friend constexpr bool operator!=(const TranscodeIterator &it, const TTranscodeSentinel &)
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
    friend constexpr bool operator!=(const TTranscodeSentinel &sentinel, const TranscodeIterator &it)
    {
      return !(sentinel == it);
    }

  private:
    constexpr bool IsBaseEmpty() const noexcept
    {
      if constexpr (Krys::Ranges::has_adl_empty<TRange>)
      {
        return Krys::Ranges::empty(this->TRangeBase::GetValue());
      }
      else
      {
        return Krys::Ranges::begin(this->TRangeBase::GetValue())
               == Krys::Ranges::end(this->TRangeBase::GetValue());
      }
    }

    constexpr void ReadOne() noexcept
    {
      if (this->IsBaseEmpty())
      {
        if constexpr (IsCursorless || (IsSingleValueType && IsInputOrOutput))
        {
          this->TBaseCursorCache::Size = static_cast<TBaseCursorCacheSize>(Impl::CursorlessSizeSentinel);
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
      TIntermediateCodePoint intermediateStorage[MaxCodePoints<TUFromEncoding>] {};
      using TPivot = Span<TIntermediateCodePoint, MaxCodePoints<TUFromEncoding>>;
      TPivot pivot(intermediateStorage);
      if constexpr (IsInputOrOutput)
      {
        auto result = Krys::TranscodeOneIntoRaw(std::move(thisInputRange), this->FromEncoding(), cacheView,
                                                this->ToEncoding(), this->FromHandler(), this->ToHandler(),
                                                this->FromState(), this->ToState(), pivot);
        thisCacheEnd = std::to_address(Krys::Ranges::begin(result.Output));
        if constexpr (!IsErrorless)
        {
          this->TBaseErrorCache::SetErrors(pivot.ErrorCode, result.ErrorCode);
        }
        this->TRangeBase::GetValue() = std::move(result.Input);
      }
      else
      {
        auto result = Krys::TranscodeOneIntoRaw(thisInputRange, this->FromEncoding(), cacheView,
                                                this->ToEncoding(), this->FromHandler(), this->ToHandler(),
                                                this->FromState(), this->ToState(), pivot);
        thisCacheEnd = std::to_address(Krys::Ranges::begin(result.Output));
        if constexpr (!IsErrorless)
        {
          this->TBaseErrorCache::SetErrors(result.PivotErrorCode, result.ErrorCode);
        }
        this->TRangeBase::GetValue() = Impl::UpdateInput<TURange>(std::move(result.Input));
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

    constexpr TURange &_range() noexcept
    {
      return this->TRangeBase::GetValue();
    }

    constexpr const TURange &_range() const noexcept
    {
      return this->TRangeBase::GetValue();
    }

    std::array<value_type, MaxValues> _cache;
  };
}
