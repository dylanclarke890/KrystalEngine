#pragma once

#include "Krystal.Lib/Pointers/IntrusivePtrV1/RefCounted.hpp"

namespace Krys::HTML
{
  class CustomElementRegistry : public RefCounted<CustomElementRegistry>
  {
  };
}