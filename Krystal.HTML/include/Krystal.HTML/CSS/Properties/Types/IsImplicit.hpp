#pragma once

#include "Krystal.Lib/Types/StronglyTypedValue.hpp"

namespace Krys::HTML
{
  struct IsImplicit : public StronglyTypedBool<IsImplicit>
  {
    using Base::Base;
  };
}