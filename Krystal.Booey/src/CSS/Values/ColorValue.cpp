/*
 * Copyright (C) 2024 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Booey/CSS/Values/ColorValue.hpp"
#include "Krystal.Booey/CSS/Values/Color/KeywordColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/ResolvedColor.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"

namespace krys::boo::css
{
  Ref<ColorValue> ColorValue::Create(css::Color color) noexcept
  {
    return AdoptRef(*new ColorValue(krys::move(color)));
  }

  Ref<ColorValue> ColorValue::Create(boo::Color color) noexcept
  {
    return AdoptRef(*new ColorValue(krys::move(color)));
  }

  ColorValue::ColorValue(css::Color color) noexcept : css::Value(ValueType::Color), _color(krys::move(color))
  {
  }

  ColorValue::ColorValue(boo::Color color) noexcept
      : ColorValue(css::Color {css::ResolvedColor {krys::move(color)}})
  {
  }

  ColorValue::ColorValue(StaticValueTag, boo::Color color) noexcept : ColorValue(krys::move(color))
  {
    // FIXME: Make static values like upstream (a reference count reserved for static values that are never
    // destroyed). For now, the extra reference is never released.
    AddRef();
  }

  boo::Color ColorValue::GetAbsoluteColor(const Value &value) noexcept
  {
    if (auto *color = DynamicDowncast<ColorValue>(value))
    {
      return color->GetColor().GetAbsoluteColor();
    }

    if (auto valueId = value.ValueId(); IsAbsoluteColorKeyword(valueId))
    {
      return ColorFromAbsoluteKeyword(valueId);
    }
    return {};
  }

  CSSOMString ColorValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    return css::Serialise(context, _color);
  }

  bool ColorValue::Equals(const ColorValue &other) const noexcept
  {
    return _color == other._color;
  }

  IterationStatus ColorValue::CustomVisitChildren(
    KRYS_NOESCAPE const Func<IterationStatus(css::Value &)> &func) const noexcept
  {
    return css::VisitValueChildren(func, _color);
  }
}
