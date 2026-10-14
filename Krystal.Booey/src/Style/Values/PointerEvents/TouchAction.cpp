#include "Krystal.Booey/Style/Values/PointerEvents/TouchAction.hpp"

#include "Krystal.Booey/Style/BuilderChecking.hpp"

namespace krys::boo::style
{
  TouchAction CSSValueConversion<TouchAction>::operator()(BuilderState &state,
                                                          const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::Auto:         return css::keywords::Auto {};
        case css::ValueId::None:         return css::keywords::None {};
        case css::ValueId::Manipulation: return css::keywords::Manipulation {};
        case css::ValueId::PanX:         return TouchActionValue::PanX;
        case css::ValueId::PanY:         return TouchActionValue::PanY;
        case css::ValueId::PinchZoom:    return TouchActionValue::PinchZoom;
        default:                         state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Auto {};
      }
    }

    auto list = RequiredListDowncast<css::ValueList, css::PrimitiveValue>(state, value);
    if (!list)
    {
      return css::keywords::Auto {};
    }

    TouchActionValueEnumSet result;
    for (const auto &item : *list)
    {
      switch (item.ValueId())
      {
        case css::ValueId::PanX:      result.Value.Add(TouchActionValue::PanX); break;
        case css::ValueId::PanY:      result.Value.Add(TouchActionValue::PanY); break;
        case css::ValueId::PinchZoom: result.Value.Add(TouchActionValue::PinchZoom); break;
        default:                      state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Auto {};
      }
    }
    return result;
  }

  // MARK: - Platform

  EnumSet<boo::TouchAction> ToPlatform<TouchAction>::operator()(const TouchAction &value) noexcept
  {
    return krys::SwitchOn(
      value, [](const css::keywords::Auto &) -> EnumSet<boo::TouchAction> { return boo::TouchAction::Auto; },
      [](const css::keywords::None &) -> EnumSet<boo::TouchAction> { return boo::TouchAction::None; },
      [](const css::keywords::Manipulation &) -> EnumSet<boo::TouchAction>
      { return boo::TouchAction::Manipulation; },
      [](const TouchActionValueEnumSet &set) -> EnumSet<boo::TouchAction>
      {
        EnumSet<boo::TouchAction> result;
        for (auto action : set)
        {
          switch (action)
          {
            case TouchActionValue::PanX:      result.Add(boo::TouchAction::PanX); break;
            case TouchActionValue::PanY:      result.Add(boo::TouchAction::PanY); break;
            case TouchActionValue::PinchZoom: result.Add(boo::TouchAction::PinchZoom); break;
          }
        }
        return result;
      });
  }
}
