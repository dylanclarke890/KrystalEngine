#include "Krystal.Booey/Animation/CompositeOperation.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo
{
  Maybe<CompositeOperation> ToCompositeOperation(const css::Value &value) noexcept
  {
    if (!Is<css::PrimitiveValue>(value))
    {
      return null;
    }

    switch (value.ValueId())
    {
      case css::ValueId::Add:        return CompositeOperation::Add;
      case css::ValueId::Accumulate: return CompositeOperation::Accumulate;
      case css::ValueId::Replace:    return CompositeOperation::Replace;
      default:                       return null;
    }
  }
}
