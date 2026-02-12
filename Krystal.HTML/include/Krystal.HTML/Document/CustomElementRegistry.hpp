#pragma once

#include "Krystal.Lib/Mixins/RefCounted.hpp"

namespace Krys::HTML
{
  class CustomElementRegistry : public RefCounted<CustomElementRegistry>
  {
  };
}