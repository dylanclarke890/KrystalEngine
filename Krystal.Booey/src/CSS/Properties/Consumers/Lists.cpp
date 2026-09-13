#include "Krystal.Booey/CSS/Properties/Consumers/Lists.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/CSSPrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/IntegerDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/String.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/CSSValueList.hpp"
#include "Krystal.Booey/CSS/Values/CSSValueListBuilder.hpp"
#include "Krystal.Booey/CSS/Values/CSSValuePair.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  KRYS_NODISCARD static RefPtr<CSSValue> ConsumeCounter(TokenRange &range, PropertyParserState &state,
                                                        int defaultValue) noexcept
  {
    if (range.Peek().ValueId() == ValueId::None)
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
        list.push_back(CSSValuePair::Create(krys::move(counterName), krys::move(counterValue)));
      }
      else
      {
        list.push_back(
          CSSValuePair::Create(krys::move(counterName), CSSPrimitiveValue::CreateInteger(defaultValue)));
      }
    } while (!range.IsAtEnd());

    return CSSValueList::CreateSpaceSeparated(krys::move(list));
  }

  RefPtr<CSSValue> ConsumeCounterReset(TokenRange &range, PropertyParserState &state) noexcept
  {
    // <'counter-reset'> = [ <counter-name> <integer>? | <reversed-counter-name> <integer>? ]+ | none
    // https://drafts.csswg.org/css-lists/#propdef-counter-reset

    // FIXME: Implement support for `reversed-counter-name`.

    return ConsumeCounter(range, state, 0);
  }

  RefPtr<CSSValue> ConsumeCounterIncrement(TokenRange &range, PropertyParserState &state) noexcept
  {
    // <'counter-increment'> = [ <counter-name> <integer>? ]+ | none
    // https://drafts.csswg.org/css-lists/#propdef-counter-increment

    return ConsumeCounter(range, state, 1);
  }

  RefPtr<CSSValue> ConsumeCounterSet(TokenRange &range, PropertyParserState &state) noexcept
  {
    // <'counter-set'> = [ <counter-name> <integer>? ]+ | none
    // https://drafts.csswg.org/css-lists/#propdef-counter-set

    return ConsumeCounter(range, state, 0);
  }
}