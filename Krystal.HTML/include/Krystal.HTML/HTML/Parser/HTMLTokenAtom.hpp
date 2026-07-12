#pragma once

#include "Krystal.HTML/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.HTML/HTML/Enums/HTMLTokenType.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLToken.hpp"
#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include <cassert>

namespace Krys::HTML
{
  class HTMLTokenAtom
  {
    friend class HTMLTreeBuilder;

  private:
    HTMLTokenType _type : BitCount<HTMLTokenType>();
    bool _isSelfClosing : 1 {false};
    bool _selfClosingTagAcknowledged : 1 {false};
    bool _dataReleased : 1 {false};
    DOMStringAtom _name {DOMStringAtom::Null()};
    UniquePtr<DoctypeData> _doctypeData;
    DOMString _data;
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
          _name = DOMString {data.begin(), data.end()};
          _doctypeData = token.ReleaseDOCTYPEData();

          break;
        }
        case HTMLTokenType::Comment:
        case HTMLTokenType::Character:
        {
          _data = DOMString {data.begin(), data.end()};

          break;
        }
        case HTMLTokenType::StartTag:
        case HTMLTokenType::EndTag:
        {
          _name = DOMString {data.begin(), data.end()};
          _attributes = Krys::Move(token.Attributes());

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

    KRYS_NODISCARD DOMStringAtom Name() const noexcept
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

    KRYS_NODISCARD DOMStringView Comment() const noexcept
    {
      assert(_type == HTMLTokenType::Comment);
      assert(!_dataReleased);

      return _data;
    }

    KRYS_NODISCARD DOMString ReleaseComment() noexcept
    {
      assert(_type == HTMLTokenType::Comment);
      assert(!_dataReleased);

      _dataReleased = true;
      return std::move(_data);
    }

    KRYS_NODISCARD DOMStringView Characters() const noexcept
    {
      assert(_type == HTMLTokenType::Character);
      assert(!_dataReleased);

      return _data;
    }

    KRYS_NODISCARD DOMString ReleaseCharacters() noexcept
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

    KRYS_NODISCARD RawPtr<DoctypeData> DOCTYPEData() const noexcept
    {
      assert(_type == HTMLTokenType::DOCTYPE);
      return _doctypeData.get();
    }
  };
}