#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Base.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Text/Encodings/Encode.hpp"
#include "Krystal.Core/Text/Encodings/UTF.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::html
{
  enum class HTMLTokenType : uint8
  {
    Uninitialized,
    DOCTYPE,
    StartTag,
    EndTag,
    Comment,
    Character,
    EndOfFile,
  };

  /// @see A general-purpose enum for all attribute states defined by the HTML spec. May split this out later.
  enum class ParsedAttributeFlags : uint8
  {
    None = 0,
    HasPrefix = 1 << 1,
    IsXMLNS = 1 << 2,
    IsXML = 1 << 3,
    IsXLink = 1 << 4,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::html::HTMLTokenType, 7uz);
KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::boo::html::ParsedAttributeFlags, 5uz);

namespace krys::boo::html
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

    dom::DOMStringView NameView() const noexcept
    {
      return dom::DOMStringView(Name.begin(), Name.end());
    }

    dom::DOMStringView ValueView() const noexcept
    {
      return dom::DOMStringView(Value.begin(), Value.end());
    }

    void SetName(dom::DOMStringView characters) noexcept
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
    ParsedAttribute *_currentAttribute {nullptr};

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

    KRYS_NODISCARD dom::DOMStringView Name() const noexcept
    {
      krys_debug_assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag
                        || _type == HTMLTokenType::DOCTYPE);

      return dom::DOMStringView(_data.begin(), _data.end());
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
      krys_debug_assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag
                        || _type == HTMLTokenType::DOCTYPE);

      AppendToDataInternal(character);
    }

#pragma region DOCTYPE

    void BeginDOCTYPE() noexcept
    {
      krys_debug_assert(_type == HTMLTokenType::Uninitialized);
      _type = HTMLTokenType::DOCTYPE;
      _doctypeData = CreateUnique<DoctypeData>();
    }

    void SetDOCTYPEForceQuirks() noexcept
    {
      krys_debug_assert(_type == HTMLTokenType::DOCTYPE);
      _doctypeData->ForceQuirks = true;
    }

    KRYS_NODISCARD bool IsForceQuirks() const noexcept
    {
      krys_debug_assert(_type == HTMLTokenType::DOCTYPE);
      return _doctypeData->ForceQuirks;
    }

    void SetPublicIdentifierToEmptyString()
    {
      krys_debug_assert(_type == HTMLTokenType::DOCTYPE);
      _doctypeData->HasPublicIdentifier = true;
      _doctypeData->PublicIdentifier.clear();
    }

    void SetSystemIdentifierToEmptyString()
    {
      krys_debug_assert(_type == HTMLTokenType::DOCTYPE);
      _doctypeData->HasSystemIdentifier = true;
      _doctypeData->SystemIdentifier.clear();
    }

    void AppendToPublicIdentifier(char32 character)
    {
      krys_debug_assert(_type == HTMLTokenType::DOCTYPE);
      krys_debug_assert(_doctypeData->HasPublicIdentifier);

      auto encodeResult = krys::text::Encode<krys::text::UTF8>(Span<char32>(&character, 1));
      krys_debug_assert(encodeResult.Error == krys::text::EncodeError::None);
      _doctypeData->PublicIdentifier.append(encodeResult.Output.begin(), encodeResult.Output.end());
    }

    void AppendToSystemIdentifier(char32 character)
    {
      krys_debug_assert(_type == HTMLTokenType::DOCTYPE);
      krys_debug_assert(_doctypeData->HasSystemIdentifier);

      auto encodeResult = krys::text::Encode<krys::text::UTF8>(Span<char32>(&character, 1));
      krys_debug_assert(encodeResult.Error == krys::text::EncodeError::None);
      _doctypeData->SystemIdentifier.append(encodeResult.Output.begin(), encodeResult.Output.end());
    }

    KRYS_NODISCARD UniquePtr<DoctypeData> ReleaseDOCTYPEData() noexcept
    {
      return std::move(_doctypeData);
    }

#pragma endregion

