#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/UniquePtr.hpp"
#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::HTML
{
  struct DoctypeData
  {
    List<char32> PublicIdentifier;
    List<char32> SystemIdentifier;
    bool HasPublicIdentifier : 1 {false};
    bool HasSystemIdentifier : 1 {false};
    bool ForceQuirks : 1 {false};
  };

  class HTMLToken
  {
  public:
    enum class Type : uint8
    {
      Uninitialized,
      DOCTYPE,
      StartTag,
      EndTag,
      Comment,
      Character,
      EndOfFile,
    };

    struct Attribute
    {
      List<char32> Name;
      List<char32> Value;
    };

    using AttributeList = List<Attribute>;
    using DataBuffer = List<char32>;

  private:
    Type _type : 7 {Type::Uninitialized};

    // For start/end tag tokens.
    bool _selfClosing : 1 {false};

    // For DOCTYPE only
    UniquePtr<DoctypeData> _doctypeData;

    // For start/end tag tokens only
    AttributeList _attributes;
    RawPtr<Attribute> _currentAttribute {nullptr};

    // The name for DOCTYPE/start/end tokens, text data for character tokens, comment data for comment tokens.
    DataBuffer _data;

  public:
    HTMLToken() = default;

    KRYS_NODISCARD Type GetType() const noexcept
    {
      return _type;
    }

    void AppendToCharacters(char32 character) noexcept
    {
      assert(_type == Type::Uninitialized || _type == Type::Character);
      _type = Type::Character;
      _data.push_back(character);
    }

    void AppendToCharacters(Span<char32> characters) noexcept
    {
      assert(_type == Type::Uninitialized || _type == Type::Character);
      _type = Type::Character;
      _data.append_range(characters);
    }

    void AppendToName(char32 character) noexcept
    {
      assert(_type == Type::StartTag || _type == Type::EndTag || _type == Type::DOCTYPE);
      _data.push_back(character);
    }

    utf32_string GetName() const noexcept
    {
      assert(_type == Type::StartTag || _type == Type::EndTag || _type == Type::DOCTYPE);
      return utf32_string(_data.begin(), _data.end());
    }

    void BeginComment() noexcept
    {
      assert(_type == Type::Uninitialized);
      _type = Type::Comment;
    }

    void AppendToComment(char32 character) noexcept
    {
      assert(_type == Type::Comment);
      _data.push_back(character);
    }

    void AppendToComment(Text::ASCIILiteral characters) noexcept
    {
      assert(_type == Type::Comment);
      _data.append_range(characters.ToSpan());
    }

    void BeginStartTag(char32 character) noexcept
    {
      assert(character);
      assert(_type == Type::Uninitialized);

      _type = Type::StartTag;
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
      assert(_type == Type::Uninitialized);

      _type = Type::EndTag;
      _selfClosing = false;
      _attributes.clear();

#if KRYS_ENV(DEV)
      _currentAttribute = nullptr;
#endif

      _data.push_back(character);
    }

    void BeginEndTag(utf32_stringview characters) noexcept
    {
      assert(_type == Type::Uninitialized);

      _type = Type::EndTag;
      _selfClosing = false;
      _attributes.clear();

#if KRYS_ENV(DEV)
      _currentAttribute = nullptr;
#endif

      _data.append_range(characters);
    }

    void BeginAttribute() noexcept
    {
#if KRYS_ENV(DEV)
      assert(_currentAttribute == nullptr);
#endif
      assert(_type == Type::StartTag || _type == Type::EndTag);

      _attributes.emplace_back();
      _currentAttribute = &_attributes.back();
    }

    void EndAttribute() noexcept
    {
      assert(_type == Type::StartTag || _type == Type::EndTag);
      assert(_currentAttribute != nullptr);

#if KRYS_ENV(DEV)
      _currentAttribute = nullptr;
#endif
    }

    AttributeList &GetAttributes() noexcept
    {
      assert(_type == Type::StartTag || _type == Type::EndTag);
      return _attributes;
    }

    Attribute *GetCurrentAttribute() const noexcept
    {
      assert(_type == Type::StartTag || _type == Type::EndTag);
      return _currentAttribute;
    }

    void AppendToCurrentAttributeName(char32 character) noexcept
    {
      assert(_type == Type::StartTag || _type == Type::EndTag);
#if KRYS_ENV(DEV)
      assert(_currentAttribute != nullptr);
#endif

      _currentAttribute->Name.push_back(character);
    }

    void AppendToCurrentAttributeValue(char32 character) noexcept
    {
      assert(_type == Type::StartTag || _type == Type::EndTag);
      assert(_currentAttribute != nullptr);

      _currentAttribute->Value.push_back(character);
    }

    template <size_t N>
    void AppendToCurrentAttributeValue(Array<char32, N> characters)
    {
      assert(_type == Type::StartTag || _type == Type::EndTag);
      assert(_currentAttribute != nullptr);

      _currentAttribute->Value.append_range(characters);
    }

    void AppendToCurrentAttributeValue(Span<char32> characters)
    {
      assert(_type == Type::StartTag || _type == Type::EndTag);
      assert(_currentAttribute != nullptr);

      _currentAttribute->Value.append_range(characters);
    }

    void BeginDOCTYPE() noexcept
    {
      assert(_type == Type::Uninitialized);

      _type = Type::DOCTYPE;
      _doctypeData = CreateUnique<DoctypeData>();
    }

    void BeginDOCTYPE(char32 character) noexcept
    {
      assert(_type == Type::Uninitialized);

      _type = Type::DOCTYPE;
      _doctypeData = CreateUnique<DoctypeData>();
      _data.push_back(character);
    }

    void SetDOCTYPEForceQuirks() noexcept
    {
      assert(_type == Type::DOCTYPE);

      _doctypeData->ForceQuirks = true;
    }

    void SetPublicIdentifierToEmptyString()
    {
      assert(_type == Type::DOCTYPE);

      _doctypeData->HasPublicIdentifier = true;
      _doctypeData->PublicIdentifier.clear();
    }

    void SetSystemIdentifierToEmptyString()
    {
      assert(_type == Type::DOCTYPE);
      _doctypeData->HasSystemIdentifier = true;
      _doctypeData->SystemIdentifier.clear();
    }

    void AppendToPublicIdentifier(char32 character)
    {
      assert(_type == Type::DOCTYPE);
      assert(_doctypeData->HasPublicIdentifier);

      _doctypeData->PublicIdentifier.push_back(character);
    }

    void AppendToSystemIdentifier(char32 character)
    {
      assert(_type == Type::DOCTYPE);
      assert(_doctypeData->HasSystemIdentifier);

      _doctypeData->SystemIdentifier.push_back(character);
    }

    KRYS_NODISCARD UniquePtr<DoctypeData> ReleaseDOCTYPEData() noexcept
    {
      return std::move(_doctypeData);
    }

    void SetSelfClosingFlag() noexcept
    {
      assert(_type == Type::StartTag);
      _selfClosing = true;
    }

    void SetAsEOF() noexcept
    {
      assert(_type == Type::Uninitialized);
      _type = Type::EndOfFile;
    }

    void Clear() noexcept
    {
      _type = Type::Uninitialized;
      _data.clear();
    }

    KRYS_NODISCARD const DataBuffer &GetDataBuffer() const noexcept
    {
      return _data;
    }

    KRYS_NODISCARD bool IsSelfClosing() const noexcept
    {
      assert(_type == Type::StartTag);
      return _selfClosing;
    }

    KRYS_NODISCARD bool IsForceQuirks() const noexcept
    {
      assert(_type == Type::DOCTYPE);
      return _doctypeData->ForceQuirks;
    }
  };
}