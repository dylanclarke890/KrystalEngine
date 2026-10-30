#include "Krystal.Booey/Rendering/GridLayoutState.hpp"

namespace krys::boo
{
  bool GridLayoutState::ContainsLayoutRequirementForGridItem(
    const RenderBox &gridItem, ItemLayoutRequirement layoutRequirement) const noexcept
  {
    if (auto itr = _itemsLayoutRequirements.find(&gridItem); itr != _itemsLayoutRequirements.end())
    {
      return itr->second.Contains(layoutRequirement);
    }
    return false;
  }

  void GridLayoutState::SetLayoutRequirementForGridItem(const RenderBox &gridItem,
                                                        ItemLayoutRequirement layoutRequirement) noexcept
  {
    _itemsLayoutRequirements[&gridItem].Add(layoutRequirement);
  }
}
