#pragma once

#include "Krystal.Booey/CSS/Parser/NumericValue.hpp"
#include "Krystal.Booey/CSS/Parser/TokenType.hpp"
#include "Krystal.Booey/CSS/Properties/CSSPropertyId.hpp"
#include "Krystal.Booey/CSS/Values/Enums/CSSValueId.hpp"
#include "Krystal.Core/Text/Encodings/Encode.hpp"
#include "Krystal.Core/Text/Encodings/UTF.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo::css
{
  class Token
  {
  private:
    TokenType _type : BitCount<TokenType>() {TokenType::Uninitialized};
    BlockTokenType _blockType : BitCount<BlockTokenType>() {BlockTokenType::None};
    HashTokenType _hashType : BitCount<HashTokenType>() {HashTokenType::Unrestricted};
    NumericValueType _numericValueType : BitCount<NumericValueType>() {NumericValueType::Integer};
    NumericSignChar _numericSignChar : BitCount<NumericSignChar>() {NumericSignChar::Missing};
    double _numericValue;
    SmallList<char8, 32uz> _codePoints;
    char32 _unicodeRangeStart;
    char32 _unicodeRangeEnd;

  public:
    Token(TokenType type) noexcept : _type(type)
    {
    }

    Token(TokenType type, BlockTokenType blockType) noexcept : _type(type), _blockType(blockType)
    {
    }

    Token(TokenType type, char32 codePoint) noexcept : _type(type)
    {
      assert(type == TokenType::Delim);

      AppendDataInternal(utf32_stringview(&codePoint, 1uz));
    }

    Token(TokenType type, utf32_stringview codePoints) noexcept : _type(type)
    {
      assert(_type == TokenType::Ident || _type == TokenType::Function || type == TokenType::Url
             || type == TokenType::String);

      AppendDataInternal(codePoints);
    }

    KRYS_NODISCARD TokenType Type() const noexcept
    {
      return _type;
    }

    KRYS_NODISCARD HashTokenType HashType() const noexcept
    {
      assert(_type == TokenType::Hash);
      return _hashType;
    }

    void HashType(HashTokenType hashType) noexcept
    {
      assert(_type == TokenType::Hash);
      _hashType = hashType;
    }

    KRYS_NODISCARD utf8_stringview IdentCodePoints() const noexcept
    {
      assert(_type == TokenType::Ident || _type == TokenType::Function || _type == TokenType::AtKeyword
             || _type == TokenType::Hash || _type == TokenType::String || _type == TokenType::Url
             || _type == TokenType::Delim);

      return utf8_stringview {_codePoints.data(), _codePoints.size()};
    }

    void IdentCodePoints(utf32_stringview codePoints) noexcept
    {
      assert(_type == TokenType::Ident || _type == TokenType::Function || _type == TokenType::AtKeyword
             || _type == TokenType::Hash || _type == TokenType::String || _type == TokenType::Url
             || _type == TokenType::Delim);

      AppendDataInternal(codePoints);
    }

    KRYS_NODISCARD CSSValueId ValueId() const noexcept
    {
      if (_type != TokenType::Ident)
      {
        return CSSValueId::Invalid;
      }

      return FindCSSValueKeyword(IdentCodePoints());
    }

    KRYS_NODISCARD CSSPropertyId PropertyId() const noexcept
    {
      if (_type != TokenType::Ident)
      {
        return CSSPropertyId::Invalid;
      }

      return FindCSSPropertyId(IdentCodePoints());
    }

    KRYS_NODISCARD CSSValueId FunctionId() const noexcept
    {
      if (_type != TokenType::Function)
      {
        return CSSValueId::Invalid;
      }

      return FindCSSValueKeyword(IdentCodePoints());
    }

    void NumericValue(NumericValue value) noexcept
    {
      assert(_type == TokenType::Number || _type == TokenType::Percentage || _type == TokenType::Dimension);

      _numericValue = value.Value;
      _numericValueType = value.Type;
      _numericSignChar = value.SignCharacter;
    }

    KRYS_NODISCARD double NumericValue() const noexcept
    {
      assert(_type == TokenType::Number || _type == TokenType::Percentage || _type == TokenType::Dimension);

      return _numericValue;
    }

    KRYS_NODISCARD NumericValueType NumericValueType() const noexcept
    {
      assert(_type == TokenType::Number || _type == TokenType::Percentage || _type == TokenType::Dimension);

      return _numericValueType;
    }

    void Unit(utf32_stringview unit) noexcept
    {
      assert(_type == TokenType::Dimension);
      AppendDataInternal(unit);
    }

    KRYS_NODISCARD utf8_stringview Unit() const noexcept
    {
      assert(_type == TokenType::Dimension);
      return utf8_stringview {_codePoints.data(), _codePoints.size()};
    }

    void UnicodeRange(char32 start, char32 end) noexcept
    {
      assert(_type == TokenType::UnicodeRange);

      _unicodeRangeStart = start;
      _unicodeRangeEnd = end;
    }

    KRYS_NODISCARD bool IsBlockStart() const noexcept
    {
      return _blockType == BlockTokenType::Start;
    }

    KRYS_NODISCARD bool IsBlockEnd() const noexcept
    {
      return _blockType == BlockTokenType::End;
    }

  private:
    void AppendDataInternal(utf32_stringview codePoints)
    {
      auto encodeResult = krys::text::Encode<krys::text::UTF8>(codePoints);
      krys_debug_assert(encodeResult.Error != krys::text::EncodeError::None);
      _codePoints.assign(encodeResult.Output.begin(), encodeResult.Output.end());
    }
  };
}