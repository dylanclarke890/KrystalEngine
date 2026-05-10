#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"

namespace Krys::HTML
{
  ShadowRoot::ShadowRoot(Document &document, RefPtr<Krys::HTML::CustomElementRegistry> &&registry,
                         NodeFlag flags) noexcept
      : DocumentFragment(document, flags | NodeFlag::IsShadowRoot),
        _customElementRegistry(Krys::Move(registry))
  {
  }
}
