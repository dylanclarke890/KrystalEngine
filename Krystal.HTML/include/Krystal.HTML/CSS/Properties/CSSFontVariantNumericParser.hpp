#pragma once

#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueList.hpp"
#include "Krystal.HTML/CSS/Values/Enums/CSSValueId.hpp"

namespace Krys::HTML
{
  class CSSFontVariantNumericParser
  {
  private:
    bool _sawNumericFigureValue : 1 {false};
    bool _sawNumericSpacingValue : 1 {false};
    bool _sawNumericFractionValue : 1 {false};
    bool _sawOrdinalValue : 1 {false};
    bool _sawSlashedZeroValue : 1 {false};
    CSSValueListBuilder _result;

  public:
    enum class ParseResult : uint8_t
    {
      ConsumedValue,
      DisallowedValue,
      UnknownValue
    };

    CSSFontVariantNumericParser() noexcept = default;

    KRYS_NODISCARD ParseResult ConsumeNumeric(CSSTokenRange &range) noexcept
    {
      CSSValueId valueId = range.Peek().ValueId();
      switch (valueId)
      {
        case CSSValueId::LiningNums:
        case CSSValueId::OldstyleNums:
        {
          if (_sawNumericFigureValue)
          {
            return ParseResult::DisallowedValue;
          }

          _sawNumericFigureValue = true;
          break;
        }
        case CSSValueId::ProportionalNums:
        case CSSValueId::TabularNums:
        {
          if (_sawNumericSpacingValue)
          {
            return ParseResult::DisallowedValue;
          }

          _sawNumericSpacingValue = true;
          break;
        }
        case CSSValueId::DiagonalFractions:
        case CSSValueId::StackedFractions:
        {
          if (_sawNumericFractionValue)
          {
            return ParseResult::DisallowedValue;
          }

          _sawNumericFractionValue = true;
          break;
        }
        case CSSValueId::Ordinal:
        {
          if (_sawOrdinalValue)
          {
            return ParseResult::DisallowedValue;
          }

          _sawOrdinalValue = true;
          break;
        }
        case CSSValueId::SlashedZero:
        {
          if (_sawSlashedZeroValue)
          {
            return ParseResult::DisallowedValue;
          }

          _sawSlashedZeroValue = true;
          break;
        }
        default:
        {
          return ParseResult::UnknownValue;
        }
      }

      _result.push_back(CSSPropertyParserHelpers::ConsumeIdent(range));
      return ParseResult::ConsumedValue;
    }

    KRYS_NODISCARD RefPtr<CSSValue> Finalize() noexcept
    {
      if (_result.empty())
      {
        return CSSPrimitiveValue::Create(CSSValueId::Normal);
      }

      return CSSValueList::CreateSpaceSeparated(Krys::Move(_result));
    }
  };
}