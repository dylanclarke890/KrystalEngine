#pragma once

#include "Krystal.Core/Types/StronglyTypedValue.hpp"

namespace krys::boo::css
{
  struct IsSetFromShorthand : public StronglyTypedBool<IsSetFromShorthand>
  {
    using Base::Base;
  };
}