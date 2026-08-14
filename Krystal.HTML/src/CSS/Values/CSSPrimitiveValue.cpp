#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"

namespace Krys::HTML
{
  CSSPrimitiveValue::CSSPrimitiveValue(CSSValueId valueId) noexcept
      : CSSValue(CSSValueType::Primitive)
  {
    _unit = CSSUnitType::ValueId;
    _value.ValueId = valueId;
  }

  CSSPrimitiveValue::CSSPrimitiveValue(CSSOMString customIdentifier) noexcept
      : CSSValue(CSSValueType::Primitive)
  {
    _unit = CSSUnitType::CustomIdent;

    auto atom = CSSOMStringAtom(customIdentifier);
    _value.String = *reinterpret_cast<const StringAtomStorage *>(atom.View().data());
  }

  CSSPrimitiveValue::CSSPrimitiveValue(double value, CSSUnitType unit) noexcept
      : CSSValue(CSSValueType::Primitive)
  {
    _unit = unit;
    _value.Number = value;
  }
}