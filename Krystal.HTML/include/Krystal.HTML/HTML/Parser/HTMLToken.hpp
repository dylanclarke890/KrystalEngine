#pragma once

#include "Krystal.HTML/HTML/Enums/HTMLTokenType.hpp"
#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/UniquePtr.hpp"
#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/SmallList.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include <cassert>

namespace Krys::HTML
{
  struct DoctypeData
  {
    SmallList<char32> PublicIdentifier;
    SmallList<char32> SystemIdentifier;
    bool HasPublicIdentifier : 1 {false};
    bool HasSystemIdentifier : 1 {false};
    bool ForceQuirks : 1 {false};
  };

  class HTMLToken
  {
  public:
    struct Attribute
    {
      SmallList<char32, 32u> Name;
      SmallList<char32, 64u> Value;
    };

    using AttributeList = SmallList<Attribute, 10u>;
    using DataBuffer = SmallList<char32, 128u>;

  private:
    HTMLTokenType _type : BitCount<HTMLTokenType>() {HTMLTokenType::Uninitialized};

    // For start/end tag tokens.
    bool _selfClosing : 1 {false};
    AttributeList _attributes;
    RawPtr<Attribute> _currentAttribute {nullptr};

    // For DOCTYPE only
    UniquePtr<DoctypeData> _doctypeData;

    // The name for DOCTYPE/start/end tokens, text data for character tokens, comment data for comment tokens.
    DataBuffer _data;

  public:
    HTMLToken() = default;

    KRYS_NODISCARD HTMLTokenType GetType() const noexcept
    {
      return _type;
    }

    KRYS_NODISCARD utf32_string GetName() const noexcept
    {
      assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag
             || _type == HTMLTokenType::DOCTYPE);
      return utf32_string(_data.begin(), _data.end());
    }

    void AppendToName(char32 character) noexcept
    {
      assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag
             || _type == HTMLTokenType::DOCTYPE);
      _data.push_back(character);
    }

    void Clear() noexcept
    {
      _type = HTMLTokenType::Uninitialized;
      _data.clear();
    }

    KRYS_NODISCARD const DataBuffer &GetDataBuffer() const noexcept
    {
      return _data;
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

      _doctypeData->PublicIdentifier.push_back(character);
    }

    void AppendToSystemIdentifier(char32 character)
    {
      assert(_type == HTMLTokenType::DOCTYPE);
      assert(_doctypeData->HasSystemIdentifier);

      _doctypeData->SystemIdentifier.push_back(character);
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

#if KRYS_ENV(DEV)
      _currentAttribute = nullptr;
#endif

      _data.push_back(character);
    }

    void BeginEndTag(char32 character) noexcept
    {
      assert(character);
      assert(_type == HTMLTokenType::Uninitialized);

      _type = HTMLTokenType::EndTag;
      _selfClosing = false;
      _attributes.clear();

#if KRYS_ENV(DEV)
      _currentAttribute = nullptr;
#endif

      _data.push_back(character);
    }

    template <size_t N>
    void BeginEndTag(const SmallList<char32, N> &characters) noexcept
    {
      assert(_type == HTMLTokenType::Uninitialized);

      _type = HTMLTokenType::EndTag;
      _selfClosing = false;
      _attributes.clear();

#if KRYS_ENV(DEV)
      _currentAttribute = nullptr;
#endif

      _data.append(characters);
    }

    void BeginAttribute() noexcept
    {
#if KRYS_ENV(DEV)
      assert(_currentAttribute == nullptr);
#endif
      assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag);

      _attributes.emplace_back();
      _currentAttribute = &_attributes.back();
    }

    void EndAttribute() noexcept
    {
      assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag);
      assert(_currentAttribute != nullptr);

#if KRYS_ENV(DEV)
      _currentAttribute = nullptr;
#endif
    }

    KRYS_NODISCARD AttributeList &GetAttributes() noexcept
    {
      assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag);
      return _attributes;
    }

    KRYS_NODISCARD const AttributeList &GetAttributes() const noexcept
    {
      assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag);
      return _attributes;
    }

    KRYS_NODISCARD Attribute *GetCurrentAttribute() const noexcept
    {
      assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag);
      return _currentAttribute;
    }

    void AppendToCurrentAttributeName(char32 character) noexcept
    {
      assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag);
#if KRYS_ENV(DEV)
      assert(_currentAttribute != nullptr);
#endif

      _currentAttribute->Name.push_back(character);
    }

    void AppendToCurrentAttributeValue(char32 character) noexcept
    {
      assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag);
      assert(_currentAttribute != nullptr);

      _currentAttribute->Value.push_back(character);
    }

    template <size_t N>
    void AppendToCurrentAttributeValue(Array<char32, N> characters)
    {
      assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag);
      assert(_currentAttribute != nullptr);

      _currentAttribute->Value.append(characters.begin(), characters.end());
    }

    void AppendToCurrentAttributeValue(Span<char32> characters)
    {
      assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag);
      assert(_currentAttribute != nullptr);

      for (auto character : characters)
      {
        _currentAttribute->Value.push_back(character);
      }
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
      _data.push_back(character);
    }

    void AppendToCharacters(Span<char32> characters) noexcept
    {
      assert(_type == HTMLTokenType::Uninitialized || _type == HTMLTokenType::Character);
      _type = HTMLTokenType::Character;

      for (auto character : characters)
      {
        _data.push_back(character);
      }
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
      _data.push_back(character);
    }

    void AppendToComment(Krys::Text::ASCIILiteral characters) noexcept
    {
      assert(_type == HTMLTokenType::Comment);

      for (auto character : characters.ToSpan())
      {
        _data.push_back(character);
      }
    }

#pragma endregion

#pragma region EndOfFile

    void SetAsEOF() noexcept
    {
      assert(_type == HTMLTokenType::Uninitialized);
      _type = HTMLTokenType::EndOfFile;
    }

#pragma endregion
  };
}