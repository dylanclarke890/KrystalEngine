#include "Krystal.HTML/CSS/Properties/Consumers/Lists.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/IntegerDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/String.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueList.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueListBuilder.hpp"
#include "Krystal.HTML/CSS/Values/CSSValuePair.hpp"

namespace Krys::HTML::CSSPropertyParserHelpers
{
  KRYS_NODISCARD static RefPtr<CSSValue> ConsumeCounter(CSSTokenRange &range, CSSPropertyParserState &state,
                                                        int defaultValue) noexcept
  {
    if (range.Peek().ValueId() == CSSValueId::None)
    {
      return ConsumeIdent(range);
    }

    CSSValueListBuilder list;
    do
    {
      auto counterName = ConsumeCustomIdent(range);
      if (!counterName)
      {
        return nullptr;
      }

      if (auto counterValue = CSSPrimitiveValueResolver<Integer<>>::ConsumeAndResolve(range, state))
      {
        list.push_back(CSSValuePair::Create(Krys::Move(counterName), Krys::Move(counterValue)));
      }
      else
      {
        list.push_back(
          CSSValuePair::Create(Krys::Move(counterName), CSSPrimitiveValue::CreateInteger(defaultValue)));
      }
    } while (!range.IsAtEnd());

    return CSSValueList::CreateSpaceSeparated(Krys::Move(list));
  }

  RefPtr<CSSValue> ConsumeCounterReset(CSSTokenRange &range, CSSPropertyParserState &state) noexcept
  {
    // <'counter-reset'> = [ <counter-name> <integer>? | <reversed-counter-name> <integer>? ]+ | none
    // https://drafts.csswg.org/css-lists/#propdef-counter-reset

    // FIXME: Implement support for `reversed-counter-name`.

    return ConsumeCounter(range, state, 0);
  }

  RefPtr<CSSValue> ConsumeCounterIncrement(CSSTokenRange &range, CSSPropertyParserState &state) noexcept
  {
    // <'counter-increment'> = [ <counter-name> <integer>? ]+ | none
    // https://drafts.csswg.org/css-lists/#propdef-counter-increment

    return ConsumeCounter(range, state, 1);
  }

  RefPtr<CSSValue> ConsumeCounterSet(CSSTokenRange &range, CSSPropertyParserState &state) noexcept
  {
    // <'counter-set'> = [ <counter-name> <integer>? ]+ | none
    // https://drafts.csswg.org/css-lists/#propdef-counter-set

    return ConsumeCounter(range, state, 0);
  }
}