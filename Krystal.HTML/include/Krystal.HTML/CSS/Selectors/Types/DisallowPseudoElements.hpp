#pragma once

#include "Krystal.Lib/Types/StronglyTypedValue.hpp"

namespace Krys::HTML
{
  struct DisallowPseudoElements : public StronglyTypedBool<DisallowPseudoElements>
  {
    using Base::Base;
  };
}