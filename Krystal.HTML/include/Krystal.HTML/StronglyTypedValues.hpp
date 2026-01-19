#pragma once

#include "Krystal.Lib/Types/StronglyTypedValue.hpp"

namespace Krys::HTML
{
  struct IsEOF : StronglyTypedBool<IsEOF>
  {
    using StronglyTypedBool::StronglyTypedBool;
  };
}