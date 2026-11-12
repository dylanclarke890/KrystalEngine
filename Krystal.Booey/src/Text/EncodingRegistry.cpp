#include "Krystal.Booey/Text/EncodingRegistry.hpp"
#include "Krystal.Core/Text/Encodings/UTF.hpp"

namespace krys::boo
{
  const krys::text::EncodingRegistry &EncodingRegistry() noexcept
  {
    static krys::text::EncodingRegistry registry = []() -> krys::text::EncodingRegistry
    {
      krys::text::EncodingRegistry er;

      // Documents are UTF-8 only (see DIFFERENCES.md); other encodings register here if they come.
      er.Register<krys::text::UTF8>();

      return er;
    }();

    return registry;
  }
}