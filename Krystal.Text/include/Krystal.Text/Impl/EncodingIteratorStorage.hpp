#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/Range.hpp"
#include "Krystal.Lib/Utils/EBCO.hpp"
#include "Krystal.Lib/Utils/ToUnderlying.hpp"
#include "Krystal.Lib/Utils/Unwrap.hpp"
#include "Krystal.Text/EncodingError.hpp"
#include "Krystal.Text/Impl/SpanReconstruct.hpp"
#include "Krystal.Text/State.hpp"
#include <climits>
#include <cstddef>

namespace Krys
{
  namespace Impl
  {
    // TODO: move this to Krys::Maths at some point
    constexpr uint_least64_t ipow_impl(uint_least64_t base, uint_least64_t exp, uint_least64_t result)
    {
      return exp < 1u ? result : ipow_impl(base * base, exp / 2, (exp % 2) ? result * base : result);
    }

    constexpr uint_least64_t ipow(uint_least64_t base, uint_least64_t exp)
    {
      return ipow_impl(base, exp, 1);
    }

    inline constexpr std::size_t CursorlessSizeSentinel = 1;

    template <typename TEncoding, typename TEncodingState, std::size_t TId = 0>
    class StateStorage : private EBCO<unwrap_remove_cvref_t<TEncodingState>, TId>
    {
    private:
      using TUEncoding = unwrap_remove_cvref_t<TEncoding>;
      using TUEncodingState = unwrap_remove_cvref_t<TEncodingState>;
      using TStateBase = EBCO<unwrap_remove_cvref_t<TEncodingState>, TId>;

    public:
      constexpr StateStorage() = default;
      constexpr StateStorage(const StateStorage &) = default;
      constexpr StateStorage(StateStorage &&) = default;
      constexpr StateStorage &operator=(const StateStorage &) = default;
      constexpr StateStorage &operator=(StateStorage &&) = default;

      template <typename TArgEncoding = TUEncoding>
      requires(!IsStateIndependant<remove_cvref_t<TArgEncoding>, TUEncodingState>
               && !SameType<remove_cvref_t<TArgEncoding>, StateStorage>)
      constexpr StateStorage(TArgEncoding &encoding) noexcept(NoThrowConstructible<TStateBase, TEncoding &>)
          : TStateBase(std::forward<TArgEncoding>(encoding))
      {
      }
      template <typename TArgEncoding = TUEncoding>
      requires(IsStateIndependant<remove_cvref_t<TArgEncoding>, TUEncodingState>
               && !SameType<remove_cvref_t<TArgEncoding>, StateStorage>)
      constexpr StateStorage(TArgEncoding &) noexcept(NoThrowDefaultConstructible<TStateBase>) : TStateBase()
      {
      }
      constexpr StateStorage(TEncoding &, const TUEncodingState &state) noexcept(
        NoThrowConstructible<TStateBase, const TUEncodingState &>)
          : TStateBase(state)
      {
      }
      constexpr StateStorage(TEncoding &, TUEncodingState &&state) noexcept(
        NoThrowConstructible<TStateBase, TUEncodingState &&>)
          : TStateBase(std::move(state))
      {
      }

      constexpr add_lvalue_ref_t<TUEncodingState> GetState() noexcept
      {
        return this->TStateBase::GetValue();
      }

      constexpr add_const_t<add_lvalue_ref_t<TUEncodingState>> GetState() const noexcept
      {
        return this->TStateBase::GetValue();
      }
    };

