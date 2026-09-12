#pragma once

#include "Krystal.Core/Types/StronglyTypedValue.hpp"

namespace krys::boo::css
{
  struct IsForgivingSelectorList : public StronglyTypedBool<IsForgivingSelectorList>
  {
    using Base::Base;
  };
}