#include "Krystal.Booey/CSS/Values/BasicShapeValue.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"

namespace krys::boo::css
{
  CSSOMString BasicShapeValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    return css::Serialise(context, _shape);
  }

  bool BasicShapeValue::Equals(const BasicShapeValue &other) const noexcept
  {
    return _shape == other._shape;
  }

  IterationStatus
    BasicShapeValue::CustomVisitChildren(const Func<IterationStatus(css::Value &)> &func) const noexcept
  {
    return css::VisitValueChildren(func, _shape);
  }
}