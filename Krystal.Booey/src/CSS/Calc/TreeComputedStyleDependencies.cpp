#include "Krystal.Booey/CSS/Calc/TreeComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Calc/TreeTraversal.hpp"
#include "Krystal.Booey/CSS/ComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Units.hpp"

namespace krys::boo::css::calc
{
  static void CollectComputedStyleDependencies(const Child &root,
                                               ComputedStyleDependencies &dependencies) noexcept
  {
    krys::SwitchOn(
      root,
      [&](const Number &)
      {
        // No potential dependencies.
      },
      [&](const Percentage &)
      {
        // No potential dependencies.
      },
      [&](const CanonicalDimension &)
      {
        // No potential dependencies.
      },
      [&](const NonCanonicalDimension &root)
      {
        if (auto lengthUnit = css::ToLengthUnit(root.Unit))
        {
          css::CollectComputedStyleDependencies(dependencies, *lengthUnit);
        }
      },
      [&](const Symbol &root)
      {
        if (auto lengthUnit = css::ToLengthUnit(root.Unit))
        {
          css::CollectComputedStyleDependencies(dependencies, *lengthUnit);
        }
      },
      [&](const SiblingCount &)
      {
        // No potential dependencies.
      },
      [&](const SiblingIndex &)
      {
        // No potential dependencies.
      },
      [&](const IndirectNode<Anchor> &anchor)
      {
        dependencies.Anchors = true;
        if (anchor->Fallback)
        {
          CollectComputedStyleDependencies(*anchor->Fallback, dependencies);
        }
      },
      [&](const IndirectNode<AnchorSize> &anchorSize)
      {
        dependencies.Anchors = true;
        if (anchorSize->Fallback)
        {
          CollectComputedStyleDependencies(*anchorSize->Fallback, dependencies);
        }
      },
      [&](const auto &root)
      {
        ForAllChildNodes(*root,
                         [&](const auto &root) { CollectComputedStyleDependencies(root, dependencies); });
      });
  }

  void CollectComputedStyleDependencies(const Tree &tree, ComputedStyleDependencies &dependencies) noexcept
  {
    CollectComputedStyleDependencies(tree.Root, dependencies);
  }
}