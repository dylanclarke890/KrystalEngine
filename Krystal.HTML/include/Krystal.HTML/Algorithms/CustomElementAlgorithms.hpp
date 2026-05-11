#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"

namespace Krys::HTML
{
  class CustomElementRegistry;

  class CustomElementAlgorithms
  {
  public:
    KRYS_NODISCARD static bool
      IsGlobalCustomElementRegistry(RawPtr<const CustomElementRegistry> registry) noexcept;
  };
}