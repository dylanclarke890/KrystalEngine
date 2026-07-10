#pragma once

#include "Krystal.HTML/HTML/Enums/HTMLTokenType.hpp"
#include "Krystal.HTML/HTML/Enums/ParsedAttributeFlags.hpp"
#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/UniquePtr.hpp"
#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/SmallList.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/StringConversion.hpp"
#include <cassert>

namespace Krys::HTML
{
  struct DoctypeData
  {
    SmallList<char8> PublicIdentifier;
    SmallList<char8> SystemIdentifier;
    bool HasPublicIdentifier : 1 {false};
    bool HasSystemIdentifier : 1 {false};
    bool ForceQuirks : 1 {false};
  };

  struct ParsedAttribute
  {
    SmallList<char8, 32u> Name;
    SmallList<char8, 64u> Value;
    /// @brief Flags for the attribute, such as whether it is a foreign attribute or has a prefix. Set during
    /// AdjustForeignAttributes() in the tree builder; checked when creating elements.
    ParsedAttributeFlags Flags {ParsedAttributeFlags::None};

    DOMStringView NameView() const noexcept
    {
      return DOMStringView(Name.begin(), Name.end());
    }

    DOMStringView ValueView() const noexcept
    {
      return DOMStringView(Value.begin(), Value.end());
    }

    void SetName(DOMStringView characters) noexcept
    {
      Name.clear();
      Name.append(characters.begin(), characters.end());
    }
  };

  using ParsedAttributeList = SmallList<ParsedAttribute, 10u>;

  class HTMLToken
  {
  public:
    using DataBuffer = SmallList<char8, 128u>;

  private:
    HTMLTokenType _type : BitCount<HTMLTokenType>() {HTMLTokenType::Uninitialized};

    // For start/end tag tokens.
    bool _selfClosing : 1 {false};
    ParsedAttributeList _attributes;
    RawPtr<ParsedAttribute> _currentAttribute {nullptr};

    // For DOCTYPE only
    UniquePtr<DoctypeData> _doctypeData;

    // The name for DOCTYPE/start/end tokens, text data for character tokens, comment data for comment tokens.
    DataBuffer _data;

  public:
    HTMLToken() = default;

    KRYS_NODISCARD HTMLTokenType Type() const noexcept
    {
      return _type;
    }

    KRYS_NODISCARD DOMStringView Name() const noexcept
    {
      assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag
             || _type == HTMLTokenType::DOCTYPE);

      return DOMStringView(_data.begin(), _data.end());
    }

    KRYS_NODISCARD const DataBuffer &Data() const noexcept
    {
      return _data;
    }

    void Clear() noexcept
    {
      _type = HTMLTokenType::Uninitialized;
      _data.clear();
      _attributes.clear();
    }

    void AppendToName(char32 character) noexcept
    {
      assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag
             || _type == HTMLTokenType::DOCTYPE);

      AppendToDataInternal(character);
    }

#pragma region DOCTYPE

    void BeginDOCTYPE() noexcept
    {
      assert(_type == HTMLTokenType::Uninitialized);
      _type = HTMLTokenType::DOCTYPE;
      _doctypeData = CreateUnique<DoctypeData>();
    }

    void SetDOCTYPEForceQuirks() noexcept
    {
      assert(_type == HTMLTokenType::DOCTYPE);
      _doctypeData->ForceQuirks = true;
    }

    KRYS_NODISCARD bool IsForceQuirks() const noexcept
    {
      assert(_type == HTMLTokenType::DOCTYPE);
      return _doctypeData->ForceQuirks;
    }

    void SetPublicIdentifierToEmptyString()
    {
      assert(_type == HTMLTokenType::DOCTYPE);
      _doctypeData->HasPublicIdentifier = true;
      _doctypeData->PublicIdentifier.clear();
    }

    void SetSystemIdentifierToEmptyString()
    {
      assert(_type == HTMLTokenType::DOCTYPE);
      _doctypeData->HasSystemIdentifier = true;
      _doctypeData->SystemIdentifier.clear();
    }

    void AppendToPublicIdentifier(char32 character)
    {
      assert(_type == HTMLTokenType::DOCTYPE);
      assert(_doctypeData->HasPublicIdentifier);

      auto converted = Krys::Text::ConvertToUTF8(Span<char32>(&character, 1));
      _doctypeData->PublicIdentifier.append(converted.begin(), converted.end());
    }

    void AppendToSystemIdentifier(char32 character)
    {
      assert(_type == HTMLTokenType::DOCTYPE);
      assert(_doctypeData->HasSystemIdentifier);

      auto converted = Krys::Text::ConvertToUTF8(Span<char32>(&character, 1));
      _doctypeData->SystemIdentifier.append(converted.begin(), converted.end());
    }

    KRYS_NODISCARD UniquePtr<DoctypeData> ReleaseDOCTYPEData() noexcept
    {
      return std::move(_doctypeData);
    }

#pragma endregion

