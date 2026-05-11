#include "Krystal.HTML/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"

namespace Krys::HTML
{
  bool CustomElementAlgorithms::IsGlobalCustomElementRegistry(
    RawPtr<const CustomElementRegistry> registry) noexcept
  {
    return registry != nullptr && !registry->IsScoped();
  }
}