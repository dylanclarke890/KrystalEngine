#include "Krystal.HTML/Document/ShadowRoot.hpp"
#include "Krystal.HTML/Document/CustomElementRegistry.hpp"
#include "Krystal.HTML/Document/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"

namespace Krys::HTML
{
  ShadowRoot::ShadowRoot(Document &document, RefPtr<CustomElementRegistry> &&registry,
                         NodeFlag flags) noexcept
      : DocumentFragment(document, flags | NodeFlag::IsShadowRoot),
        TreeScope(*this, document, Krys::Move(registry)), _host(nullptr)
  {
  }
}
