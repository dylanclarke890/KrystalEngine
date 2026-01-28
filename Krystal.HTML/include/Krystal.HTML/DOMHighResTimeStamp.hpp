#pragma once

#include "Krystal.Lib/Types/StronglyTypedValue.hpp"

namespace Krys::HTML
{
  struct DOMHighResTimeStamp : public StronglyTypedNumber<DOMHighResTimeStamp, double>
  {
    using Base::StronglyTypedNumber;
  };
}