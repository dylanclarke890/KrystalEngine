#include "Krystal.Booey/CSS/Properties/Consumers/PositionTry.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Anchor.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/List.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/CSSValueList.hpp"
#include "Krystal.Booey/CSS/Values/CSSValueListBuilder.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  RefPtr<CSSValue> ConsumePositionTryFallbacks(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <'position-try-fallbacks'> = none | [ [<dashed-ident> || <try-tactic>] | <'position-area'> ]#
    // https://drafts.csswg.org/css-anchor-position-1/#propdef-position-try-fallbacks

    if (auto result = ConsumeIdent<ValueId::None>(tokens))
    {
      return result;
    }

    auto ConsumeFallback = [&](TokenRange &tokens) -> RefPtr<CSSValue>
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
      while (auto tactic =
               ConsumeIdentRaw<ValueId::FlipBlock, ValueId::FlipInline, ValueId::FlipStart,
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

      CSSValueListBuilder list;
      if (tryRuleIdent)
      {
        list.push_back(CSSPrimitiveValue::CreateCustomIdent(*tryRuleIdent));
      }
      for (auto tactic : tryTactics)
      {
        list.push_back(CSSPrimitiveValue::Create(tactic));
      }

      // At least one @position-try rule ident or tactic must be present.
      if (list.empty())
      {
        return nullptr;
      }

      return CSSValueList::CreateSpaceSeparated(krys::move(list));
    };

    return ConsumeListSeparatedBy<',', OneOrMore, ListOptimization::SingleValue>(tokens, ConsumeFallback);
  }
}