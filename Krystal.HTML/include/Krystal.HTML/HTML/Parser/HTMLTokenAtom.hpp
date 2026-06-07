#pragma once

#include "Krystal.HTML/HTML/Enums/HTMLTagName.hpp"
#include "Krystal.HTML/HTML/Enums/HTMLTokenType.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLToken.hpp"
#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Text/StringConversion.hpp"
#include <cassert>

namespace Krys::HTML
{
  class HTMLTokenAtom
  {
  private:
    HTMLTokenType _type : BitCount<HTMLTokenType>();
    bool _isSelfClosing : 1 {false};
    HTMLTagName _tagName : BitCount<HTMLTagName>() {HTMLTagName::Unknown};
    DOMStringAtom _name {DOMStringAtom::Null()};
    UniquePtr<DoctypeData> _doctypeData;
    DOMString _comment;
    DOMStringView _data;

  public:
    explicit HTMLTokenAtom(HTMLToken &token) noexcept : _type(token.GetType())
    {
      const auto &data = token.GetDataBuffer();

      switch (_type)
      {
        case HTMLTokenType::Uninitialized: assert(false); return;
        case HTMLTokenType::EndOfFile:     return;
        case HTMLTokenType::DOCTYPE:
        {
          _name = DOMStringAtom(Krys::Text::ConvertToUTF8(utf32_stringview {data.begin(), data.end()}));
          _doctypeData = token.ReleaseDOCTYPEData();
          return;
        }
        case HTMLTokenType::Comment:
        case HTMLTokenType::Character:
        {
          _data = Krys::Text::ConvertToUTF8(utf32_stringview {data.begin(), data.end()});
          return;
        }
        case HTMLTokenType::StartTag:
        case HTMLTokenType::EndTag:
        {
          auto name = Krys::Text::ConvertToUTF8(utf32_stringview {data.begin(), data.end()});
          _tagName = ParseHTMLTagName(name);

          if (_tagName == HTMLTagName::Unknown)
          {
            _name = DOMStringAtom(name);
          }

          _isSelfClosing = token.IsSelfClosing();
          return;
        }
      }
    }
  };
}