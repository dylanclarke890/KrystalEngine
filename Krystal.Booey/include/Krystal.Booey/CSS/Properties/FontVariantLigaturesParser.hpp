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
  class FontVariantLigaturesParser
  {
  private:
    bool _sawCommonLigaturesValue = false;
    bool _sawDiscretionaryLigaturesValue = false;
    bool _sawHistoricalLigaturesValue = false;
    bool _sawContextualLigaturesValue = false;
    ValueListBuilder _result;

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