/*
 * Copyright (c) 2024 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/CSS/Properties/Consumers/PositionTry.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Anchor.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/List.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValueListBuilder.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  RefPtr<Value> ConsumePositionTryFallbacks(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <'position-try-fallbacks'> = none | [ [<dashed-ident> || <try-tactic>] | <'position-area'> ]#
    // https://drafts.csswg.org/css-anchor-position-1/#propdef-position-try-fallbacks

    if (auto result = ConsumeIdent<ValueId::None>(tokens))
    {
      return result;
    }

    auto ConsumeFallback = [&](TokenRange &tokens) -> RefPtr<Value>
    {
      // Try to parse <'position-area'>
      auto rangeCopy = tokens;
      // consumePositionArea accepts 'none', so detect and reject it beforehand.
      if (tokens.Peek().ValueId() == ValueId::None)
      {
        return nullptr;
      }

      if (auto positionArea = ConsumePositionArea(tokens, state))
      {
        return positionArea;
      }

      tokens = rangeCopy;

      // Try to parse [<dashed-ident> || <try-tactic>]
      // <try-tactic> = flip-block || flip-inline || flip-start || flip-x || flip-y
      auto tryRuleIdent = ConsumeDashedIdentRaw(tokens);

      SmallList<ValueId, 5uz> tryTactics;
      while (auto tactic = ConsumeIdentRaw<ValueId::FlipBlock, ValueId::FlipInline, ValueId::FlipStart,
                                           ValueId::FlipX, ValueId::FlipY>(tokens))
      {
        if (std::ranges::contains(tryTactics, *tactic))
        {
          return nullptr;
        }

        tryTactics.push_back(*tactic);
      }

      if (!tryRuleIdent)
      {
        tryRuleIdent = ConsumeDashedIdentRaw(tokens);
      }

      ValueListBuilder list;
      if (tryRuleIdent)
      {
        list.push_back(PrimitiveValue::CreateCustomIdent(*tryRuleIdent));
      }
      for (auto tactic : tryTactics)
      {
        list.push_back(PrimitiveValue::Create(tactic));
      }

      // At least one @position-try rule ident or tactic must be present.
      if (list.empty())
      {
        return nullptr;
      }

      return ValueList::CreateSpaceSeparated(krys::move(list));
    };

    return ConsumeListSeparatedBy<',', OneOrMore, ListOptimization::SingleValue>(tokens, ConsumeFallback);
  }
}