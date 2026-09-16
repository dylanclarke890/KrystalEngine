#include "Krystal.Booey/DOM/Internals/RareData/ParentNodeRareData.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/HTMLCollection.hpp"
#include "Krystal.Booey/DOM/Internals/HTMLCollection/LiveHTMLCollection.hpp"
#include "Krystal.Booey/DOM/NodeList.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"

namespace krys::boo::dom
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
