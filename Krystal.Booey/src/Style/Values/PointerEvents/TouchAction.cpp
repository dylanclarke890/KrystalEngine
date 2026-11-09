/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Booey/Style/Values/PointerEvents/TouchAction.hpp"

#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"

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
