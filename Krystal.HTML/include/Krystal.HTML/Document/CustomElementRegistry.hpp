#pragma once

#include "Krystal.Lib/Pointers/RefCounted.hpp"

namespace Krys::HTML
{
  class CustomElementRegistry : public RefCounted<CustomElementRegistry>
  {
  };
}