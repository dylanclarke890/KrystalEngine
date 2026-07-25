#pragma once

#include "Krystal.Lib/Types/StronglyTypedValue.hpp"

namespace Krys::HTML
{
  struct IsCaseSensitive : public StronglyTypedBool<IsCaseSensitive>
  {
    using Base::Base;
  };
}