    template <typename TEncoding, typename TRange, typename TErrorHandler, typename TState>
    class IteratorStorage : private EBCO<remove_cvref_t<TEncoding>, 0>,
                            private EBCO<remove_cvref_t<TErrorHandler>, 1>,
                            private StateStorage<unwrap_remove_cvref_t<TEncoding>, remove_cvref_t<TState>, 2>,
                            private EBCO<Impl::span_reconstruct_t<unwrap_remove_cvref_t<TRange>, TRange>, 3>
    {
    private:
      using TURange = unwrap_remove_cvref_t<TRange>;
      using TUEncoding = unwrap_remove_cvref_t<TEncoding>;
      using TUErrorHandler = unwrap_remove_cvref_t<TErrorHandler>;
      using TUState = unwrap_remove_cvref_t<TState>;
      using TEncodingBase = EBCO<remove_cvref_t<TEncoding>, 0>;
      using TErrorHandlerBase = EBCO<remove_cvref_t<TErrorHandler>, 1>;
      using TStateBase = StateStorage<unwrap_remove_cvref_t<TEncoding>, remove_cvref_t<TState>, 2>;
      using TRangeBase = EBCO<Impl::span_reconstruct_t<TURange, TRange>, 3>;

    public:
      using range_type = TRange;
      using encoding_type = TEncoding;
      using state_type = TState;
      using error_handler_type = TErrorHandler;

      constexpr IteratorStorage() = default;
      constexpr IteratorStorage(const IteratorStorage &) = default;
      constexpr IteratorStorage(IteratorStorage &&) = default;
      constexpr IteratorStorage &operator=(const IteratorStorage &) = default;
      constexpr IteratorStorage &operator=(IteratorStorage &&) = default;

      template <typename _ArgRange,
                enable_if_t<!SameType<remove_cvref_t<_ArgRange>, IteratorStorage>> * = nullptr>
      constexpr IteratorStorage(_ArgRange &&range) noexcept(noexcept(IteratorStorage(
        std::declval<range_type>(), std::declval<encoding_type>(), std::declval<error_handler_type>())))
          : IteratorStorage(std::forward<_ArgRange>(range), encoding_type {}, error_handler_type {})
      {
      }

      constexpr IteratorStorage(range_type range, encoding_type encoding) noexcept(noexcept(IteratorStorage(
        std::declval<range_type>(), std::declval<encoding_type>(), std::declval<error_handler_type>())))
          : IteratorStorage(std::move(range), std::move(encoding), error_handler_type {})
      {
      }

      constexpr IteratorStorage(range_type range, error_handler_type errorHandler) noexcept(
        noexcept(IteratorStorage(std::declval<range_type>(), std::declval<encoding_type>(),
                                 std::declval<error_handler_type>())))
          : IteratorStorage(std::move(range), encoding_type {}, std::move(errorHandler))
      {
      }

      constexpr IteratorStorage(range_type range, encoding_type encoding,
                                error_handler_type errorHandler) // cf
        noexcept(NoThrowMoveConstructible<encoding_type>         // cf
                 && NoThrowMoveConstructible<error_handler_type> // cf
                 && NoThrowMoveConstructible<range_type>         // cf
                 && noexcept(TStateBase(std::declval<encoding_type &>())))
          : TEncodingBase(std::move(encoding)), TErrorHandlerBase(std::move(errorHandler)),
            TStateBase(this->GetEncoding()), TRangeBase(Impl::SpanReconstruct<TURange>(std::move(range)))
      {
      }

      constexpr IteratorStorage(range_type range, encoding_type encoding, error_handler_type errorHandler,
                                state_type state) noexcept( // cf
        NoThrowMoveConstructible<encoding_type>             // cf
        && NoThrowMoveConstructible<error_handler_type>     // cf
        && NoThrowMoveConstructible<range_type>             // cf
        && noexcept(TStateBase(std::declval<encoding_type &>(), std::declval<state_type>())))
          : TEncodingBase(std::move(encoding)), TErrorHandlerBase(std::move(errorHandler)),
            TStateBase(this->GetEncoding(), std::move(state)),
            TRangeBase(Impl::SpanReconstruct<TURange>(std::move(range)))
      {
      }

      using TStateBase::GetState;

      const encoding_type &GetEncoding() const & noexcept
      {
        return this->TEncodingBase::GetValue();
      }

      encoding_type &GetEncoding() & noexcept
      {
        return this->TEncodingBase::GetValue();
      }

      encoding_type &&GetEncoding() && noexcept
      {
        return std::move(this->TEncodingBase::GetValue());
      }

      const TUErrorHandler &GetErrorHandler() const & noexcept
      {
        return this->TErrorHandlerBase::GetValue();
      }

      TUErrorHandler &GetErrorHandler() & noexcept
      {
        return this->TErrorHandlerBase::GetValue();
      }

      TUErrorHandler &&GetErrorHandler() && noexcept
      {
        return std::move(this->TErrorHandlerBase::GetValue());
      }

      const auto &GetRange() const & noexcept
      {
        return this->TRangeBase::GetValue();
      }

      auto &GetRange() & noexcept
      {
        return this->TRangeBase::GetValue();
      }

      auto &&GetRange() && noexcept
      {
        return std::move(this->TRangeBase::GetValue());
      }
    };

