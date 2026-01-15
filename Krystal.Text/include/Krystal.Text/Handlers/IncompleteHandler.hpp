#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/Algorithm.hpp"
#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Lib/Utils/EBCO.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/Decode/DecodeResult.hpp"
#include "Krystal.Text/Encode/EncodeResult.hpp"
#include "Krystal.Text/Handlers/PassHandler.hpp"
#include <array>
#include <cstddef>

namespace Krys::Text::Handlers
{
  /// @brief This handler detects if the error code is an incomplete sequence, and sets the error code to
  /// being okay before returning.
  /// @tparam TEncoding The encoding type which dictates the `CodeUnit` and `CodePoint` buffers to store
  /// in the handler to catch unused input from the last parameter of error handler invocations by the
  /// encoding.
  /// @tparam TErrorHandler An error handler to invoke if the encoding error code is @b NOT an incomplete
  /// sequence.
  /// @remarks This type is often useful in conjunction with an accumulation state or buffer, which can be
  /// very handy for I/O (e.g., Networking) operations.
  template <typename TEncoding, typename TErrorHandler = PassHandler>
  class BasicIncompleteHandler : private EBCO<TErrorHandler>
  {
  private:
    using TErrorHandlerBase = EBCO<TErrorHandler>;
    using TCodeUnit = code_unit_t<TEncoding>;
    using TCodePoint = code_point_t<TEncoding>;

    mutable Array<TCodeUnit, MaxCodeUnits<TEncoding>> _codeUnits;
    mutable std::size_t _codeUnitsSize;
    mutable Array<TCodePoint, MaxCodePoints<TEncoding>> _codePoints;
    mutable std::size_t _codePointsSize;

  public:
    /// @brief The underlying error handler type.
    using ErrorHandler = TErrorHandler;

    /// @brief Constructs a BasicIncompleteHandler with a default-constructed internal error handler.
    constexpr BasicIncompleteHandler() noexcept(NoThrowDefaultConstructible<TErrorHandlerBase>)
        : TErrorHandlerBase(), _codeUnits(), _codeUnitsSize(), _codePoints(), _codePointsSize()
    {
    }

    /// @brief Constructs a BasicIncompleteHandler with the provided internal error handler object.
    /// @param errorHandler The provided error handler object to copy in and use when the error is not an
    /// incomplete error.
    constexpr BasicIncompleteHandler(const TErrorHandler &errorHandler) noexcept(
      NoThrowConstructible<TErrorHandlerBase, const TErrorHandler &>)
        : TErrorHandlerBase(errorHandler), _codeUnits(), _codeUnitsSize(), _codePoints(), _codePointsSize()
    {
    }

    /// @brief Constructs a BasicIncompleteHandler with the provided internal error handler object.
    /// @param errorHandler The provided error handler object to move in and use when the error is not an
    /// incomplete error.
    constexpr BasicIncompleteHandler(TErrorHandler &&errorHandler) noexcept(
      NoThrowConstructible<TErrorHandlerBase, TErrorHandler &&>)
        : TErrorHandlerBase(std::move(errorHandler)), _codeUnits(), _codeUnitsSize(), _codePoints(),
          _codePointsSize()
    {
    }

    /// @brief Returns the Base error handler that is called when a non-incomplete error occurs.
    constexpr TErrorHandler &Base() & noexcept
    {
      return this->TErrorHandlerBase::GetValue();
    }

    /// @brief Returns the Base error handler that is called when a non-incomplete error occurs.
    constexpr const TErrorHandler &Base() const & noexcept
    {
      return this->TErrorHandlerBase::GetValue();
    }

    /// @brief Returns the Base error handler that is called when a non-incomplete error occurs.
    constexpr TErrorHandler &&Base() && noexcept
    {
      return this->TErrorHandlerBase::GetValue();
    }

    /// @brief Checks if the result.ErrorCode is EncodingError::IncompleteSequence, it saves
    /// the values from `progress` and returns. Otherwise, invokes the provided error handler this object
    /// was constructed with.
    /// @param[in] encoding The Encoding that experienced the error.
    /// @param[in] result The current state of the encode operation.
    /// @param[in] inputProgress Any code units or code points that were read but not yet used before the
    /// failure occurred. These will be stored in this handler.
    /// @param[in] outputProgress Any code points or code units that have not yet been written before the
    /// failure occurred. These will be stored in this handler.
    template <typename TResult, typename TInputProgress, typename TOutputProgress>
    constexpr auto operator()(const TEncoding &encoding, TResult result, const TInputProgress &inputProgress,
                              const TOutputProgress &outputProgress) const & // cf
      noexcept(NoThrowInvocable<TErrorHandler, const TEncoding &, TResult &&, const TInputProgress &,
                                const TOutputProgress &>)
    {
      if (result.ErrorCode == EncodingError::IncompleteSequence)
      {
        SaveProgress(inputProgress, outputProgress);
        return result;
      }
      return this->Base()(encoding, std::move(result), inputProgress, outputProgress);
    }

