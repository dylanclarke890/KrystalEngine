#pragma once

#include "Krystal.Core/Types/StronglyTypedValue.hpp"

namespace krys::boo::css
{
  struct IsImplicit : public StronglyTypedBool<IsImplicit>
  {
    using Base::Base;
  };
}