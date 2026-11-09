/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Rendering/Grid/AncestorSubgridIterator.hpp"
#include "Krystal.Booey/Rendering/Grid/GridLayoutFunctions.hpp"
#include "Krystal.Booey/Rendering/Grid/RenderGrid.hpp"
#include "Krystal.Booey/Rendering/Iterators/RenderIterator.hpp"

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
