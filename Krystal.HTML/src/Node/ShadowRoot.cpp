#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
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
