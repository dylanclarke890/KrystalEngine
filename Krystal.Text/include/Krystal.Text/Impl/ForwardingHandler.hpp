#pragma once

#include "Krystal.Lib/Utils/EBCO.hpp"
#include <utility>

namespace Krys::Impl
{
  template <typename TEncoding, typename TErrorHandler>
  class ForwardingHandler : private EBCO<TEncoding &, 0>, private EBCO<TErrorHandler &, 1>
  {
  private:
    using EncodingBase = EBCO<TEncoding &, 0>;
    using ErrorHandlerBase = EBCO<TErrorHandler &, 1>;

  public:
    constexpr ForwardingHandler(TEncoding &encoding, TErrorHandler &errorHandler) noexcept
        : EncodingBase(encoding), ErrorHandlerBase(errorHandler)
    {
    }

    template <typename TUnderlyingEncoding, typename TResult, typename TInputProgress,
              typename TOutputProgress>
    constexpr auto operator()(TUnderlyingEncoding &&, TResult &&result, TInputProgress &&inputProgress,
                              TOutputProgress &&outputProgress)
      const & noexcept(noexcept(this->ErrorHandlerBase::GetValue()(
        this->EncodingBase::GetValue(), std::forward<TResult>(result),
        std::forward<TInputProgress>(inputProgress), std::forward<TOutputProgress>(outputProgress))))
    {
      return this->ErrorHandlerBase::GetValue()(this->EncodingBase::GetValue(), std::forward<TResult>(result),
                                                std::forward<TInputProgress>(inputProgress),
                                                std::forward<TOutputProgress>(outputProgress));
    }

    template <typename TUnderlyingEncoding, typename TResult, typename TInputProgress,
              typename TOutputProgress>
    constexpr auto operator()(TUnderlyingEncoding &&, TResult &&result, TInputProgress &&inputProgress,
                              TOutputProgress &&outputProgress)
      & noexcept(noexcept(this->ErrorHandlerBase::GetValue()(this->EncodingBase::GetValue(),
                                                             std::forward<TResult>(result),
                                                             std::forward<TInputProgress>(inputProgress),
                                                             std::forward<TOutputProgress>(outputProgress))))
    {
      return this->ErrorHandlerBase::GetValue()(this->EncodingBase::GetValue(), std::forward<TResult>(result),
                                                std::forward<TInputProgress>(inputProgress),
                                                std::forward<TOutputProgress>(outputProgress));
    }

    template <typename TUnderlyingEncoding, typename TResult, typename TInputProgress,
              typename TOutputProgress>
    constexpr auto operator()(TUnderlyingEncoding &&, TResult &&result, TInputProgress &&inputProgress,
                              TOutputProgress &&outputProgress)
      && noexcept(noexcept(this->ErrorHandlerBase::GetValue()(this->EncodingBase::GetValue(),
                                                              std::forward<TResult>(result),
                                                              std::forward<TInputProgress>(inputProgress),
                                                              std::forward<TOutputProgress>(outputProgress))))
    {
      return this->ErrorHandlerBase::GetValue()(this->EncodingBase::GetValue(), std::forward<TResult>(result),
                                                std::forward<TInputProgress>(inputProgress),
                                                std::forward<TOutputProgress>(outputProgress));
    }
  };
}
