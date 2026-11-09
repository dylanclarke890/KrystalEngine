#pragma once

#include "Krystal.Booey/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/HTML/Parser/Token.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::html
{
  class TokenAtom
  {
    friend class TreeBuilder;

  private:
    TokenType _type : BitCount<TokenType>();
    bool _isSelfClosing : 1 {false};
    bool _selfClosingTagAcknowledged : 1 {false};
    bool _dataReleased : 1 {false};
    dom::DOMStringAtom _name {dom::DOMStringAtom::Null()};
    UniquePtr<DoctypeData> _doctypeData;
    dom::DOMString _data;
    ParsedAttributeList _attributes;

  public:
    explicit TokenAtom(Token &token) noexcept : _type(token.Type())
    {
      const auto &data = token.Data();

      switch (_type)
      {
        case TokenType::Uninitialized:
        {
          krys_debug_assert(false);
          return;
        }
        case TokenType::EndOfFile:
        {
          return;
        }
        case TokenType::DOCTYPE:
        {
          _name = dom::DOMString {data.begin(), data.end()};
          _doctypeData = token.ReleaseDOCTYPEData();

          break;
        }
        case TokenType::Comment:
        case TokenType::Character:
        {
          _data = dom::DOMString {data.begin(), data.end()};

          break;
        }
        case TokenType::ProcessingInstruction:
        {
          _name = dom::DOMString {data.begin(), data.end()};
          const auto &piData = token.ProcessingInstructionData();
          _data = dom::DOMString {piData.begin(), piData.end()};

          break;
        }
        case TokenType::StartTag:
        case TokenType::EndTag:
        {
          _name = dom::DOMString {data.begin(), data.end()};
          _attributes = krys::move(token.Attributes());

          if (_type == TokenType::StartTag)
          {
            _isSelfClosing = token.IsSelfClosing();
          }

          break;
        }
      }

      token.Clear();
    }

    KRYS_NODISCARD TokenType Type() const noexcept
    {
      return _type;
    }

    KRYS_NODISCARD bool IsSelfClosing() const noexcept
    {
      krys_debug_assert(_type == TokenType::StartTag || _type == TokenType::EndTag);
      return _isSelfClosing;
    }

    void AcknowledgeSelfClosingTag() noexcept
    {
      krys_debug_assert(_type == TokenType::StartTag);
      _selfClosingTagAcknowledged = true;
    }

    KRYS_NODISCARD dom::DOMStringAtom Name() const noexcept
    {
      krys_debug_assert(_type == TokenType::StartTag || _type == TokenType::EndTag
                        || _type == TokenType::DOCTYPE);

      return _name;
    }

    KRYS_NODISCARD ParsedAttributeList &Attributes() noexcept
    {
      krys_debug_assert(_type == TokenType::StartTag || _type == TokenType::EndTag);
      return _attributes;
    }

    KRYS_NODISCARD dom::DOMStringView Comment() const noexcept
    {
      krys_debug_assert(_type == TokenType::Comment);
      krys_debug_assert(!_dataReleased);

      return _data;
    }

    KRYS_NODISCARD dom::DOMString ReleaseComment() noexcept
    {
      krys_debug_assert(_type == TokenType::Comment);
      krys_debug_assert(!_dataReleased);

      _dataReleased = true;
      return std::move(_data);
    }

    KRYS_NODISCARD dom::DOMStringAtom ProcessingInstructionTarget() const noexcept
    {
      krys_debug_assert(_type == TokenType::ProcessingInstruction);
      return _name;
    }

    KRYS_NODISCARD dom::DOMStringView ProcessingInstructionData() const noexcept
    {
      krys_debug_assert(_type == TokenType::ProcessingInstruction);
      return _data;
    }

    KRYS_NODISCARD dom::DOMStringView Characters() const noexcept
    {
      krys_debug_assert(_type == TokenType::Character);
      krys_debug_assert(!_dataReleased);

      return _data;
    }

    KRYS_NODISCARD dom::DOMString ReleaseCharacters() noexcept
    {
      krys_debug_assert(_type == TokenType::Character);
      krys_debug_assert(!_dataReleased);

      _dataReleased = true;
      return std::move(_data);
    }

    KRYS_NODISCARD bool IsForceQuirks() const noexcept
    {
      krys_debug_assert(_type == TokenType::DOCTYPE);
      return _doctypeData && _doctypeData->ForceQuirks;
    }

    KRYS_NODISCARD DoctypeData *DOCTYPEData() const noexcept
    {
      krys_debug_assert(_type == TokenType::DOCTYPE);
      return _doctypeData.get();
    }
  };
}