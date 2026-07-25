#pragma once

#include "Krystal.Lib/Types/StronglyTypedValue.hpp"

namespace Krys::HTML
{
  struct MatchLowercase : public StronglyTypedBool<MatchLowercase>
  {
    using Base::Base;
  };
}