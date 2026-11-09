/*
 * Copyright (C) 2017 Igalia S.L.
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Style/Values/Grid/GridPosition.hpp"
#include "Krystal.Booey/CSS/Values/GridLineValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Core/Maths/Extras.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::style
{
  namespace
  {
    Maybe<int> maxPositionForTesting;
    constexpr int GridMaxPosition = 1'000'000;

    KRYS_NODISCARD int ClampGridIntegerPosition(int integerPosition) noexcept
    {
      return ClampTo(integerPosition, GridPosition::Min(), GridPosition::Max());
    }
  }

  GridPosition::GridPosition(Explicit explicitPosition) noexcept
      : _type {GridPositionType::Explicit},
        _integerPosition {ClampGridIntegerPosition(explicitPosition.Position.Value)},
        _namedGridLine {krys::move(explicitPosition.Name)}
  {
  }

  GridPosition::GridPosition(Span spanPosition) noexcept
      : _type {GridPositionType::Span},
        _integerPosition {ClampGridIntegerPosition(spanPosition.Position.Value)},
        _namedGridLine {krys::move(spanPosition.Name)}
  {
  }

  GridPosition::GridPosition(css::CustomIdentifier namedGridAreaPosition) noexcept
      : _type {GridPositionType::NamedGridArea}, _namedGridLine {krys::move(namedGridAreaPosition)}
  {
  }

  int GridPosition::ExplicitPosition() const noexcept
  {
    krys_debug_assert(_type == GridPositionType::Explicit);
    return _integerPosition;
  }

  int GridPosition::SpanPosition() const noexcept
  {
    krys_debug_assert(_type == GridPositionType::Span);
    return _integerPosition;
  }

  css::CSSOMString GridPosition::NamedGridLine() const noexcept
  {
    krys_debug_assert(_type == GridPositionType::Explicit || _type == GridPositionType::Span
                      || _type == GridPositionType::NamedGridArea);
    return css::CSSOMString {_namedGridLine.Value.View()};
  }

  int GridPosition::Max() noexcept
  {
    return maxPositionForTesting.value_or(GridMaxPosition);
  }

  int GridPosition::Min() noexcept
  {
    return -Max();
  }

  void GridPosition::SetMaxPositionForTesting(unsigned maxPosition) noexcept
  {
    maxPositionForTesting = static_cast<int>(maxPosition);
  }

  // MARK: - Conversion

  GridPosition CSSValueConversion<GridPosition>::operator()(BuilderState &state,
                                                            const css::Value &value) noexcept
  {
    if (const auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      if (primitiveValue->ValueId() == css::ValueId::Auto)
      {
        return css::keywords::Auto {};
      }

      if (primitiveValue->IsCustomIdent())
      {
        return css::CustomIdentifier {css::CSSOMStringAtom {primitiveValue->StringValue()}};
      }

      state.SetCurrentPropertyInvalidAtComputedValueTime();
      return css::keywords::Auto {};
    }

    const auto *gridLineValue = RequiredDowncast<css::GridLineValue>(state, value);
    if (!gridLineValue)
    {
      return css::keywords::Auto {};
    }

    const auto *uncheckedSpanValue = gridLineValue->SpanValue();
    const auto *uncheckedNumericValue = gridLineValue->NumericValue();
    const auto *uncheckedGridLineName = gridLineValue->GridLineName();

    auto gridLineNumber = uncheckedNumericValue && uncheckedNumericValue->IsInteger()
                            ? uncheckedNumericValue->ResolveAsInteger<int>(state.CssToLengthConversionData())
                            : 0;
    auto gridLineName = uncheckedGridLineName && uncheckedGridLineName->IsCustomIdent()
                          ? css::CSSOMStringAtom {uncheckedGridLineName->StringValue()}
                          : css::CSSOMStringAtom::Null();

    if (uncheckedSpanValue && uncheckedSpanValue->ValueId() == css::ValueId::Span)
    {
      return GridPosition::Span {{gridLineNumber > 0 ? gridLineNumber : 1},
                                 css::CustomIdentifier {krys::move(gridLineName)}};
    }

    return GridPosition::Explicit {{gridLineNumber}, css::CustomIdentifier {krys::move(gridLineName)}};
  }
}
