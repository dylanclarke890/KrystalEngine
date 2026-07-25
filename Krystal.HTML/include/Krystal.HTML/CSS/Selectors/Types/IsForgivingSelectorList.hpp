#pragma once

#include "Krystal.Lib/Types/StronglyTypedValue.hpp"

namespace Krys::HTML
{
  struct IsForgivingSelectorList : public StronglyTypedBool<IsForgivingSelectorList>
  {
    using Base::Base;
  };
}