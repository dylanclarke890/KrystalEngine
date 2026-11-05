#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/HashMap.hpp"

namespace krys::boo
{
  class RenderBox;

  enum class ItemLayoutRequirement : uint8
  {
    NeedsColumnAxisStretchAlignment,
    MinContentContributionForSecondColumnPass,
  };
  // Keyed by address: the state lives for one layout of the grid.
  using ItemsLayoutRequirements = HashMap<const RenderBox *, EnumSet<ItemLayoutRequirement>>;

  /// @brief The state of one layout of a grid: what its items need from later passes.
  class GridLayoutState
  {
  private:
    ItemsLayoutRequirements _itemsLayoutRequirements;
    bool _needsSecondTrackSizingPass {false};
    bool _hasAspectRatioBlockSizeDependentItem {false};

  public:
    KRYS_NODISCARD bool
      ContainsLayoutRequirementForGridItem(const RenderBox &gridItem,
                                           ItemLayoutRequirement layoutRequirement) const noexcept;
    void SetLayoutRequirementForGridItem(const RenderBox &gridItem,
                                         ItemLayoutRequirement layoutRequirement) noexcept;

    KRYS_NODISCARD bool NeedsSecondTrackSizingPass() const noexcept
    {
      return _needsSecondTrackSizingPass;
    }
    void SetNeedsSecondTrackSizingPass() noexcept
    {
      _needsSecondTrackSizingPass = true;
    }

    void SetHasAspectRatioBlockSizeDependentItem() noexcept
    {
      _hasAspectRatioBlockSizeDependentItem = true;
    }
    KRYS_NODISCARD bool HasAspectRatioBlockSizeDependentItem() const noexcept
    {
      return _hasAspectRatioBlockSizeDependentItem;
    }
  };
}
