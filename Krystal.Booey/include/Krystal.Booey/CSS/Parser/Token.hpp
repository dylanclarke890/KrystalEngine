#pragma once

#include "Krystal.Booey/CSS/Parser/KeywordLookup.hpp"
#include "Krystal.Booey/CSS/Parser/NumericValue.hpp"
#include "Krystal.Booey/CSS/Parser/TokenSerialisationMode.hpp"
#include "Krystal.Booey/CSS/Parser/TokenType.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Text/Encodings/Encode.hpp"
#include "Krystal.Core/Text/Encodings/UTF.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo::css
{
  /// @see https://drafts.csswg.org/css-syntax/#tokenization
  class Token
  {
  private:
    using ValueIdStorage = std::underlying_type_t<css::ValueId>;

    /// @brief Marks `_valueId` as not looked up yet.
    constexpr static ValueIdStorage UncachedValueId = std::numeric_limits<ValueIdStorage>::max();
    static_assert(TotalValueKeywords < UncachedValueId);

    TokenType _type : BitCount<TokenType>() {TokenType::Uninitialized};
    BlockTokenType _blockType : BitCount<BlockTokenType>() {BlockTokenType::None};
    HashTokenType _hashType : BitCount<HashTokenType>() {HashTokenType::Unrestricted};
    css::NumericValueType _numericValueType
      : BitCount<css::NumericValueType>() {css::NumericValueType::Integer};
    NumericSignChar _numericSignChar : BitCount<NumericSignChar>() {NumericSignChar::Missing};
    double _numericValue {0.0};
    char32 _delimiter {0};
    char32 _unicodeRangeStart {0};
    char32 _unicodeRangeEnd {0};
    mutable ValueIdStorage _valueId {UncachedValueId};
    // For numeric and whitespace tokens, the original text is stored first and a dimension's unit after it.
    uint32 _originalTextLength {0};
    SmallList<char8, 32uz> _codePoints;

  public:
    Token(TokenType type) noexcept : _type(type)
    {
    }

    Token(TokenType type, BlockTokenType blockType) noexcept : _type(type), _blockType(blockType)
    {
    }

    Token(TokenType type, char32 codePoint) noexcept : _type(type), _delimiter(codePoint)
    {
      krys_debug_assert(type == TokenType::Delim);

      // Also stored as text so delimiters can be compared against string literals.
      AppendDataInternal(utf32_stringview(&codePoint, 1uz));
    }

    Token(TokenType type, utf32_stringview codePoints,
          BlockTokenType blockType = BlockTokenType::None) noexcept
        : _type(type), _blockType(blockType)
    {
      krys_debug_assert(_type == TokenType::Ident || _type == TokenType::Function || _type == TokenType::Url
                        || _type == TokenType::String);

      AppendDataInternal(codePoints);
    }

    KRYS_NODISCARD bool operator==(const Token &other) const noexcept;

    KRYS_NODISCARD TokenType Type() const noexcept
    {
      return _type;
    }

    KRYS_NODISCARD BlockTokenType BlockType() const noexcept
    {
      return _blockType;
    }

    KRYS_NODISCARD bool IsBlockStart() const noexcept
    {
      return _blockType == BlockTokenType::Start;
    }

    KRYS_NODISCARD bool IsBlockEnd() const noexcept
    {
      return _blockType == BlockTokenType::End;
    }

    KRYS_NODISCARD HashTokenType HashType() const noexcept
    {
      krys_debug_assert(_type == TokenType::Hash);
      return _hashType;
    }

    void HashType(HashTokenType hashType) noexcept
    {
      krys_debug_assert(_type == TokenType::Hash);
      _hashType = hashType;
    }

    KRYS_NODISCARD utf8_stringview IdentCodePoints() const noexcept
    {
      krys_debug_assert(_type == TokenType::Ident || _type == TokenType::Function
                        || _type == TokenType::AtKeyword || _type == TokenType::Hash
                        || _type == TokenType::String || _type == TokenType::Url
                        || _type == TokenType::Delim);

      return utf8_stringview {_codePoints.data(), _codePoints.size()};
    }

    void IdentCodePoints(utf32_stringview codePoints) noexcept
    {
      krys_debug_assert(_type == TokenType::Ident || _type == TokenType::Function
                        || _type == TokenType::AtKeyword || _type == TokenType::Hash
                        || _type == TokenType::String || _type == TokenType::Url
                        || _type == TokenType::Delim);

      AppendDataInternal(codePoints);
    }

    KRYS_NODISCARD char32 Delimiter() const noexcept
    {
      krys_debug_assert(_type == TokenType::Delim);
      return _delimiter;
    }

    KRYS_NODISCARD css::ValueId ValueId() const noexcept
    {
      if (_type != TokenType::Ident)
      {
        return css::ValueId::Invalid;
      }

      return IdentOrFunctionId();
    }

    KRYS_NODISCARD css::ValueId FunctionId() const noexcept
    {
      if (_type != TokenType::Function)
      {
        return css::ValueId::Invalid;
      }

      return IdentOrFunctionId();
    }

    KRYS_NODISCARD css::PropertyId PropertyId() const noexcept
    {
      if (_type != TokenType::Ident)
      {
        return css::PropertyId::Invalid;
      }

      return PropertyIdFromName(IdentCodePoints());
    }

    void NumericValue(css::NumericValue value) noexcept
    {
      krys_debug_assert(_type == TokenType::Number || _type == TokenType::Percentage
                        || _type == TokenType::Dimension);

      _numericValue = value.Value;
      _numericValueType = value.Type;
      _numericSignChar = value.SignCharacter;
    }

    KRYS_NODISCARD double NumericValue() const noexcept
    {
      krys_debug_assert(_type == TokenType::Number || _type == TokenType::Percentage
                        || _type == TokenType::Dimension);

      return _numericValue;
    }

    KRYS_NODISCARD css::NumericValueType NumericValueType() const noexcept
    {
      krys_debug_assert(_type == TokenType::Number || _type == TokenType::Percentage
                        || _type == TokenType::Dimension);

      return _numericValueType;
    }

    KRYS_NODISCARD NumericSignChar NumericSign() const noexcept
    {
      // This is valid for Dimension and Percentage tokens, but only used in <an+b> parsing on Number tokens.
      krys_debug_assert(_type == TokenType::Number);

      return _numericSignChar;
    }

    /// @brief Sets the text the token was tokenized from: the number (without `%`) for number and percentage
    /// tokens, the number and the unit as written (escapes included) for dimension tokens, and the whitespace
    /// for whitespace tokens.
    void OriginalText(utf32_stringview text) noexcept
    {
      krys_debug_assert(_type == TokenType::Number || _type == TokenType::Percentage
                        || _type == TokenType::Dimension || _type == TokenType::Whitespace);

      // Keep the unit, which is stored after the original text.
      SmallList<char8, 32uz> unit(_codePoints.begin() + _originalTextLength, _codePoints.end());
      AppendDataInternal(text);
      _originalTextLength = static_cast<uint32>(_codePoints.size());
      _codePoints.append(unit.begin(), unit.end());
    }

    /// @brief The text the token was tokenized from (see the setter).
    KRYS_NODISCARD utf8_stringview OriginalText() const noexcept
    {
      krys_debug_assert(_type == TokenType::Number || _type == TokenType::Percentage
                        || _type == TokenType::Dimension || _type == TokenType::Whitespace);

      return utf8_stringview {_codePoints.data(), _originalTextLength};
    }

    void Unit(utf32_stringview unit) noexcept
    {
      krys_debug_assert(_type == TokenType::Dimension);

      _codePoints.resize(_originalTextLength);
      AppendEncoded(unit);
    }

    KRYS_NODISCARD utf8_stringview Unit() const noexcept
    {
      krys_debug_assert(_type == TokenType::Dimension);
      return utf8_stringview {_codePoints.data() + _originalTextLength,
                              _codePoints.size() - _originalTextLength};
    }

    void UnicodeRange(char32 start, char32 end) noexcept
    {
      krys_debug_assert(_type == TokenType::UnicodeRange);

      _unicodeRangeStart = start;
      _unicodeRangeEnd = end;
    }

    KRYS_NODISCARD char32 UnicodeRangeStart() const noexcept
    {
      krys_debug_assert(_type == TokenType::UnicodeRange);
      return _unicodeRangeStart;
    }

    KRYS_NODISCARD char32 UnicodeRangeEnd() const noexcept
    {
      krys_debug_assert(_type == TokenType::UnicodeRange);
      return _unicodeRangeEnd;
    }

    void Serialise(CSSOMString &builder, const Token *nextToken = nullptr,
                   TokenSerialisationMode mode = TokenSerialisationMode::Normal) const noexcept;

  private:
    KRYS_NODISCARD css::ValueId IdentOrFunctionId() const noexcept
    {
      krys_debug_assert(_type == TokenType::Ident || _type == TokenType::Function);

      if (_valueId == UncachedValueId)
      {
        _valueId = static_cast<ValueIdStorage>(ValueIdFromName(IdentCodePoints()));
      }

      return static_cast<css::ValueId>(_valueId);
    }

    void AppendDataInternal(utf32_stringview codePoints) noexcept
    {
      _valueId = UncachedValueId;
      _codePoints.clear();
      AppendEncoded(codePoints);
    }

    void AppendEncoded(utf32_stringview codePoints) noexcept
    {
      // Empty strings and URLs are valid, but Encode reports empty input as an error.
      if (codePoints.empty())
      {
        return;
      }

      auto encodeResult = krys::text::Encode<krys::text::UTF8>(codePoints);
      krys_debug_assert(encodeResult.Error == krys::text::EncodeError::None);
      _codePoints.append(encodeResult.Output.begin(), encodeResult.Output.end());
    }
  };
}
