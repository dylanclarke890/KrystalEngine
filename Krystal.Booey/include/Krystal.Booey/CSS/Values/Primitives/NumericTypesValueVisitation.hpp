#pragma once

#include "Krystal.Booey/Core/IterationStatus.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Core/Types/Func.hpp"

namespace krys::boo::css
{
  // MARK: - Value Visitation

  template <NumericRaw RawType>
  struct ValueChildrenVisitor<RawType>
  {
    IterationStatus operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &, const RawType &) noexcept
    {
      return IterationStatus::Continue;
    }
  };
}
