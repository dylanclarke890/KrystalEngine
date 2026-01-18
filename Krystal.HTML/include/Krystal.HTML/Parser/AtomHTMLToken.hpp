#pragma once

#include "Krystal.HTML/Parser/HTMLToken.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include <cassert>

namespace Krys::HTML
{
  class AtomHTMLToken
  {
  public:
    using Type = HTMLToken::Type;

  private:
    Type _type;

  public:
    explicit AtomHTMLToken(HTMLToken &token) noexcept : _type(token.GetType())
    {
      switch (_type)
      {
      }
    }
  };
}