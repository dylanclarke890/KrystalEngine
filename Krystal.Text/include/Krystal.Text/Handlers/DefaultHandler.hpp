#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Text/Handlers/ReplacementHandler.hpp"

namespace Krys::Text::Handlers
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

  /// @brief An instance of the DefaultHandler type for ease of use.
  constexpr inline DefaultHandler Default = {};

  /// @brief Copies the given error handler if it is a function type and is copy constructible, otherwise
  /// returns a DefaultHandler instance.
  template <typename TErrorHandler>
  constexpr auto CopyOrDefault(TErrorHandler &original)
  {
    if constexpr (IsFunction<remove_cvref_t<TErrorHandler>>)
    {
      if constexpr (CopyConstructible<remove_cvref_t<TErrorHandler>>)
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