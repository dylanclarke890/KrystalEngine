#pragma once

#include "Krystal.HTML/CSS/Parser/Enums/CSSTokenType.hpp"
#include "Krystal.HTML/CSS/Parser/Types/NumericValue.hpp"
#include "Krystal.HTML/CSS/Values/Enums/CSSValueId.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/SmallList.hpp"
#include "Krystal.Text/StringConversion.hpp"

namespace Krys::HTML
{
  class CSSToken
  {
  private:
    CSSTokenType _type : BitCount<CSSTokenType>() {CSSTokenType::Uninitialized};
    BlockTokenType _blockType : BitCount<BlockTokenType>() {BlockTokenType::None};
    HashTokenType _hashType : BitCount<HashTokenType>() {HashTokenType::Unrestricted};
    NumericValueType _numericValueType : BitCount<NumericValueType>() {NumericValueType::Integer};
    NumericSignChar _numericSignChar : BitCount<NumericSignChar>() {NumericSignChar::Missing};
    double _numericValue;
    SmallList<char8, 32uz> _codePoints;
    char32 _unicodeRangeStart;
    char32 _unicodeRangeEnd;

  public:
    CSSToken(CSSTokenType type) noexcept : _type(type)
    {
    }

    CSSToken(CSSTokenType type, BlockTokenType blockType) noexcept : _type(type), _blockType(blockType)
    {
    }

    CSSToken(CSSTokenType type, char32 codePoint) noexcept : _type(type)
    {
      assert(type == CSSTokenType::Delim);

      AppendDataInternal(utf32_stringview(&codePoint, 1uz));
    }

    CSSToken(CSSTokenType type, utf32_stringview codePoints) noexcept : _type(type)
    {
      assert(_type == CSSTokenType::Ident || _type == CSSTokenType::Function || type == CSSTokenType::Url
             || type == CSSTokenType::String);

      AppendDataInternal(codePoints);
    }

    KRYS_NODISCARD CSSTokenType Type() const noexcept
    {
      return _type;
    }

    KRYS_NODISCARD HashTokenType HashType() const noexcept
    {
      assert(_type == CSSTokenType::Hash);
      return _hashType;
    }

    void HashType(HashTokenType hashType) noexcept
    {
      assert(_type == CSSTokenType::Hash);
      _hashType = hashType;
    }

    KRYS_NODISCARD utf8_stringview IdentCodePoints() const noexcept
    {
      assert(_type == CSSTokenType::Ident || _type == CSSTokenType::Function
             || _type == CSSTokenType::AtKeyword || _type == CSSTokenType::Hash
             || _type == CSSTokenType::String || _type == CSSTokenType::Url || _type == CSSTokenType::Delim);

      return utf8_stringview {_codePoints.data(), _codePoints.size()};
    }

    void IdentCodePoints(utf32_stringview codePoints) noexcept
    {
      assert(_type == CSSTokenType::Ident || _type == CSSTokenType::Function
             || _type == CSSTokenType::AtKeyword || _type == CSSTokenType::Hash
             || _type == CSSTokenType::String || _type == CSSTokenType::Url || _type == CSSTokenType::Delim);

      AppendDataInternal(codePoints);
    }

    KRYS_NODISCARD CSSValueId ValueId() const noexcept
    {
      assert(_type == CSSTokenType::Ident);
      return FindCSSValueKeyword(IdentCodePoints());
    }

    void NumericValue(NumericValue value) noexcept
    {
      assert(_type == CSSTokenType::Number || _type == CSSTokenType::Percentage
             || _type == CSSTokenType::Dimension);

      _numericValue = value.Value;
      _numericValueType = value.Type;
      _numericSignChar = value.SignCharacter;
    }

    KRYS_NODISCARD double NumericValue() const noexcept
    {
      assert(_type == CSSTokenType::Number || _type == CSSTokenType::Percentage
             || _type == CSSTokenType::Dimension);

      return _numericValue;
    }

    KRYS_NODISCARD NumericValueType NumericValueType() const noexcept
    {
      assert(_type == CSSTokenType::Number || _type == CSSTokenType::Percentage
             || _type == CSSTokenType::Dimension);

      return _numericValueType;
    }

    void Unit(utf32_stringview unit) noexcept
    {
      assert(_type == CSSTokenType::Dimension);
      AppendDataInternal(unit);
    }

    KRYS_NODISCARD utf8_stringview Unit() const noexcept
    {
      assert(_type == CSSTokenType::Dimension);
      return utf8_stringview {_codePoints.data(), _codePoints.size()};
    }

    void UnicodeRange(char32 start, char32 end) noexcept
    {
      assert(_type == CSSTokenType::UnicodeRange);

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
      auto converted = Krys::Text::ConvertToUTF8(codePoints);
      _codePoints.assign(converted.begin(), converted.end());
    }
  };
}