#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/CSS/ComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"

namespace krys::boo::css
{
  css::ComputedStyleDependencies Value::ComputedStyleDependencies() const noexcept
  {
    css::ComputedStyleDependencies dependencies;
    CollectComputedStyleDependencies(dependencies);
    krys_not_implemented();
    return dependencies;
  }

  ValueId Value::ValueId() const noexcept
  {
    auto *value = DynamicDowncast<PrimitiveValue>(*this);
    return value ? value->ValueId() : ValueId::Invalid;
  }
}