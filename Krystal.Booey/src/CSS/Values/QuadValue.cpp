#include "Krystal.Booey/CSS/Values/QuadValue.hpp"

namespace krys::boo::css
{
  QuadValue::QuadValue(Quad quad) noexcept : Value(ValueType::Quad), _quad(krys::move(quad))
  {
  }

  Ref<QuadValue> QuadValue::Create(Quad quad) noexcept
  {
    return AdoptRef(*new QuadValue(krys::move(quad)));
  }

  CSSOMString QuadValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    return _quad.CssText(context);
  }

  bool QuadValue::Equals(const QuadValue &other) const noexcept
  {
    return _quad.Equals(other._quad);
  }

  bool QuadValue::CanBeCoalesced() const noexcept
  {
    const Value &top = _quad.Top();
    return _coalesceIdenticalValues && top.Equals(_quad.Right()) && top.Equals(_quad.Left())
           && top.Equals(_quad.Bottom());
  }
}
