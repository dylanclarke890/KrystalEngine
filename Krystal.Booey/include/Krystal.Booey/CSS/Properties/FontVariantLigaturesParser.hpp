#pragma once

#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Values/CSSValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"

namespace krys::boo::css
{
  class FontVariantLigaturesParser
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

    FontVariantLigaturesParser() noexcept = default;

    KRYS_NODISCARD ParseResult ConsumeLigature(TokenRange &range) noexcept
    {
      ValueId valueID = range.Peek().ValueId();
      switch (valueID)
      {
        case ValueId::NoCommonLigatures:
        case ValueId::CommonLigatures:
        {
          if (_sawCommonLigaturesValue)
          {
            return ParseResult::DisallowedValue;
          }

          _sawCommonLigaturesValue = true;
          break;
        }
        case ValueId::NoDiscretionaryLigatures:
        case ValueId::DiscretionaryLigatures:
        {
          if (_sawDiscretionaryLigaturesValue)
          {
            return ParseResult::DisallowedValue;
          }

          _sawDiscretionaryLigaturesValue = true;
          break;
        }
        case ValueId::NoHistoricalLigatures:
        case ValueId::HistoricalLigatures:
        {
          if (_sawHistoricalLigaturesValue)
          {
            return ParseResult::DisallowedValue;
          }

          _sawHistoricalLigaturesValue = true;
          break;
        }
        case ValueId::NoContextual:
        case ValueId::Contextual:
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