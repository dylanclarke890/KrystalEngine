#include "Krystal.HTML/Algorithms/NonElementParentNodeAlgorithms.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/SubtreeRanges.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML
{
  RawPtr<Element> NonElementParentNodeAlgorithms::GetElementById(ContainerNode &node,
                                                                 DOMStringView id) noexcept
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

  RawPtr<const Element> NonElementParentNodeAlgorithms::GetElementById(const ContainerNode &node,
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