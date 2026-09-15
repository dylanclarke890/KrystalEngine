#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/CSS/ComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo::css
{
  css::ComputedStyleDependencies Value::ComputedStyleDependencies() const noexcept
  {
    css::ComputedStyleDependencies dependencies;
    krys_not_implemented();
    // CollectComputedStyleDependencies(dependencies);
    return dependencies;
  }

  ValueId Value::ValueId() const noexcept
  {
    auto *value = DynamicDowncast<PrimitiveValue>(*this);
    return value ? value->ValueId() : ValueId::Invalid;
  }
}