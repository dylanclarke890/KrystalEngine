#include "Krystal.Booey/DOM/FragmentDirective/FragmentDirectiveUtilities.hpp"
#include "Krystal.Booey/DOM/Nodes/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::dom
{
  ContainerNode &FragmentDirectiveUtilities::NearestBlockAncestor(Node &node) noexcept
  {
    for (auto *currentNode = &node; currentNode; currentNode = currentNode->ParentNode())
    {
      if (auto *renderElement = DynamicDowncast<RenderElement>(currentNode->Renderer());
          renderElement && renderElement->Style().IsDisplayBlockLevel())
      {
        return Downcast<ContainerNode>(*currentNode);
      }
    }
    return node.NodeDocument();
  }
}