    /// @brief Checks if the result.ErrorCode is EncodingError::IncompleteSequence, it saves
    /// the values from `progress` and returns. Otherwise, invokes the provided error handler this object
    /// was constructed with.
    /// @param[in] encoding The Encoding that experienced the error.
    /// @param[in] result The current state of the encode operation.
    /// @param[in] inputProgress Any code units or code points that were read but not yet used before the
    /// failure occurred. These will be stored in this handler.
    /// @param[in] outputProgress Any code points or code units that have not yet been written before the
    /// failure occurred. These will be stored in this handler.
    template <typename TResult, typename TInputProgress, typename TOutputProgress>
    constexpr auto operator()(const TEncoding &encoding, TResult result, const TInputProgress &inputProgress,
                              const TOutputProgress &outputProgress) & // cf
      noexcept(NoThrowInvocable<TErrorHandler, const TEncoding &, TResult &&, const TInputProgress &,
                                const TOutputProgress &>)
    {
      if (result.ErrorCode == EncodingError::IncompleteSequence)
      {
        SaveProgress(inputProgress, outputProgress);
        return result;
      }
      return this->Base()(encoding, std::move(result), inputProgress, outputProgress);
    }

    /// @brief Checks if the result.ErrorCode is EncodingError::IncompleteSequence, it saves
    /// the values from `progress` and returns. Otherwise, invokes the provided error handler this object
    /// was constructed with.
    /// @param[in] encoding The Encoding that experienced the error.
    /// @param[in] result The current state of the encode operation.
    /// @param[in] inputProgress Any code units or code points that were read but not yet used before the
    /// failure occurred. These will be stored in this handler.
    /// @param[in] outputProgress Any code points or code units that have not yet been written before the
    /// failure occurred. These will be stored in this handler.
    template <typename TResult, typename TInputProgress, typename TOutputProgress>
    constexpr auto operator()(const TEncoding &encoding, TResult result, const TInputProgress &inputProgress,
                              const TOutputProgress &outputProgress) && // cf
      noexcept(NoThrowInvocable<TErrorHandler, const TEncoding &, TResult &&, const TInputProgress &,
                                const TOutputProgress &>)
    {
      if (result.ErrorCode == EncodingError::IncompleteSequence)
      {
        SaveProgress(inputProgress, outputProgress);
        return result;
      }
      return this->Base()(encoding, std::move(result), inputProgress, outputProgress);
    }

    /// @brief Returns the code units from the last incomplete decode operations.
    Span<TCodeUnit> CodeUnits() const noexcept
    {
      return Span<TCodeUnit>(this->_codeUnits.data(), this->_codeUnitsSize);
    }

    /// @brief Returns the code points from the last incomplete encode operations.
    Span<TCodePoint> CodePoints() const noexcept
    {
      return Span<TCodePoint>(this->_codeUnits.data(), this->_codeUnitsSize);
    }

  private:
    template <typename TResult, typename TInputProgress, typename TOutputProgress>
    constexpr void SaveProgress(const TInputProgress &inputProgress, const TOutputProgress &outputProgress)
    {
      // it's incomplete and we are okay with that
      if constexpr (IsSpecializationOf<TResult, DecodeResult>)
      {
        this->_codeUnitsSize = ::Krys::Ranges::size(inputProgress);
        ::Krys::Ranges::Impl::CopyNUnsafe(::Krys::Ranges::cbegin(inputProgress), this->_codeUnitsSize,
                                          this->_codeUnits.data());
        this->_codePointsSize = ::Krys::Ranges::size(outputProgress);
        ::Krys::Ranges::Impl::CopyNUnsafe(::Krys::Ranges::cbegin(outputProgress), this->_codePointsSize,
                                          this->_codePoints.data());
      }
      else
      {
        this->_codeUnitsSize = ::Krys::Ranges::size(outputProgress);
        ::Ranges::Impl::CopyNUnsafe(::Krys::Ranges::cbegin(outputProgress), this->_codeUnitsSize,
                                    this->_codeUnits.data());
        this->_codePointsSize = ::Krys::Ranges::size(inputProgress);
        ::Ranges::Impl::CopyNUnsafe(::Krys::Ranges::cbegin(inputProgress), this->_codePointsSize,
                                    this->_codePoints.data());
      }
    }
  };
}