#pragma region Start/End Tag

    void BeginStartTag(char32 character) noexcept
    {
      assert(character);
      assert(_type == HTMLTokenType::Uninitialized);

      _type = HTMLTokenType::StartTag;
      _selfClosing = false;
      
      _attributes.clear();
      _currentAttribute = nullptr;

      AppendToDataInternal(character);
    }

    void BeginEndTag(char32 character) noexcept
    {
      BeginEndTag(Span<char32>(&character, 1));
    }

    void BeginEndTag(Span<char32> characters) noexcept
    {
      assert(_type == HTMLTokenType::Uninitialized);

      _type = HTMLTokenType::EndTag;
      _selfClosing = false;
      
      _attributes.clear();
      _currentAttribute = nullptr;

      AppendToDataInternal(characters);
    }

    void BeginAttribute() noexcept
    {
      assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag);
      assert(_currentAttribute == nullptr);

      _attributes.emplace_back();
      _currentAttribute = &_attributes.back();
    }

    void EndAttribute() noexcept
    {
      assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag);
      assert(_currentAttribute != nullptr);
      _currentAttribute = nullptr;
    }

    KRYS_NODISCARD ParsedAttributeList &Attributes() noexcept
    {
      assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag);
      return _attributes;
    }

    KRYS_NODISCARD const ParsedAttributeList &Attributes() const noexcept
    {
      assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag);
      return _attributes;
    }

    KRYS_NODISCARD RawPtr<ParsedAttribute> CurrentAttribute() const noexcept
    {
      assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag);
      return _currentAttribute;
    }

    void AppendToCurrentAttributeName(char32 character) noexcept
    {
      assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag);
      assert(_currentAttribute != nullptr);

      auto converted = Krys::Text::ConvertToUTF8(Span<char32>(&character, 1));
      _currentAttribute->Name.append(converted.begin(), converted.end());
    }

    void AppendToCurrentAttributeValue(char32 character) noexcept
    {
      assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag);
      assert(_currentAttribute != nullptr);

      auto converted = Krys::Text::ConvertToUTF8(Span<char32>(&character, 1));
      _currentAttribute->Value.append(converted.begin(), converted.end());
    }

    template <size_t N>
    void AppendToCurrentAttributeValue(Array<char32, N> characters)
    {
      assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag);
      assert(_currentAttribute != nullptr);

      auto converted = Krys::Text::ConvertToUTF8(characters);
      _currentAttribute->Value.append(converted.begin(), converted.end());
    }

    void AppendToCurrentAttributeValue(Span<char32> characters)
    {
      assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag);
      assert(_currentAttribute != nullptr);

      auto converted = Krys::Text::ConvertToUTF8(characters);
      _currentAttribute->Value.append(converted.begin(), converted.end());
    }

    void SetSelfClosingFlag() noexcept
    {
      assert(_type == HTMLTokenType::StartTag);
      _selfClosing = true;
    }

    KRYS_NODISCARD bool IsSelfClosing() const noexcept
    {
      assert(_type == HTMLTokenType::StartTag);
      return _selfClosing;
    }

#pragma endregion

#pragma region Character

    void AppendToCharacters(char32 character) noexcept
    {
      assert(_type == HTMLTokenType::Uninitialized || _type == HTMLTokenType::Character);
      _type = HTMLTokenType::Character;
      AppendToDataInternal(character);
    }

    void AppendToCharacters(Span<char32> characters) noexcept
    {
      assert(_type == HTMLTokenType::Uninitialized || _type == HTMLTokenType::Character);
      _type = HTMLTokenType::Character;
      AppendToDataInternal(characters);
    }

    void AppendToCharacters(utf32_stringview characters) noexcept
    {
      assert(_type == HTMLTokenType::Uninitialized || _type == HTMLTokenType::Character);
      _type = HTMLTokenType::Character;
      AppendToDataInternal(characters);
    }

#pragma endregion

#pragma region Comment

    void BeginComment() noexcept
    {
      assert(_type == HTMLTokenType::Uninitialized);
      _type = HTMLTokenType::Comment;
    }

    void AppendToComment(char32 character) noexcept
    {
      assert(_type == HTMLTokenType::Comment);
      AppendToDataInternal(character);
    }

    void AppendToComment(Krys::Text::ASCIILiteral characters) noexcept
    {
      assert(_type == HTMLTokenType::Comment);
      auto span = characters.ToSpan();
      _data.append(span.begin(), span.end());
    }

#pragma endregion

#pragma region EndOfFile

    void SetAsEOF() noexcept
    {
      assert(_type == HTMLTokenType::Uninitialized);
      _type = HTMLTokenType::EndOfFile;
    }

#pragma endregion

  private:
    void AppendToDataInternal(char32 character) noexcept
    {
      AppendToDataInternal(Span<char32>(&character, 1));
    }

    void AppendToDataInternal(utf32_stringview characters) noexcept
    {
      auto converted = Krys::Text::ConvertToUTF8(characters);
      _data.append(converted.begin(), converted.end());
    }

    void AppendToDataInternal(Span<char32> characters) noexcept
    {
      auto converted = Krys::Text::ConvertToUTF8(characters);
      _data.append(converted.begin(), converted.end());
    }
  };
}