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
