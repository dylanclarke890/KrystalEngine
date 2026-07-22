#pragma once

#include "Krystal.Lib/Types/StronglyTypedValue.hpp"

namespace Krys::HTML
{
  struct IsSetFromShorthand : public StronglyTypedBool<IsSetFromShorthand>
  {
    using Base::Base;
  };
}