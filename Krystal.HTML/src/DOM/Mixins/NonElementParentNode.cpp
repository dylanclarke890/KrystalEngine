#include "Krystal.HTML/DOM/Mixins/NonElementParentNode.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/DOM/Element.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"

namespace Krys::HTML::Mixins
{
  RefPtr<Element> NonElementParentNode::GetElementById(ContainerNode &node, DOMStringView id) noexcept
  {
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