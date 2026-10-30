#include "Krystal.Booey/Style/Values/TextDecoration/TextEmphasisPosition.hpp"

#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"

namespace krys::boo::style
{
  TextEmphasisPosition CSSValueConversion<TextEmphasisPosition>::operator()(BuilderState &state,
                                                                            const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::Over:  return {TextEmphasisPositionValue::Over, TextEmphasisPositionValue::Right};
        case css::ValueId::Under: return {TextEmphasisPositionValue::Under, TextEmphasisPositionValue::Right};
        default:
          state.SetCurrentPropertyInvalidAtComputedValueTime();
          return {TextEmphasisPositionValue::Over, TextEmphasisPositionValue::Right};
      }
    }

    auto list = RequiredListDowncast<css::ValueList, css::PrimitiveValue>(state, value);
    if (!list)
    {
      return {TextEmphasisPositionValue::Over, TextEmphasisPositionValue::Right};
    }

    TextEmphasisPositionValueEnumSet result;
    for (const auto &item : *list)
    {
      switch (item.ValueId())
      {
        case css::ValueId::Over:
          if (result.Contains(TextEmphasisPositionValue::Under))
          {
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return {TextEmphasisPositionValue::Over, TextEmphasisPositionValue::Right};
          }
          result.Value.Add(TextEmphasisPositionValue::Over);
          break;
        case css::ValueId::Under:
          if (result.Contains(TextEmphasisPositionValue::Over))
          {
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return {TextEmphasisPositionValue::Over, TextEmphasisPositionValue::Right};
          }
          result.Value.Add(TextEmphasisPositionValue::Under);
          break;
        case css::ValueId::Left:
          if (result.Contains(TextEmphasisPositionValue::Right))
          {
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return {TextEmphasisPositionValue::Over, TextEmphasisPositionValue::Right};
          }
          result.Value.Add(TextEmphasisPositionValue::Left);
          break;
        case css::ValueId::Right:
          if (result.Contains(TextEmphasisPositionValue::Left))
          {
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return {TextEmphasisPositionValue::Over, TextEmphasisPositionValue::Right};
          }
          result.Value.Add(TextEmphasisPositionValue::Right);
          break;
        default:
          state.SetCurrentPropertyInvalidAtComputedValueTime();
          return {TextEmphasisPositionValue::Over, TextEmphasisPositionValue::Right};
      }
    }

    // Result must contain either `over` or `under`.
    if (!result.ContainsAny({TextEmphasisPositionValue::Over, TextEmphasisPositionValue::Under}))
    {
      state.SetCurrentPropertyInvalidAtComputedValueTime();
      return {TextEmphasisPositionValue::Over, TextEmphasisPositionValue::Right};
    }

    // If neither `left` nor `right` has been specified, `right` is added as the default.
    if (!result.ContainsAny({TextEmphasisPositionValue::Left, TextEmphasisPositionValue::Right}))
    {
      result.Value.Add(TextEmphasisPositionValue::Right);
    }

    return result;
  }
}
