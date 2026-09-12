#pragma once

#include "Krystal.Core/Types/StronglyTypedValue.hpp"

namespace krys::boo::css
{
  struct MatchLowercase : public StronglyTypedBool<MatchLowercase>
  {
    using Base::Base;
  };
}