#pragma once

#include "Krystal.HTML/HTML/Parser/HTMLToken.hpp"
#include "Krystal.Lib/String/StringAtom.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Text/StringConversion.hpp"
#include <cassert>

namespace Krys::HTML
{
  class HTMLTokenAtom
  {
  public:
    using Type = HTMLToken::Type;

  private:
    Type _type;
    StringAtom _name {StringAtom::Null()};
    bool _isSelfClosing : 1 {false};

  public:
    explicit HTMLTokenAtom(HTMLToken &token) noexcept : _type(token.GetType())
    {
      const auto &data = token.GetDataBuffer();

      switch (_type)
      {
        case Type::Uninitialized: assert(false); return;
        case Type::EndOfFile:     return;
        case Type::StartTag:
        case Type::EndTag:
        {
          _name = StringAtom(Krys::Text::ConvertToUTF8(utf32_stringview {data.begin(), data.end()}));
          _isSelfClosing = token.IsSelfClosing();
          return;
        }
      }
    }
  };
}