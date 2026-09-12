#pragma once

#include "Krystal.Core/Types/StronglyTypedValue.hpp"

namespace krys::boo::css
{
  struct IsCaseSensitive : public StronglyTypedBool<IsCaseSensitive>
  {
    using Base::Base;
  };
}