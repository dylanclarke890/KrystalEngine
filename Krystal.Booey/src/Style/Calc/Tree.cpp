#include "Krystal.Booey/Style/Calc/Tree.hpp"
#include "Krystal.Booey/Style/Calc/TreeTraversal.hpp"
#include "Krystal.Core/Visitor.hpp"
#include <algorithm>

namespace krys::boo::style::calc
{
  KRYS_NODISCARD static size_t ComputeDepth(const Child &root) noexcept
  {
    size_t maximumChildDepth = 0;
    ForAllChildren(
      root,
      krys::CreateVisitor(
        [&](const Maybe<Child> &child)
        {
          if (child)
          {
            maximumChildDepth = std::max(ComputeDepth(*child), maximumChildDepth);
          }
        },
        [&](const Child &child) { maximumChildDepth = std::max(ComputeDepth(child), maximumChildDepth); },
        [&](const ChildOrNone &childOrNone)
        {
          if (krys::HoldsAlternative<Child>(childOrNone))
          {
            maximumChildDepth = std::max(ComputeDepth(get<Child>(childOrNone)), maximumChildDepth);
          }
        },
        [&](const auto &) { maximumChildDepth = std::max<size_t>(1, maximumChildDepth); }));
    return maximumChildDepth + 1;
  }

  size_t ComputeDepth(const Tree &tree) noexcept
  {
    return ComputeDepth(tree.Root);
  }
}
