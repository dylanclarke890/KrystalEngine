#pragma once

#include "Krystal.Booey/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.Booey/HTML/Enums/HTMLTokenType.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLToken.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Numeric.hpp"
#include <cassert>

namespace krys::boo::html
{
  class HTMLTokenAtom
  {
    friend class HTMLTreeBuilder;

  private:
    HTMLTokenType _type : BitCount<HTMLTokenType>();
    bool _isSelfClosing : 1 {false};
    bool _selfClosingTagAcknowledged : 1 {false};
    bool _dataReleased : 1 {false};
    dom::DOMStringAtom _name {DOMStringAtom::Null()};
    UniquePtr<DoctypeData> _doctypeData;
    dom::DOMString _data;
    ParsedAttributeList _attributes;

  public:
    explicit HTMLTokenAtom(HTMLToken &token) noexcept : _type(token.Type())
    {
      const auto &data = token.Data();

      switch (_type)
      {
        case HTMLTokenType::Uninitialized:
        {
          assert(false);
          return;
        }
        case HTMLTokenType::EndOfFile:
        {
          return;
        }
        case HTMLTokenType::DOCTYPE:
        {
          _name = dom::DOMString {data.begin(), data.end()};
          _doctypeData = token.ReleaseDOCTYPEData();

          break;
        }
        case HTMLTokenType::Comment:
        case HTMLTokenType::Character:
        {
          _data = dom::DOMString {data.begin(), data.end()};

          break;
        }
        case HTMLTokenType::StartTag:
        case HTMLTokenType::EndTag:
        {
          _name = dom::DOMString {data.begin(), data.end()};
          _attributes = krys::move(token.Attributes());

          if (_type == HTMLTokenType::StartTag)
          {
            _isSelfClosing = token.IsSelfClosing();
          }

          break;
        }
      }

      token.Clear();
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

    void AcknowledgeSelfClosingTag() noexcept
    {
      assert(_type == HTMLTokenType::StartTag);
      _selfClosingTagAcknowledged = true;
    }

    KRYS_NODISCARD dom::DOMStringAtom Name() const noexcept
    {
      assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag
             || _type == HTMLTokenType::DOCTYPE);

      return _name;
    }

    KRYS_NODISCARD ParsedAttributeList &Attributes() noexcept
    {
      assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag);
      return _attributes;
    }

    KRYS_NODISCARD dom::DOMStringView Comment() const noexcept
    {
      assert(_type == HTMLTokenType::Comment);
      assert(!_dataReleased);

      return _data;
    }

    KRYS_NODISCARD dom::DOMString ReleaseComment() noexcept
    {
      assert(_type == HTMLTokenType::Comment);
      assert(!_dataReleased);

      _dataReleased = true;
      return std::move(_data);
    }

    KRYS_NODISCARD dom::DOMStringView Characters() const noexcept
    {
      assert(_type == HTMLTokenType::Character);
      assert(!_dataReleased);

      return _data;
    }

    KRYS_NODISCARD dom::DOMString ReleaseCharacters() noexcept
    {
      assert(_type == HTMLTokenType::Character);
      assert(!_dataReleased);

      _dataReleased = true;
      return std::move(_data);
    }

    KRYS_NODISCARD bool IsForceQuirks() const noexcept
    {
      assert(_type == HTMLTokenType::DOCTYPE);
      return _doctypeData && _doctypeData->ForceQuirks;
    }

    KRYS_NODISCARD DoctypeData *DOCTYPEData() const noexcept
    {
      assert(_type == HTMLTokenType::DOCTYPE);
      return _doctypeData.get();
    }
  };
}