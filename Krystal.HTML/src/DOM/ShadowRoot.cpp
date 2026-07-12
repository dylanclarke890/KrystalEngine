#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/Event.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"

namespace Krys::HTML
{
  ShadowRoot::ShadowRoot(Document &document, RefPtr<Krys::HTML::CustomElementRegistry> &&registry,
                         NodeFlags flags) noexcept
      : DocumentFragment(document, flags | NodeFlags::IsShadowRoot),
        _customElementRegistry(Krys::Move(registry))
  {
    SetNodeFlag(NodeFlags::IsInShadowTree);
  }

  RawPtr<Element> ShadowRoot::Host() const noexcept
  {
    if (auto host = _host.lock())
    {
      return host.get();
    }

    return nullptr;
  }

#pragma region DocumentOrShadowRoot Mixin - https://dom.spec.whatwg.org/#mixin-documentorshadowroot

  RefPtr<CustomElementRegistry> ShadowRoot::CustomElementRegistry() const noexcept
  {
    return _customElementRegistry;
  }

#pragma endregion

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
