#include "Krystal.UI/Algorithm/PixelGrid.hpp"
#include "Krystal.Lib/ComparisonHelpers.hpp"

namespace Krys::UI
{
  float RoundValueToPixelGrid(const double value, const double pointScaleFactor, const bool forceCeil,
                              const bool forceFloor)
  {
    double scaledValue = value * pointScaleFactor;
    // We want to calculate `fractial` such that `floor(scaledValue) = scaledValue - fractial`.
    double fractial = fmod(scaledValue, 1.0);
    if (fractial < 0)
    {
      // This branch is for handling negative numbers for `value`.
      //
      // Regarding `floor` and `ceil`. Note that for a number x, `floor(x) <= x <=
      // ceil(x)` even for negative numbers. Here are a couple of examples:
      //   - x =  2.2: floor( 2.2) =  2, ceil( 2.2) =  3
      //   - x = -2.2: floor(-2.2) = -3, ceil(-2.2) = -2
      //
      // Regarding `fmodf`. For fractional negative numbers, `fmodf` returns a
      // negative number. For example, `fmodf(-2.2) = -0.2`. However, we want
      // `fractial` to be the number such that subtracting it from `value` will
      // give us `floor(value)`. In the case of negative numbers, adding 1 to
      // `fmodf(value)` gives us this. Let's continue the example from above:
      //   - fractial = fmodf(-2.2) = -0.2
      //   - Add 1 to the fraction: fractial2 = fractial + 1 = -0.2 + 1 = 0.8
      //   - Finding the `floor`: -2.2 - fractial2 = -2.2 - 0.8 = -3
      ++fractial;
    }

    if (Krys::InexactEquals(fractial, 0))
    {
      // First we check if the value is already rounded
      scaledValue = scaledValue - fractial;
    }
    else if (Krys::InexactEquals(fractial, 1.0))
    {
      scaledValue = scaledValue - fractial + 1.0;
    }
    else if (forceCeil)
    {
      // Next we check if we need to use forced rounding
      scaledValue = scaledValue - fractial + 1.0;
    }
    else if (forceFloor)
    {
      scaledValue = scaledValue - fractial;
    }
    else
    {
      // Finally we just round the value
      scaledValue =
        scaledValue - fractial
        + (!std::isnan(fractial) && (fractial > 0.5 || Krys::InexactEquals(fractial, 0.5)) ? 1.0 : 0.0);
    }
    return (std::isnan(scaledValue) || std::isnan(pointScaleFactor))
             ? std::numeric_limits<float>::quiet_NaN()
             : (float)(scaledValue / pointScaleFactor);
  }

  void RoundLayoutResultsToPixelGrid(Node *const node, const double absoluteLeft, const double absoluteTop)
  {
    const auto pointScaleFactor = static_cast<double>(node->GetConfig()->GetPointScaleFactor());

    const double nodeLeft = node->GetLayout().GetPosition(PhysicalEdge::Left);
    const double nodeTop = node->GetLayout().GetPosition(PhysicalEdge::Top);

    const double nodeWidth = node->GetLayout().GetDimension(Dimension::Width);
    const double nodeHeight = node->GetLayout().GetDimension(Dimension::Height);

    const double absoluteNodeLeft = absoluteLeft + nodeLeft;
    const double absoluteNodeTop = absoluteTop + nodeTop;

    const double absoluteNodeRight = absoluteNodeLeft + nodeWidth;
    const double absoluteNodeBottom = absoluteNodeTop + nodeHeight;

    if (pointScaleFactor != 0.0)
    {
      // If a node has a custom measure function we never want to round down its
      // size as this could lead to unwanted text truncation.
      const bool textRounding = node->GetNodeType() == NodeType::Text;

      node->SetLayoutPosition(RoundValueToPixelGrid(nodeLeft, pointScaleFactor, false, textRounding),
                              PhysicalEdge::Left);

      node->SetLayoutPosition(RoundValueToPixelGrid(nodeTop, pointScaleFactor, false, textRounding),
                              PhysicalEdge::Top);

      // We multiply dimension by scale factor and if the result is close to the
      // whole number, we don't have any fraction To verify if the result is close
      // to whole number we want to check both floor and ceil numbers

      const double scaledNodeWith = nodeWidth * pointScaleFactor;
      const bool hasFractionalWidth = !Krys::InexactEquals(round(scaledNodeWith), scaledNodeWith);

      const double scaledNodeHeight = nodeHeight * pointScaleFactor;
      const bool hasFractionalHeight = !Krys::InexactEquals(round(scaledNodeHeight), scaledNodeHeight);

      node->GetLayout().SetDimension(
        Dimension::Width,
        RoundValueToPixelGrid(absoluteNodeRight, pointScaleFactor, (textRounding && hasFractionalWidth),
                              (textRounding && !hasFractionalWidth))
          - RoundValueToPixelGrid(absoluteNodeLeft, pointScaleFactor, false, textRounding));

      node->GetLayout().SetDimension(
        Dimension::Height,
        RoundValueToPixelGrid(absoluteNodeBottom, pointScaleFactor, (textRounding && hasFractionalHeight),
                              (textRounding && !hasFractionalHeight))
          - RoundValueToPixelGrid(absoluteNodeTop, pointScaleFactor, false, textRounding));
    }

    for (Node *child : node->GetChildren())
    {
      RoundLayoutResultsToPixelGrid(child, absoluteNodeLeft, absoluteNodeTop);
    }
  }
}