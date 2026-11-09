/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"

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
    ValueListBuilder _result;

  public:
    enum class ParseResult : uint8
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

    KRYS_NODISCARD RefPtr<Value> FinalizeValue() noexcept
    {
      if (_result.empty())
      {
        return PrimitiveValue::Create(ValueId::Normal);
      }

      return ValueList::CreateSpaceSeparated(krys::move(_result));
    }
  };
}