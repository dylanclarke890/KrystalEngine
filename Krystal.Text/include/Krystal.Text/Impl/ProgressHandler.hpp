#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/Algorithm.hpp"
#include "Krystal.Lib/Utils/EBCO.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/DecodeResult.hpp"
#include "Krystal.Text/EncodeResult.hpp"
#include <array>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace Krys
{
  namespace Impl
  {
    template <bool, typename>
    class ProgressHandler;

    template <bool, typename, typename>
    class ForwardingProgressHandler;

    template <typename THandler>
    class IsProgressHandlerSpecialization : public std::false_type
    {
    };

    template <bool TAssumeValid, typename TEncoding>
    class IsProgressHandlerSpecialization<ProgressHandler<TAssumeValid, TEncoding>> : public std::true_type
    {
    };

    template <typename THandler>
    concept IsProgressHandler = IsProgressHandlerSpecialization<THandler>::value;

    template <typename THandler>
    class IsForwardingProgressHandlerSpecialization : public std::false_type
    {
    };

    template <bool TAssumeValid, typename TErrorHandler, typename TEncoding>
    class IsForwardingProgressHandlerSpecialization<
      ForwardingProgressHandler<TAssumeValid, TErrorHandler, TEncoding>> : public std::true_type
    {
    };

    template <typename THandler>
    concept IsForwardingProgressHandler = IsForwardingProgressHandlerSpecialization<THandler>::value;

    template <bool TAssumeValid, typename _DesiredEncoding>
    class ProgressHandler
    {
    private:
      using TCodePoint = code_point_t<_DesiredEncoding>;
      using TCodeUnit = code_unit_t<_DesiredEncoding>;

    public:
      using assumeTValid = IntegralConstant<bool, TAssumeValid>;

      constexpr ProgressHandler() noexcept : _codePoints(), _codePointsSize(), _codeUnits(), _codeUnitsSize()
      {
      }

      template <typename TIgnored>
      requires(!SameType<ProgressHandler, remove_cvref_t<TIgnored>>)
      constexpr ProgressHandler(KRYS_MAYBE_UNUSED TIgnored &&) noexcept
          : _codePoints(), _codePointsSize(), _codeUnits(), _codeUnitsSize()
      {
      }

      constexpr Span<const TCodePoint> ConstCodePointsProgress() const noexcept
      {
        return Span<const TCodePoint>(this->_codePoints.data(), this->_codePointsSize);
      }

      constexpr Span<const TCodeUnit> ConstCodeUnitsProgress() const noexcept
      {
        return Span<const TCodeUnit>(this->_codeUnits.data(), this->_codeUnitsSize);
      }

      constexpr Span<const TCodePoint> CodePointsProgress() const noexcept
      {
        return Span<const TCodePoint>(this->_codePoints.data(), this->_codePointsSize);
      }

      constexpr Span<const TCodeUnit> CodeUnitsProgress() const noexcept
      {
        return Span<const TCodeUnit>(this->_codeUnits.data(), this->_codeUnitsSize);
      }

      constexpr Span<TCodePoint> CodePointsProgress() noexcept
      {
        return Span<TCodePoint>(this->_codePoints.data(), this->_codePointsSize);
      }

      constexpr Span<TCodeUnit> CodeUnitsProgress() noexcept
      {
        return Span<TCodeUnit>(this->_codeUnits.data(), this->_codeUnitsSize);
      }

      constexpr std::size_t CodePointsProgressSize() const noexcept
      {
        return this->_codePointsSize;
      }

      constexpr std::size_t CodeUnitsProgressSize() const noexcept
      {
        return this->_codeUnitsSize;
      }

      template <typename TEncoding, typename TResult, typename TInputProgress, typename TOutputProgress>
      constexpr auto operator()(const TEncoding &, TResult result, const TInputProgress &inputProgress,
                                const TOutputProgress &outputProgress) noexcept
      {
        if constexpr (IsSpecializationOf<remove_cvref_t<TResult>, DecodeResult>)
        {
          this->_codeUnitsSize = Krys::Ranges::size(inputProgress);
          Krys::Ranges::Impl::CopyNUnsafe(Krys::Ranges::cbegin(inputProgress), this->_codeUnitsSize,
                                          this->_codeUnits.data());
          this->_codePointsSize = Krys::Ranges::size(outputProgress);
          Krys::Ranges::Impl::CopyNUnsafe(Krys::Ranges::cbegin(outputProgress), this->_codePointsSize,
                                          this->_codePoints.data());
        }
        else
        {
          this->_codePointsSize = Krys::Ranges::size(inputProgress);
          Krys::Ranges::Impl::CopyNUnsafe(Krys::Ranges::cbegin(inputProgress), this->_codePointsSize,
                                          this->_codePoints.data());
          this->_codeUnitsSize = Krys::Ranges::size(outputProgress);
          Krys::Ranges::Impl::CopyNUnsafe(Krys::Ranges::cbegin(outputProgress), this->_codeUnitsSize,
                                          this->_codeUnits.data());
        }
        return result;
      }

      constexpr void clear() noexcept
      {
        this->_codePointsSize = 0;
        this->_codeUnitsSize = 0;
      }

    private:
      std::array<TCodePoint, MaxCodePoints<_DesiredEncoding>> _codePoints;
      std::size_t _codePointsSize;
      std::array<TCodeUnit, MaxCodeUnits<_DesiredEncoding>> _codeUnits;
      std::size_t _codeUnitsSize;
    };

    template <bool TAssumeValid, typename TErrorHandler, typename TEncoding>
    class ForwardingProgressHandler : private EBCO<TEncoding &, 0>, private EBCO<TErrorHandler &, 1>
    {
    private:
      using TCodePoint = code_point_t<remove_cvref_t<TEncoding>>;
      using TCodeUnit = code_unit_t<remove_cvref_t<TEncoding>>;
      using TEncodingBase = EBCO<TEncoding &, 0>;
      using TErrorHandlerBase = EBCO<TErrorHandler &, 1>;
      inline static constexpr bool TIsProgressHandler =
        IsProgressHandler<remove_cvref_t<TErrorHandler>>
        || IsForwardingProgressHandler<remove_cvref_t<TErrorHandler>>;

    public:
      using assumeTValid = IntegralConstant<bool, TAssumeValid>;

      constexpr ForwardingProgressHandler(TEncoding &encoding, TErrorHandler &errorHandler) noexcept
          : TEncodingBase(encoding), TErrorHandlerBase(errorHandler), _codePoints(), _codePointsSize(),
            _codeUnits(), _codeUnitsSize()
      {
      }

      constexpr Span<const TCodePoint> CodePointsProgress() const noexcept
      {
        if constexpr (TIsProgressHandler)
        {
          return this->TErrorHandlerBase::GetValue().CodePointsProgress();
        }
        else
        {
          return Span<const TCodePoint>(this->_codePoints.data(), this->_codePointsSize);
        }
      }

      constexpr Span<const TCodeUnit> CodeUnitsProgress() const noexcept
      {
        if constexpr (TIsProgressHandler)
        {
          return this->TErrorHandlerBase::GetValue().CodeUnitsProgress();
        }
        else
        {
          return Span<const TCodeUnit>(this->_codeUnits.data(), this->_codeUnitsSize);
        }
      }

      constexpr Span<TCodePoint> CodePointsProgress() noexcept
      {
        if constexpr (TIsProgressHandler)
        {
          return this->TErrorHandlerBase::GetValue().CodePointsProgress();
        }
        else
        {
          return Span<TCodePoint>(this->_codePoints.data(), this->_codePointsSize);
        }
      }

      constexpr Span<TCodeUnit> CodeUnitsProgress() noexcept
      {
        if constexpr (TIsProgressHandler)
        {
          return this->TErrorHandlerBase::GetValue().CodeUnitsProgress();
        }
        else
        {
          return Span<TCodeUnit>(this->_codeUnits.data(), this->_codeUnitsSize);
        }
      }

      constexpr std::size_t CodePointsProgressSize() const noexcept
      {
        if constexpr (TIsProgressHandler)
        {
          return this->TErrorHandlerBase::GetValue().CodePointsProgressSize();
        }
        else
        {
          return this->_codePointsSize;
        }
      }

      constexpr std::size_t CodeUnitsProgressSize() const noexcept
      {
        if constexpr (TIsProgressHandler)
        {
          return this->TErrorHandlerBase::GetValue().CodeUnitsProgressSize();
        }
        else
        {
          return this->_codeUnitsSize;
        }
      }

      template <typename TSomeEncoding, typename TResult, typename TInputProgress, typename TOutputProgress>
      constexpr auto operator()(const TSomeEncoding &, TResult &&result, TInputProgress &&inputProgress,
                                TOutputProgress &&outputProgress) & // cf
        noexcept(noexcept(this->TErrorHandlerBase::GetValue()(this->TEncodingBase::GetValue(),
                                                              std::forward<TResult>(result),
                                                              std::forward<TInputProgress>(inputProgress),
                                                              std::forward<TOutputProgress>(outputProgress))))
      {
        if constexpr (TIsProgressHandler)
        {
          ForwardingProgressHandler &nonConstSelf = *this;
          if constexpr (IsSpecializationOf<remove_cvref_t<TResult>, DecodeResult>)
          {
            nonConstSelf._codeUnitsSize = Krys::Ranges::size(inputProgress);
            Krys::Ranges::Impl::CopyNUnsafe(Krys::Ranges::cbegin(inputProgress), nonConstSelf._codeUnitsSize,
                                            nonConstSelf._codeUnits.data());
            nonConstSelf._codePointsSize = Krys::Ranges::size(outputProgress);
            Krys::Ranges::Impl::CopyNUnsafe(Krys::Ranges::cbegin(outputProgress),
                                            nonConstSelf._codePointsSize, nonConstSelf._codePoints.data());
          }
          else
          {
            nonConstSelf._codePointsSize = Krys::Ranges::size(inputProgress);
            Krys::Ranges::Impl::CopyNUnsafe(Krys::Ranges::cbegin(inputProgress), nonConstSelf._codePointsSize,
                                            nonConstSelf._codePoints.data());
            nonConstSelf._codeUnitsSize = Krys::Ranges::size(outputProgress);
            Krys::Ranges::Impl::CopyNUnsafe(Krys::Ranges::cbegin(outputProgress), nonConstSelf._codeUnitsSize,
                                            nonConstSelf._codeUnits.data());
          }
        }
        return this->TErrorHandlerBase::GetValue()(
          this->TEncodingBase::GetValue(), std::forward<TResult>(result),
          std::forward<TInputProgress>(inputProgress), std::forward<TOutputProgress>(outputProgress));
      }

      template <typename TSomeEncoding, typename TResult, typename TInputProgress, typename TOutputProgress>
      constexpr auto operator()(const TSomeEncoding &, TResult &&result, TInputProgress &&inputProgress,
                                TOutputProgress &&outputProgress) const & // cf
        noexcept(noexcept(this->TErrorHandlerBase::GetValue()(this->TEncodingBase::GetValue(),
                                                              std::forward<TResult>(result),
                                                              std::forward<TInputProgress>(inputProgress),
                                                              std::forward<TOutputProgress>(outputProgress))))
      {
        if constexpr (TIsProgressHandler)
        {
          ForwardingProgressHandler &nonConstSelf = const_cast<ForwardingProgressHandler &>(*this);
          if constexpr (IsSpecializationOf<remove_cvref_t<TResult>, DecodeResult>)
          {
            nonConstSelf._codeUnitsSize = Krys::Ranges::size(inputProgress);
            Krys::Ranges::Impl::CopyNUnsafe(Krys::Ranges::cbegin(inputProgress), nonConstSelf._codeUnitsSize,
                                            nonConstSelf._codeUnits.data());
            nonConstSelf._codePointsSize = Krys::Ranges::size(outputProgress);
            Krys::Ranges::Impl::CopyNUnsafe(Krys::Ranges::cbegin(outputProgress),
                                            nonConstSelf._codePointsSize, nonConstSelf._codePoints.data());
          }
          else
          {
            nonConstSelf._codePointsSize = Krys::Ranges::size(inputProgress);
            Krys::Ranges::Impl::CopyNUnsafe(Krys::Ranges::cbegin(inputProgress), nonConstSelf._codePointsSize,
                                            nonConstSelf._codePoints.data());
            nonConstSelf._codeUnitsSize = Krys::Ranges::size(outputProgress);
            Krys::Ranges::Impl::CopyNUnsafe(Krys::Ranges::cbegin(outputProgress), nonConstSelf._codeUnitsSize,
                                            nonConstSelf._codeUnits.data());
          }
        }
        return this->TErrorHandlerBase::GetValue()(
          this->TEncodingBase::GetValue(), std::forward<TResult>(result),
          std::forward<TInputProgress>(inputProgress), std::forward<TOutputProgress>(outputProgress));
      }

      template <typename TSomeEncoding, typename TResult, typename TInputProgress, typename TOutputProgress>
      constexpr auto operator()(const TSomeEncoding &, TResult &&result, TInputProgress &&inputProgress,
                                TOutputProgress &&outputProgress) && // cf
        noexcept(noexcept(this->TErrorHandlerBase::GetValue()(this->TEncodingBase::GetValue(),
                                                              std::forward<TResult>(result),
                                                              std::forward<TInputProgress>(inputProgress),
                                                              std::forward<TOutputProgress>(outputProgress))))
      {
        if constexpr (TIsProgressHandler)
        {
          ForwardingProgressHandler &nonConstSelf = *this;
          if constexpr (IsSpecializationOf<remove_cvref_t<TResult>, DecodeResult>)
          {
            nonConstSelf._codeUnitsSize = Krys::Ranges::size(inputProgress);
            Krys::Ranges::Impl::CopyNUnsafe(Krys::Ranges::cbegin(inputProgress), nonConstSelf._codeUnitsSize,
                                            nonConstSelf._codeUnits.data());
            nonConstSelf._codePointsSize = Krys::Ranges::size(outputProgress);
            Krys::Ranges::Impl::CopyNUnsafe(Krys::Ranges::cbegin(outputProgress),
                                            nonConstSelf._codePointsSize, nonConstSelf._codePoints.data());
          }
          else
          {
            nonConstSelf._codePointsSize = Krys::Ranges::size(inputProgress);
            Krys::Ranges::Impl::CopyNUnsafe(Krys::Ranges::cbegin(inputProgress), nonConstSelf._codePointsSize,
                                            nonConstSelf._codePoints.data());
            nonConstSelf._codeUnitsSize = Krys::Ranges::size(outputProgress);
            Krys::Ranges::Impl::CopyNUnsafe(Krys::Ranges::cbegin(outputProgress), nonConstSelf._codeUnitsSize,
                                            nonConstSelf._codeUnits.data());
          }
        }
        return this->TErrorHandlerBase::GetValue()(
          this->TEncodingBase::GetValue(), std::forward<TResult>(result),
          std::forward<TInputProgress>(inputProgress), std::forward<TOutputProgress>(outputProgress));
      }

      constexpr void clear() noexcept
      {
        this->_codePointsSize = 0;
        this->_codeUnitsSize = 0;
      }

    private:
      std::array<TCodePoint, MaxCodePoints<remove_cvref_t<TEncoding>>> _codePoints;
      std::size_t _codePointsSize;
      std::array<TCodeUnit, MaxCodeUnits<remove_cvref_t<TEncoding>>> _codeUnits;
      std::size_t _codeUnitsSize;
    };
  }
}
