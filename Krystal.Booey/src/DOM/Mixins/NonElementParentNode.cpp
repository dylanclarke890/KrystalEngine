#include "Krystal.Booey/DOM/Mixins/NonElementParentNode.hpp"
#include "Krystal.Booey/DOM/Abort/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Iterators/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Nodes/Attr.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSlotElement.hpp"

namespace krys::boo::dom::mixins
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