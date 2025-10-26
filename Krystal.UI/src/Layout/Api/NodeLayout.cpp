#include "Krystal.UI/Layout/Api/NodeLayout.hpp"
#include "Krystal.UI/Layout/Node/Node.hpp"
#include "Krystal.UI/Styles/Enums/Edge.hpp"

namespace
{
  using namespace Krys::UI;

  template <auto LayoutMember>
  float GetResolvedLayoutProperty(const NodeConstRef node, const Edge edge)
  {
    assert(edge <= Edge::End && "Cannot get layout properties of multi-edge shorthands");

    if (edge == Edge::Start)
    {
      if (node->GetLayout().GetDirection() == Direction::RTL)
      {
        return (node->GetLayout().*LayoutMember)(PhysicalEdge::Right);
      }
      else
      {
        return (node->GetLayout().*LayoutMember)(PhysicalEdge::Left);
      }
    }

    if (edge == Edge::End)
    {
      if (node->GetLayout().GetDirection() == Direction::RTL)
      {
        return (node->GetLayout().*LayoutMember)(PhysicalEdge::Left);
      }
      else
      {
        return (node->GetLayout().*LayoutMember)(PhysicalEdge::Right);
      }
    }

    return (node->GetLayout().*LayoutMember)(static_cast<PhysicalEdge>(edge));
  }

}

namespace Krys::UI
{
  float NodeLayoutGetLeft(NodeConstRef node)
  {
    return node->GetLayout().GetPosition(PhysicalEdge::Left);
  }

  float NodeLayoutGetTop(NodeConstRef node)
  {
    return node->GetLayout().GetPosition(PhysicalEdge::Top);
  }

  float NodeLayoutGetRight(NodeConstRef node)
  {
    return node->GetLayout().GetPosition(PhysicalEdge::Right);
  }

  float NodeLayoutGetBottom(NodeConstRef node)
  {
    return node->GetLayout().GetPosition(PhysicalEdge::Bottom);
  }

  float NodeLayoutGetWidth(NodeConstRef node)
  {
    return node->GetLayout().GetDimension(Dimension::Width);
  }

  float NodeLayoutGetHeight(NodeConstRef node)
  {
    return node->GetLayout().GetDimension(Dimension::Height);
  }

  Direction NodeLayoutGetDirection(NodeConstRef node)
  {
    return node->GetLayout().GetDirection();
  }

  bool NodeLayoutGetHadOverflow(NodeConstRef node)
  {
    return node->GetLayout().GetHadOverflow();
  }

  float NodeLayoutGetMargin(NodeConstRef node, Edge edge)
  {
    return GetResolvedLayoutProperty<&LayoutResults::GetMargin>(node, edge);
  }

  float NodeLayoutGetBorder(NodeConstRef node, Edge edge)
  {
    return GetResolvedLayoutProperty<&LayoutResults::GetBorder>(node, edge);
  }

  float NodeLayoutGetPadding(NodeConstRef node, Edge edge)
  {
    return GetResolvedLayoutProperty<&LayoutResults::GetPadding>(node, edge);
  }

  float NodeLayoutGetRawHeight(NodeConstRef node)
  {
    return node->GetLayout().GetRawDimension(Dimension::Height);
  }

  float NodeLayoutGetRawWidth(NodeConstRef node)
  {
    return node->GetLayout().GetRawDimension(Dimension::Width);
  }
}