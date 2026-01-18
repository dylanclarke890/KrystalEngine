#pragma once

#include "Krystal.Lib/Types/StronglyTypedValue.hpp"

namespace Krys::HTML
{
  struct IsFinalChunk : StronglyTypedBool<IsFinalChunk>
  {
    using StronglyTypedBool::StronglyTypedBool;
  };
}