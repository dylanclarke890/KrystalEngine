#include "Krystal.HTML/Node/RareData/ParentNodeRareData.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/MutationObserver/MutationObserver.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/DOM/HTMLCollection.hpp"
#include "Krystal.HTML/DOM/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML
{
  Ref<HTMLCollection> ParentNodeRareData::Children(ContainerNode &parent) noexcept
  {
    if (auto children = _children.lock())
    {
      return ShareRef(*children.get());
    }

    auto children =
      CreateRef<LiveHTMLCollection>(parent, [&](const Element &n) { return n.ParentNode() == &parent; });
    _children = CreateWeakPtr(children.get());

    return children;
  }
}
