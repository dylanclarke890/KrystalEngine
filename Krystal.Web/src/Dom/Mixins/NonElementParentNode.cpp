#include "Krystal.Web/Dom/Mixins/NonElementParentNode.hpp"
#include "Krystal.Web/Dom/Internals/Iterators/SubtreeRanges.hpp"
#include "Krystal.Web/Dom/Nodes/Attr.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Web/Html/Elements/HTMLSlotElement.hpp"

namespace krys::web::dom::mixins
{
  RefPtr<Element> NonElementParentNode::GetElementById(ContainerNode &node, DOMStringView id) noexcept
  {
    // No element has the empty string as its ID.
    if (id.empty())
    {
      return nullptr;
    }

    // The root of a tree scope maps its IDs; a fragment or an element has to look.
    if (Is<Document>(node) || Is<ShadowRoot>(node))
    {
      return ShareRefPtr(node.TreeScope().FirstElementWithId(id));
    }

    auto descendants = DescendantRange(node);

    auto it = First(descendants,
                    [id](auto &&descendant)
                    {
                      if (auto *element = DynamicDowncast<Element>(descendant))
                      {
                        return element->Id() == id;
                      }
                      return false;
                    });

    return it != std::ranges::end(descendants) ? ShareRefPtr(DynamicDowncast<Element>(&*it)) : nullptr;
  }

  RefPtr<const Element> NonElementParentNode::GetElementById(const ContainerNode &node,
                                                             DOMStringView id) noexcept
  {
    return GetElementById(const_cast<ContainerNode &>(node), id);
  }
}
