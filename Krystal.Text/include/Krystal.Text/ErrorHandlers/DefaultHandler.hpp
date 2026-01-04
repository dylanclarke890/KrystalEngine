#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Text/ErrorHandlers/AssumeValidHandler.hpp"
#include "Krystal.Text/ErrorHandlers/ReplacementHandler.hpp"
#include "Krystal.Text/ErrorHandlers/ThrowHandler.hpp"
#include <climits>

namespace Krys::Handlers
{
  /// @brief The default error handler for the entire library. Can be configured to use different strategies
  /// at build time. Without configuration, it defaults to the ReplacementHandler.
  class DefaultHandler : private ReplacementHandler
  {
  private:
    using TErrorHandlerBase = ReplacementHandler;

  public:
    ///@brief The underlying error handler type.
    using ErrorHandler = TErrorHandlerBase;

    using TErrorHandlerBase::TErrorHandlerBase;

    using TErrorHandlerBase::operator();
  };

  /// @brief The default error handler for the entire library. Can be configured to use different strategies
  /// at build time. Without configuration, it defaults to the ReplacementHandler.
  class UncheckedDefaultHandler : private DefaultHandler
  {
  private:
    using TErrorHandlerBase = DefaultHandler;

  public:
    ///@brief The underlying error handler type.
    using ErrorHandler = TErrorHandlerBase;

    using TErrorHandlerBase::TErrorHandlerBase;

    using TErrorHandlerBase::operator();
  };

  /// @brief An instance of the DefaultHandler type for ease of use.
  constexpr inline DefaultHandler Default = {};

  /// @brief An instance of the DefaultHandler type for ease of use.
  constexpr inline UncheckedDefaultHandler UncheckedDefault = {};

  namespace Impl
  {
    template <typename TErrorHandler>
    constexpr auto DuplicateOrBeCareless(TErrorHandler &original)
    {
      using TUErrorHandler = remove_cvref_t<TErrorHandler>;
      if constexpr (IsFunction<TUErrorHandler>)
      {
        if constexpr (CopyConstructible<TUErrorHandler>)
        {
          return original;
        }
        else
        {
          return DefaultHandler {};
        }
      }
      else
      {
        return DefaultHandler {};
      }
    }
  }
}