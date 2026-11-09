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

#include "Krystal.Booey/Style/Values/Text/TextAlign.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"

#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueConversion.hpp"

namespace krys::boo::style
{
  TextAlign CSSValueConversion<TextAlign>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return TextAlign::Start;
    }

    if (!primitiveValue->IsValueId())
    {
      state.SetCurrentPropertyInvalidAtComputedValueTime();
      return TextAlign::Start;
    }

    auto &parentStyle = state.ParentStyle();

    // User agents are expected to have a rule in their user agent stylesheet that matches 'th' elements that
    // have a parent node whose computed value for the 'text-align' property is its initial value, whose
    // declaration block consists of just a single declaration that sets the 'text-align' property to the
    // value 'center'. https://html.spec.whatwg.org/multipage/rendering.html#rendering
    if (primitiveValue->ValueId() == css::ValueId::InternalThCenter)
    {
      if (parentStyle.GetTextAlign() == TextAlign::Start)
      {
        return TextAlign::Center;
      }
      return parentStyle.GetTextAlign();
    }

    if (primitiveValue->ValueId() == css::ValueId::MatchParent)
    {
      const auto *element = state.GetElement();

      if (element && element == state.GetDocument().DocumentElement().get())
      {
        return TextAlign::Start;
      }
      if (parentStyle.GetTextAlign() == TextAlign::Start)
      {
        return parentStyle.GetWritingMode().IsBidiLTR() ? TextAlign::Left : TextAlign::Right;
      }
      if (parentStyle.GetTextAlign() == TextAlign::End)
      {
        return parentStyle.GetWritingMode().IsBidiLTR() ? TextAlign::Right : TextAlign::Left;
      }

      return parentStyle.GetTextAlign();
    }

    return FromCSSValue<TextAlign>(value);
  }
}
