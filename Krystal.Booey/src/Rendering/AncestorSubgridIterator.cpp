#include "Krystal.Booey/Rendering/AncestorSubgridIterator.hpp"
#include "Krystal.Booey/Rendering/GridLayoutFunctions.hpp"
#include "Krystal.Booey/Rendering/RenderGrid.hpp"
#include "Krystal.Booey/Rendering/RenderIterator.hpp"

namespace krys::boo
{
  AncestorSubgridIterator::AncestorSubgridIterator(const RenderGrid *firstAncestorSubgrid,
                                                   style::GridTrackSizingDirection direction) noexcept
      : _firstAncestorSubgrid(firstAncestorSubgrid), _direction(direction)
  {
    krys_debug_assert(!firstAncestorSubgrid || firstAncestorSubgrid->IsSubgrid(direction));
  }

  AncestorSubgridIterator::AncestorSubgridIterator(const RenderGrid *firstAncestorSubgrid,
                                                   RenderGrid *currentAncestorSubgrid,
                                                   Maybe<style::GridTrackSizingDirection> direction) noexcept
      : _firstAncestorSubgrid(firstAncestorSubgrid), _currentAncestorSubgrid(currentAncestorSubgrid),
        _direction(direction)
  {
  }

  AncestorSubgridIterator AncestorSubgridIterator::begin() const noexcept
  {
    // The iterator hands out the subgrids as mutable: they're the grid item's ancestors, laid out by its
    // grid.
    return AncestorSubgridIterator(_firstAncestorSubgrid, const_cast<RenderGrid *>(_firstAncestorSubgrid),
                                   _direction);
  }

  AncestorSubgridIterator AncestorSubgridIterator::end() const noexcept
  {
    return AncestorSubgridIterator(_firstAncestorSubgrid, nullptr, _direction);
  }

  AncestorSubgridIterator &AncestorSubgridIterator::operator++() noexcept
  {
    krys_debug_assert(_firstAncestorSubgrid && _currentAncestorSubgrid && _direction);

    if (_firstAncestorSubgrid && _currentAncestorSubgrid && _direction)
    {
      auto *nextAncestor = RenderTraversal::FindAncestorOfType<RenderGrid>(*_currentAncestorSubgrid);
      _currentAncestorSubgrid =
        (nextAncestor
         && nextAncestor->IsSubgrid(GridLayoutFunctions::FlowAwareDirectionForGridItem(
           *nextAncestor, *_firstAncestorSubgrid, *_direction)))
          ? nextAncestor
          : nullptr;
    }
    return *this;
  }

  RenderGrid &AncestorSubgridIterator::operator*() const noexcept
  {
    krys_debug_assert(_currentAncestorSubgrid);
    return *_currentAncestorSubgrid;
  }

  bool AncestorSubgridIterator::operator==(const AncestorSubgridIterator &other) const noexcept
  {
    return _currentAncestorSubgrid == other._currentAncestorSubgrid
           && _firstAncestorSubgrid == other._firstAncestorSubgrid && _direction == other._direction;
  }

  AncestorSubgridIterator AncestorSubgridsOfGridItem(const RenderBox &gridItem,
                                                     style::GridTrackSizingDirection direction) noexcept
  {
    krys_debug_assert(gridItem.Parent()->IsRenderGrid());
    if (const auto *gridItemParent = DynamicDowncast<RenderGrid>(gridItem.Parent());
        gridItemParent && gridItemParent->IsSubgrid(direction))
    {
      return AncestorSubgridIterator(gridItemParent, direction);
    }
    return AncestorSubgridIterator();
  }
}
