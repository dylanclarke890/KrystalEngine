#include "Krystal.HTML/Node/ParentNodeRareData.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/MutationObserver/MutationObserver.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/NodeList/HTMLCollection.hpp"

namespace Krys::HTML
{
  Ref<HTMLCollection> ParentNodeRareData::Children(ContainerNode &parent) noexcept
  {
    if (!_children)
    {
      auto children = CreateRef<HTMLCollection>(parent);
      _children = CreateWeakPtr(children.get());
      return children;
    }

    return ShareRef(*_children.get());
  }

  void ParentNodeRareData::InvalidateChildren() noexcept
  {
    if (_children)
    {
      _children->Invalidate();
    }
  }
}
