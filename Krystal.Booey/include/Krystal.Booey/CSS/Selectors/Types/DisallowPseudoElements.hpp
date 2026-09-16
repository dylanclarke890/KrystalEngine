#pragma once

#include "Krystal.Core/Types/StronglyTypedValue.hpp"

namespace krys::boo::css
{
  struct DisallowPseudoElements : public StronglyTypedBool<DisallowPseudoElements>
  {
    using Base::Base;
  };
}