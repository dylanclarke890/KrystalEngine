#pragma once

#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueList.hpp"
#include "Krystal.HTML/CSS/Values/Enums/CSSValueId.hpp"

namespace Krys::HTML
{
  class CSSFontVariantLigaturesParser
  {
  private:
    bool _sawCommonLigaturesValue = false;
    bool _sawDiscretionaryLigaturesValue = false;
    bool _sawHistoricalLigaturesValue = false;
    bool _sawContextualLigaturesValue = false;
    CSSValueListBuilder _result;

  public:
    enum class ParseResult : uint8
    {
      ConsumedValue,
      DisallowedValue,
      UnknownValue
    };

    CSSFontVariantLigaturesParser() noexcept = default;

    KRYS_NODISCARD ParseResult ConsumeLigature(CSSTokenRange &range) noexcept
    {
      CSSValueId valueID = range.Peek().ValueId();
      switch (valueID)
      {
        case CSSValueId::NoCommonLigatures:
        case CSSValueId::CommonLigatures:
        {
          if (_sawCommonLigaturesValue)
          {
            return ParseResult::DisallowedValue;
          }

          _sawCommonLigaturesValue = true;
          break;
        }
        case CSSValueId::NoDiscretionaryLigatures:
        case CSSValueId::DiscretionaryLigatures:
        {
          if (_sawDiscretionaryLigaturesValue)
          {
            return ParseResult::DisallowedValue;
          }

          _sawDiscretionaryLigaturesValue = true;
          break;
        }
        case CSSValueId::NoHistoricalLigatures:
        case CSSValueId::HistoricalLigatures:
        {
          if (_sawHistoricalLigaturesValue)
          {
            return ParseResult::DisallowedValue;
          }

          _sawHistoricalLigaturesValue = true;
          break;
        }
        case CSSValueId::NoContextual:
        case CSSValueId::Contextual:
        {
          if (_sawContextualLigaturesValue)
          {
            return ParseResult::DisallowedValue;
          }

          _sawContextualLigaturesValue = true;
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