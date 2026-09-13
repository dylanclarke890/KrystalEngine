#pragma once

#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Values/CSSValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"

namespace krys::boo::css
{
  class FontVariantNumericParser
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

    FontVariantNumericParser() noexcept = default;

    KRYS_NODISCARD ParseResult ConsumeNumeric(TokenRange &range) noexcept
    {
      ValueId valueId = range.Peek().ValueId();
      switch (valueId)
      {
        case ValueId::LiningNums:
        case ValueId::OldstyleNums:
        {
          if (_sawNumericFigureValue)
          {
            return ParseResult::DisallowedValue;
          }

          _sawNumericFigureValue = true;
          break;
        }
        case ValueId::ProportionalNums:
        case ValueId::TabularNums:
        {
          if (_sawNumericSpacingValue)
          {
            return ParseResult::DisallowedValue;
          }

          _sawNumericSpacingValue = true;
          break;
        }
        case ValueId::DiagonalFractions:
        case ValueId::StackedFractions:
        {
          if (_sawNumericFractionValue)
          {
            return ParseResult::DisallowedValue;
          }

          _sawNumericFractionValue = true;
          break;
        }
        case ValueId::Ordinal:
        {
          if (_sawOrdinalValue)
          {
            return ParseResult::DisallowedValue;
          }

          _sawOrdinalValue = true;
          break;
        }
        case ValueId::SlashedZero:
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

      _result.push_back(PropertyParserHelpers::ConsumeIdent(range));
      return ParseResult::ConsumedValue;
    }

    KRYS_NODISCARD RefPtr<CSSValue> Finalize() noexcept
    {
      if (_result.empty())
      {
        return CSSPrimitiveValue::Create(ValueId::Normal);
      }

      return CSSValueList::CreateSpaceSeparated(krys::move(_result));
    }
  };
}