#pragma region Start/End Tag

    void BeginStartTag(char32 character) noexcept
    {
      krys_debug_assert(character);
      krys_debug_assert(_type == HTMLTokenType::Uninitialized);

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
      krys_debug_assert(_type == HTMLTokenType::Uninitialized);

      _type = HTMLTokenType::EndTag;
      _selfClosing = false;

      _attributes.clear();
      _currentAttribute = nullptr;

      AppendToDataInternal(characters);
    }

    void BeginAttribute() noexcept
    {
      krys_debug_assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag);
      krys_debug_assert(_currentAttribute == nullptr);

      _attributes.emplace_back();
      _currentAttribute = &_attributes.back();
    }

    void EndAttribute() noexcept
    {
      krys_debug_assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag);
      krys_debug_assert(_currentAttribute != nullptr);
      _currentAttribute = nullptr;
    }

    KRYS_NODISCARD ParsedAttributeList &Attributes() noexcept
    {
      krys_debug_assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag);
      return _attributes;
    }

    KRYS_NODISCARD const ParsedAttributeList &Attributes() const noexcept
    {
      krys_debug_assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag);
      return _attributes;
    }

    KRYS_NODISCARD ParsedAttribute *CurrentAttribute() const noexcept
    {
      krys_debug_assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag);
      return _currentAttribute;
    }

    void AppendToCurrentAttributeName(char32 character) noexcept
    {
      krys_debug_assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag);
      krys_debug_assert(_currentAttribute != nullptr);

      auto encodeResult = krys::text::Encode<krys::text::UTF8>(Span<char32>(&character, 1));
      krys_debug_assert(encodeResult.Error == krys::text::EncodeError::None);
      _currentAttribute->Name.append(encodeResult.Output.begin(), encodeResult.Output.end());
    }

    void AppendToCurrentAttributeValue(char32 character) noexcept
    {
      krys_debug_assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag);
      krys_debug_assert(_currentAttribute != nullptr);

      auto encodeResult = krys::text::Encode<krys::text::UTF8>(Span<char32>(&character, 1));
      krys_debug_assert(encodeResult.Error == krys::text::EncodeError::None);
      _currentAttribute->Value.append(encodeResult.Output.begin(), encodeResult.Output.end());
    }

    template <size_t N>
    void AppendToCurrentAttributeValue(Array<char32, N> characters)
    {
      krys_debug_assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag);
      krys_debug_assert(_currentAttribute != nullptr);

      auto encodeResult = krys::text::Encode<krys::text::UTF8>(characters);
      krys_debug_assert(encodeResult.Error == krys::text::EncodeError::None);
      _currentAttribute->Value.append(encodeResult.Output.begin(), encodeResult.Output.end());
    }

    void AppendToCurrentAttributeValue(Span<char32> characters)
    {
      krys_debug_assert(_type == HTMLTokenType::StartTag || _type == HTMLTokenType::EndTag);
      krys_debug_assert(_currentAttribute != nullptr);

      auto encodeResult = krys::text::Encode<krys::text::UTF8>(characters);
      krys_debug_assert(encodeResult.Error == krys::text::EncodeError::None);
      _currentAttribute->Value.append(encodeResult.Output.begin(), encodeResult.Output.end());
    }

    void SetSelfClosingFlag() noexcept
    {
      krys_debug_assert(_type == HTMLTokenType::StartTag);
      _selfClosing = true;
    }

    KRYS_NODISCARD bool IsSelfClosing() const noexcept
    {
      krys_debug_assert(_type == HTMLTokenType::StartTag);
      return _selfClosing;
    }

#pragma endregion

#pragma region Character

    void AppendToCharacters(char32 character) noexcept
    {
      krys_debug_assert(_type == HTMLTokenType::Uninitialized || _type == HTMLTokenType::Character);
      _type = HTMLTokenType::Character;
      AppendToDataInternal(character);
    }

    void AppendToCharacters(Span<char32> characters) noexcept
    {
      krys_debug_assert(_type == HTMLTokenType::Uninitialized || _type == HTMLTokenType::Character);
      _type = HTMLTokenType::Character;
      AppendToDataInternal(characters);
    }

    void AppendToCharacters(utf32_stringview characters) noexcept
    {
      krys_debug_assert(_type == HTMLTokenType::Uninitialized || _type == HTMLTokenType::Character);
      _type = HTMLTokenType::Character;
      AppendToDataInternal(characters);
    }

#pragma endregion

#pragma region Comment

    void BeginComment() noexcept
    {
      krys_debug_assert(_type == HTMLTokenType::Uninitialized);
      _type = HTMLTokenType::Comment;
    }

    void AppendToComment(char32 character) noexcept
    {
      krys_debug_assert(_type == HTMLTokenType::Comment);
      AppendToDataInternal(character);
    }

    void AppendToComment(stringview characters) noexcept
    {
      krys_debug_assert(_type == HTMLTokenType::Comment);
      _data.append(characters.begin(), characters.end());
    }

#pragma endregion

#pragma region EndOfFile

    void SetAsEOF() noexcept
    {
      krys_debug_assert(_type == HTMLTokenType::Uninitialized);
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
      auto encodeResult = krys::text::Encode<krys::text::UTF8>(characters);
      krys_debug_assert(encodeResult.Error == krys::text::EncodeError::None);
      _data.append(encodeResult.Output.begin(), encodeResult.Output.end());
    }

    void AppendToDataInternal(Span<char32> characters) noexcept
    {
      auto encodeResult = krys::text::Encode<krys::text::UTF8>(characters);
      krys_debug_assert(encodeResult.Error == krys::text::EncodeError::None);
      _data.append(encodeResult.Output.begin(), encodeResult.Output.end());
    }
  };
}