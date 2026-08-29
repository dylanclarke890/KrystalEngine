#include "Krystal.HTML/CSS/Properties/Consumers/PositionTry.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Anchor.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/List.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueList.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueListBuilder.hpp"

namespace Krys::HTML::CSSPropertyParserHelpers
{
  RefPtr<CSSValue> ConsumePositionTryFallbacks(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    // <'position-try-fallbacks'> = none | [ [<dashed-ident> || <try-tactic>] | <'position-area'> ]#
    // https://drafts.csswg.org/css-anchor-position-1/#propdef-position-try-fallbacks

    if (auto result = ConsumeIdent<CSSValueId::None>(tokens))
    {
      return result;
    }

    auto ConsumeFallback = [&](CSSTokenRange &tokens) -> RefPtr<CSSValue>
    {
      // Try to parse <'position-area'>
      auto rangeCopy = tokens;
      // consumePositionArea accepts 'none', so detect and reject it beforehand.
      if (tokens.Peek().ValueId() == CSSValueId::None)
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

      SmallList<CSSValueId, 5uz> tryTactics;
      while (auto tactic =
               ConsumeIdentRaw<CSSValueId::FlipBlock, CSSValueId::FlipInline, CSSValueId::FlipStart,
                               CSSValueId::FlipX, CSSValueId::FlipY>(tokens))
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

      return CSSValueList::CreateSpaceSeparated(Krys::Move(list));
    };

    return ConsumeListSeparatedBy<',', OneOrMore, ListOptimization::SingleValue>(tokens, ConsumeFallback);
  }
}