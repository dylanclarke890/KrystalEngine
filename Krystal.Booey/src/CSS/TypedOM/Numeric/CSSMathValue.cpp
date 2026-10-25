#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSMathValue.hpp"
#include "Krystal.Booey/CSS/Calc/Tree.hpp"
#include "Krystal.Booey/CSS/Calc/Value.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo::css
{
  RefPtr<css::Value> CSSMathValue::ToCSSValue() const noexcept
  {
    auto node = ToCalcTreeNode();
    if (!node)
    {
      return nullptr;
    }

    auto type = calc::GetType(*node);
    auto category = type.CalculationCategory();
    if (!category)
    {
      return nullptr;
    }

    return PrimitiveValue::Create(calc::Value::Create(*category, All,
                                                      calc::Tree {
                                                        .Root = krys::move(*node),
                                                        .Type = type,
                                                        .Stage = calc::Stage::Specified,
                                                      }));
  }
}
