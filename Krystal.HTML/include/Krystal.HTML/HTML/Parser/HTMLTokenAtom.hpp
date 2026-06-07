#pragma once

#include "Krystal.HTML/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.HTML/HTML/Enums/HTMLTagName.hpp"
#include "Krystal.HTML/HTML/Enums/HTMLTokenType.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLToken.hpp"
#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Text/StringConversion.hpp"
#include <cassert>

namespace Krys::HTML
{
  struct IntermediaryAttribute
  {
    QualifiedName Name;
    DOMString Value;
  };

  class HTMLTokenAtom
  {
  private:
    HTMLTokenType _type : BitCount<HTMLTokenType>();
    bool _isSelfClosing : 1 {false};
    DOMStringAtom _name {DOMStringAtom::Null()};
    UniquePtr<DoctypeData> _doctypeData;
    DOMString _comment;
    DOMStringView _data;
    List<IntermediaryAttribute> _attributes;

  public:
    explicit HTMLTokenAtom(HTMLToken &token) noexcept : _type(token.Type())
    {
      const auto &data = token.Data();

      switch (_type)
      {
        case HTMLTokenType::Uninitialized: assert(false); return;
        case HTMLTokenType::EndOfFile:     return;
        case HTMLTokenType::DOCTYPE:
        {
          _name = Krys::Text::ConvertToUTF8(utf32_stringview {data.begin(), data.end()});
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
          _name = Krys::Text::ConvertToUTF8(utf32_stringview {data.begin(), data.end()});
          _isSelfClosing = token.IsSelfClosing();

          for (auto &attr : token.Attributes())
          {
            auto name = Krys::Text::ConvertToUTF8(utf32_stringview {attr.Name.begin(), attr.Name.end()});
            auto value = Krys::Text::ConvertToUTF8(utf32_stringview {attr.Value.begin(), attr.Value.end()});
          }

          return;
        }
      }
    }
  };
}