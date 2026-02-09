#pragma once

#include "Krystal.Lib/Pointers/RefCounted/RefCounted.hpp"

namespace Krys::HTML
{
  class CustomElementRegistry : public RefCounted<CustomElementRegistry>
  {
  };
}