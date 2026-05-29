#include "Krystal.HTML/DOM/Internals/RareData/ParentNodeRareData.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Internals/HTMLCollection/LiveHTMLCollection.hpp"
#include "Krystal.HTML/DOM/HTMLCollection.hpp"
#include "Krystal.HTML/DOM/NodeList.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/Element.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"

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
