#include "Krystal.UI.Layout/Algorithm/Baseline.hpp"
#include "Krystal.UI.Layout/Algorithm/Align.hpp"
#include "Krystal.UI.Layout/Event/Event.hpp"

namespace Krys::UI::Layout
{
  float CalculateBaseline(const Node *node)
  {
    if (node->HasBaselineFunc())
    {
      Event::Publish<Event::NodeBaselineStart>(node);

      const float baseline = node->Baseline(node->GetLayout().GetMeasuredDimension(Dimension::Width),
                                            node->GetLayout().GetMeasuredDimension(Dimension::Height));

      Event::Publish<Event::NodeBaselineEnd>(node);

      if (std::isnan(baseline))
      {
        throw std::runtime_error("A custom baseline function returned NaN");
      }

      return baseline;
    }

    Node *baselineChild = nullptr;
    for (auto child : node->GetLayoutChildren())
    {
      if (child->GetLineIndex() > 0)
      {
        break;
      }
      if (child->GetStyle().GetPositionType() == PositionType::Absolute)
      {
        continue;
      }
      if (ResolveChildAlignment(node, child) == Align::Baseline || child->IsReferenceBaseline())
      {
        baselineChild = child;
        break;
      }

      if (baselineChild == nullptr)
      {
        baselineChild = child;
      }
    }

    if (baselineChild == nullptr)
    {
      return node->GetLayout().GetMeasuredDimension(Dimension::Height);
    }

    const float baseline = CalculateBaseline(baselineChild);
    return baseline + baselineChild->GetLayout().GetPosition(PhysicalEdge::Top);
  }

  bool IsBaselineLayout(const Node *node)
  {
    if (IsColumn(node->GetStyle().GetFlexDirection()))
    {
      return false;
    }
    if (node->GetStyle().GetAlignItems() == Align::Baseline)
    {
      return true;
    }
    for (auto child : node->GetLayoutChildren())
    {
      if (child->GetStyle().GetPositionType() != PositionType::Absolute
          && child->GetStyle().GetAlignSelf() == Align::Baseline)
      {
        return true;
      }
    }

    return false;
  }
}