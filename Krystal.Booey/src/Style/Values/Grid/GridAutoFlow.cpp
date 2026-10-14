#include "Krystal.Booey/Style/Values/Grid/GridAutoFlow.hpp"

#include "Krystal.Booey/Style/BuilderChecking.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  GridAutoFlow CSSValueConversion<GridAutoFlow>::operator()(BuilderState &state,
                                                            const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::Normal: return css::keywords::Normal {};
        case css::ValueId::Row:    return css::keywords::Row {};
        case css::ValueId::Column: return css::keywords::Column {};
        case css::ValueId::Dense:  return css::keywords::Dense {};
        default:                   state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Row {};
      }
    }

    auto list = RequiredListDowncast<css::ValueList, css::PrimitiveValue, 1>(state, value);
    if (!list)
    {
      return css::keywords::Normal {};
    }

    const auto &first = list->item(0);
    switch (first.ValueId())
    {
      case css::ValueId::Row:
        if (list->size() == 2)
        {
          const auto &second = list->item(1);
          switch (second.ValueId())
          {
            case css::ValueId::Dense: return {css::keywords::Row {}, css::keywords::Dense {}};
            default:                  state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Row {};
          }
        }
        return css::keywords::Row {};
      case css::ValueId::Column:
        if (list->size() == 2)
        {
          const auto &second = list->item(1);
          switch (second.ValueId())
          {
            case css::ValueId::Dense: return {css::keywords::Column {}, css::keywords::Dense {}};
            default:                  state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Column {};
          }
        }
        return css::keywords::Column {};
      case css::ValueId::Dense:
        if (list->size() == 2)
        {
          const auto &second = list->item(1);
          switch (second.ValueId())
          {
            case css::ValueId::Row:    return {css::keywords::Row {}, css::keywords::Dense {}};
            case css::ValueId::Column: return {css::keywords::Column {}, css::keywords::Dense {}};
            default:                   state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Dense {};
          }
        }
        return css::keywords::Dense {};
      case css::ValueId::Normal:
        if (list->size() != 1)
        {
          state.SetCurrentPropertyInvalidAtComputedValueTime();
        }
        return css::keywords::Normal {};
      default: state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Row {};
    }
  }
}
