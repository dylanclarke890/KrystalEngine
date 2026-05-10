#include "Krystal.HTML/Algorithms/Mixins/NonElementParentNode.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Utils/SubtreeRanges.hpp"

namespace Krys::HTML::Mixins
{
  RawPtr<Element> NonElementParentNode::GetElementById(ContainerNode &node, DOMStringView id) noexcept
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

    return it != std::ranges::end(descendants) ? DynamicDowncast<Element>(&*it) : nullptr;
  }

  RawPtr<const Element> NonElementParentNode::GetElementById(const ContainerNode &node,
                                                             DOMStringView id) noexcept
  {
    auto descendants = ConstDescendantRange(node);

    auto it = First(descendants,
                    [id](auto &&descendant)
                    {
                      if (auto *element = DynamicDowncast<const Element>(descendant))
                      {
                        return element->Id() == id;
                      }
                      return false;
                    });

    return it != std::ranges::end(descendants) ? DynamicDowncast<const Element>(&*it) : nullptr;
  }
}