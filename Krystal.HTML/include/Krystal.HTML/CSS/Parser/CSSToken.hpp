#pragma once

#include "Krystal.HTML/CSS/Parser/Enums/CSSTokenType.hpp"
#include "Krystal.HTML/CSS/Parser/Types/ParsedInt64OrDouble.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/SmallList.hpp"

namespace Krys::HTML
{
  class CSSToken
  {
  private:
    CSSTokenType _type : BitCount<CSSTokenType>() {CSSTokenType::Uninitialized};
    HashTokenType _hashType : BitCount<HashTokenType>() {HashTokenType::Unrestricted};
    NumericTokenType _numericTokenType : BitCount<NumericTokenType>() {NumericTokenType::Integer};
    Int64OrDouble _numericValue;
    SmallList<char32, 32uz> _codePoints;

  public:
    CSSToken(CSSTokenType type) noexcept : _type(type)
    {
    }

    CSSToken(CSSTokenType type, char32 codePoint) noexcept : _type(type)
    {
      assert(type == CSSTokenType::Delim);

      _codePoints.push_back(codePoint);
    }

    CSSToken(CSSTokenType type, utf32_stringview codePoints) noexcept : _type(type)
    {
      assert(_type == CSSTokenType::Ident || _type == CSSTokenType::Function || type == CSSTokenType::Url
             || type == CSSTokenType::String);

      _codePoints.assign(codePoints.begin(), codePoints.end());
    }

    void SetHashType(HashTokenType hashType) noexcept
    {
      assert(_type == CSSTokenType::Hash);

      _hashType = hashType;
    }

    void SetIdentCodePoints(utf32_stringview codePoints) noexcept
    {
      assert(_type == CSSTokenType::Ident || _type == CSSTokenType::Function
             || _type == CSSTokenType::AtKeyword);

      _codePoints.assign(codePoints.begin(), codePoints.end());
    }

    void SetNumericValue(ParsedInt64OrDouble value) noexcept
    {
      assert(_type == CSSTokenType::Number || _type == CSSTokenType::Percentage
             || _type == CSSTokenType::Dimension);

      _numericValue = value.Value;
      _numericTokenType = value.Type;
    }

    void SetUnit(utf32_stringview unit) noexcept
    {
      assert(_type == CSSTokenType::Dimension);

      _codePoints.assign(unit.begin(), unit.end());
    }
  };
}