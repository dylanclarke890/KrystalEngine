#include "Krystal.Booey/Style/Values/Text/TextAutospace.hpp"

#include "Krystal.Booey/Style/BuilderChecking.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  TextAutospace CSSValueConversion<TextAutospace>::operator()(BuilderState &state,
                                                              const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::Normal:           return css::keywords::Normal {};
        case css::ValueId::Auto:             return css::keywords::Auto {};
        case css::ValueId::NoAutospace:      return css::keywords::NoAutospace {};
        case css::ValueId::IdeographAlpha:   return css::keywords::IdeographAlpha {};
        case css::ValueId::IdeographNumeric: return css::keywords::IdeographNumeric {};
        default:                             state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::NoAutospace {};
      }
    }

    auto list = RequiredListDowncast<css::ValueList, css::PrimitiveValue>(state, value);
    if (!list)
    {
      return css::keywords::NoAutospace {};
    }

    if (list->size() == 1)
    {
      switch (const auto &first = list->item(0); first.ValueId())
      {
        case css::ValueId::Normal:           return css::keywords::Normal {};
        case css::ValueId::Auto:             return css::keywords::Auto {};
        case css::ValueId::NoAutospace:      return css::keywords::NoAutospace {};
        case css::ValueId::IdeographAlpha:   return css::keywords::IdeographAlpha {};
        case css::ValueId::IdeographNumeric: return css::keywords::IdeographNumeric {};
        default:                             state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::NoAutospace {};
      }
    }
    if (list->size() == 2)
    {
      switch (const auto &first = list->item(0); first.ValueId())
      {
        case css::ValueId::IdeographAlpha:
          if (const auto &second = list->item(1); second.ValueId() == css::ValueId::IdeographNumeric)
          {
            return {css::keywords::IdeographAlpha {}, css::keywords::IdeographNumeric {}};
          }
          break;
        case css::ValueId::IdeographNumeric:
          if (const auto &second = list->item(1); second.ValueId() == css::ValueId::IdeographAlpha)
          {
            return {css::keywords::IdeographAlpha {}, css::keywords::IdeographNumeric {}};
          }
          break;
        default: break;
      }
      state.SetCurrentPropertyInvalidAtComputedValueTime();
      return css::keywords::NoAutospace {};
    }

    state.SetCurrentPropertyInvalidAtComputedValueTime();
    return css::keywords::NoAutospace {};
  }
}