    template <typename TRef>
    class RefStorage : private EBCO<unwrap_t<TRef>, 0>,
                       private StateStorage<remove_cvref_t<unwrap_t<typename TRef::encoding_type>>,
                                            remove_cvref_t<unwrap_t<typename TRef::state_type>>, 1>
    {
    private:
      using TEncoding = typename TRef::encoding_type;
      using TRange = typename TRef::range_type;
      using TErrorHandler = typename TRef::error_handler_type;
      using TState = typename TRef::state_type;
      using TUEncoding = unwrap_remove_cvref_t<TEncoding>;
      using TURange = unwrap_remove_cvref_t<TRange>;
      using TUErrorHandler = unwrap_remove_cvref_t<TErrorHandler>;
      using TUState = unwrap_remove_cvref_t<TState>;
      using TRefBase = EBCO<unwrap_t<TRef>, 0>;
      using TStateBase = StateStorage<TUEncoding, TUState, 1>;

      unwrap_t<TRef> GetRef() noexcept
      {
        return this->TRefBase::GetValue();
      }

      const unwrap_t<TRef> GetRef() const noexcept
      {
        return this->TRefBase::GetValue();
      }

    public:
      using range_type = TRange;
      using encoding_type = TEncoding;
      using state_type = TState;
      using error_handler_type = TErrorHandler;

      constexpr RefStorage(const RefStorage &) = default;
      constexpr RefStorage(RefStorage &&) = default;
      constexpr RefStorage &operator=(const RefStorage &) = default;
      constexpr RefStorage &operator=(RefStorage &&) = default;

      template <typename TArg>
      requires(!SameType<remove_cvref_t<TArg>, RefStorage>)
      constexpr RefStorage(TArg &&arg) noexcept(NoThrowConstructible<TRefBase, TArg>)
          : TRefBase(std::forward<TArg>(arg)), TStateBase(this->GetEncoding())
      {
      }

      using TStateBase::GetState;

      const encoding_type &GetEncoding() const & noexcept
      {
        return this->GetRef()->encoding();
      }

      encoding_type &GetEncoding() & noexcept
      {
        return this->GetRef()->encoding();
      }

      encoding_type &&GetEncoding() && noexcept
      {
        return this->GetRef()->encoding();
      }

      const TUErrorHandler &GetErrorHandler() const & noexcept
      {
        return this->GetRef()->error_handler();
      }

      TUErrorHandler &GetErrorHandler() & noexcept
      {
        return this->GetRef()->error_handler();
      }

      TUErrorHandler &&GetErrorHandler() && noexcept
      {
        return this->GetRef()->error_handler();
      }

      const TURange &GetRange() const & noexcept
      {
        return this->GetRef()->range();
      }

      TURange &GetRange() & noexcept
      {
        return this->GetRef()->range();
      }

      TURange &&GetRange() && noexcept
      {
        return this->GetRef()->range();
      }
    };

    template <std::size_t MaxN, bool TIsInputOrOutputIterator>
    class CursorCache
    {
    public:
      // clang-format off
			using TSize = 
       conditional_t<(MaxN <= UCHAR_MAX), uchar,
			  conditional_t<(MaxN <= USHRT_MAX), ushort, 
			   conditional_t<(MaxN <= UINT_MAX), uint,
			    conditional_t<(MaxN <= ULONG_MAX), ulong,
			     conditional_t<(MaxN <= ULLONG_MAX), ullong, std::size_t
      >>>>>;
      // clang-format on

      TSize Size = static_cast<TSize>(0);
      TSize Position = static_cast<TSize>(0);
    };

    template <>
    class CursorCache<1, true>
    {
    public:
      using TSize = uchar;

      TSize Size = static_cast<TSize>(0);
    };

    template <>
    class CursorCache<1, false>
    {
    public:
      using TSize = uchar;

      TSize Size = static_cast<TSize>(0);
    };

    template <bool>
    class ErrorCache
    {
    public:
      uchar ErrorCode = static_cast<uchar>(Krys::ToUnderlying(EncodingError::OK)
                                           | (Krys::ToUnderlying(EncodingError::OK) << 2));

      constexpr EncodingError FromError() const noexcept
      {
        return static_cast<EncodingError>((ErrorCode & (0x02 << 0)) >> 0);
      }

      constexpr EncodingError ToError() const noexcept
      {
        return static_cast<EncodingError>((ErrorCode & (0x02 << 2)) >> 2);
      }

      constexpr void SetErrors(EncodingError fromError, EncodingError toError) noexcept
      {
        this->ErrorCode = 0;
        this->ErrorCode |= (::Krys::ToUnderlying(fromError) << 0);
        this->ErrorCode |= (::Krys::ToUnderlying(toError) << 2);
      }
    };

    template <>
    class ErrorCache<true>
    {
      constexpr EncodingError FromError() const noexcept
      {
        return EncodingError::OK;
      }

      constexpr EncodingError ToError() const noexcept
      {
        return EncodingError::OK;
      }
    };
  }
}
