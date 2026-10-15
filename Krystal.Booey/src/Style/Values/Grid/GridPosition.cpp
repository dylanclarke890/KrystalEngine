#include "Krystal.Booey/Style/Values/Grid/GridPosition.hpp"
#include "Krystal.Booey/CSS/Values/GridLineValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Core/Maths/Base.hpp"
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
