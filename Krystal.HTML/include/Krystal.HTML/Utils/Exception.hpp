#pragma once

#include "Krystal.HTML/Utils/ExceptionCode.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Move.hpp"
#include "Krystal.Lib/String/String.hpp"

namespace Krys::HTML
{
  class Exception
  {
  private:
    ExceptionCode _code;
    utf8_string _message;

  public:
    explicit Exception(ExceptionCode code, utf8_string &&message = {}) noexcept
        : _code {code}, _message {Krys::Move(message)}
    {
    }

    KRYS_NODISCARD ExceptionCode Code() const noexcept
    {
      return _code;
    }

    KRYS_NODISCARD const utf8_string &Message() const noexcept
    {
      return _message;
    }
  };
}