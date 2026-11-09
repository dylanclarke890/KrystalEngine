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

#include "Krystal.Booey/CSS/Values/PositionValue.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"

namespace krys::boo::css
{
  Ref<PositionValue> PositionValue::Create(Position position) noexcept
  {
    return AdoptRef(*new PositionValue(krys::move(position)));
  }

  PositionValue::PositionValue(Position &&position) noexcept
      : Value(ValueType::Position), _position(krys::move(position))
  {
  }

  CSSOMString PositionValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    return Serialise(context, _position);
  }

  bool PositionValue::Equals(const PositionValue &other) const noexcept
  {
    return _position == other._position;
  }

  IterationStatus PositionValue::CustomVisitChildren(
    KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
  {
    return VisitValueChildren(func, _position);
  }

  // MARK: PositionX

  Ref<PositionXValue> PositionXValue::Create(PositionX position) noexcept
  {
    return AdoptRef(*new PositionXValue(krys::move(position)));
  }

  PositionXValue::PositionXValue(PositionX &&position) noexcept
      : Value(ValueType::PositionX), _position(krys::move(position))
  {
  }

  CSSOMString PositionXValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    return Serialise(context, _position);
  }

  bool PositionXValue::Equals(const PositionXValue &other) const noexcept
  {
    return _position == other._position;
  }

  IterationStatus PositionXValue::CustomVisitChildren(
    KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
  {
    return VisitValueChildren(func, _position);
  }

  // MARK: PositionY

  Ref<PositionYValue> PositionYValue::Create(PositionY position) noexcept
  {
    return AdoptRef(*new PositionYValue(krys::move(position)));
  }

  PositionYValue::PositionYValue(PositionY &&position) noexcept
      : Value(ValueType::PositionY), _position(krys::move(position))
  {
  }

  CSSOMString PositionYValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    return Serialise(context, _position);
  }

  bool PositionYValue::Equals(const PositionYValue &other) const noexcept
  {
    return _position == other._position;
  }

  IterationStatus PositionYValue::CustomVisitChildren(
    KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
  {
    return VisitValueChildren(func, _position);
  }
}
