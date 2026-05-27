#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/DOM/Event/Event.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/DOM/NodeList.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"

namespace Krys::HTML
{
  ShadowRoot::ShadowRoot(Document &document, RefPtr<Krys::HTML::CustomElementRegistry> &&registry,
                         NodeFlag flags) noexcept
      : DocumentFragment(document, flags | NodeFlag::IsShadowRoot),
        _customElementRegistry(Krys::Move(registry))
  {
    SetEventTargetFlag(EventTargetFlags::IsInShadowTree);
  }

  RawPtr<Element> ShadowRoot::Host() const noexcept
  {
    if (auto host = _host.lock())
    {
      return host.get();
    }

    return nullptr;
  }

  RawPtr<EventTarget> ShadowRoot::GetParent(Event &event) const noexcept
  {
    if (!event.Composed())
    {
      if (event._path.size() >= 1)
      {
        auto &firstItem = event._path[0];
        if (auto *invocationTargetNode = DynamicDowncast<Node>(firstItem.InvocationTarget()))
        {
          if (&TreeQueries::Root(*invocationTargetNode) == this)
          {
            return nullptr;
          }
        }
      }
    }

    if (auto host = _host.lock())
    {
      return host.get();
    }

    return nullptr;
  }
}
