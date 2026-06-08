#pragma once

#include "Krystal.HTML/DOM/Algorithms/NameValidation.hpp"
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
    DOMStringAtom Name;
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
            _attributes.push_back({name, Krys::Move(value)});
          }

          return;
        }
      }
    }

    KRYS_NODISCARD HTMLTokenType Type() const noexcept
    {
      return _type;
    }

    KRYS_NODISCARD bool IsSelfClosing() const noexcept
    {
      assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag);
      return _isSelfClosing;
    }

    KRYS_NODISCARD DOMStringAtom Name() const noexcept
    {
      assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag
             || _type == HTMLTokenType::DOCTYPE);

      return _name;
    }

    KRYS_NODISCARD List<IntermediaryAttribute> &Attributes() noexcept
    {
      assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag);
      return _attributes;
    }

    KRYS_NODISCARD DOMStringView Comment() const noexcept
    {
      assert(_type == HTMLTokenType::Comment);
      return _comment;
    }

    KRYS_NODISCARD DOMStringView Data() const noexcept
    {
      assert(_type == HTMLTokenType::Character);
      return _data;
    }

    KRYS_NODISCARD bool IsForceQuirks() const noexcept
    {
      assert(_type == HTMLTokenType::DOCTYPE);
      return _doctypeData && _doctypeData->ForceQuirks;
    }

    KRYS_NODISCARD const DoctypeData *DOCTYPEData() const noexcept
    {
      assert(_type == HTMLTokenType::DOCTYPE);
      return _doctypeData.get();
    }
  };
}