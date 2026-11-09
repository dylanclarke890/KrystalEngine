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

#pragma once

#include "Krystal.Booey/Style/Values/Grid/GridTrackSizingDirection.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  class RenderBox;
  class RenderGrid;

  /// @brief Walks a grid item's ancestor subgrids in one direction, from its parent outwards, while each one
  /// is subgridded in that direction.
  class AncestorSubgridIterator
  {
  private:
    const RenderGrid *_firstAncestorSubgrid {nullptr};
    RenderGrid *_currentAncestorSubgrid {nullptr};
    Maybe<style::GridTrackSizingDirection> _direction;

  public:
    AncestorSubgridIterator() noexcept = default;
    AncestorSubgridIterator(const RenderGrid *firstAncestorSubgrid,
                            style::GridTrackSizingDirection direction) noexcept;

    KRYS_NODISCARD RenderGrid &operator*() const noexcept;

    KRYS_NODISCARD bool operator==(const AncestorSubgridIterator &other) const noexcept;

    AncestorSubgridIterator &operator++() noexcept;
    KRYS_NODISCARD AncestorSubgridIterator begin() const noexcept;
    KRYS_NODISCARD AncestorSubgridIterator end() const noexcept;

  private:
    AncestorSubgridIterator(const RenderGrid *firstAncestorSubgrid, RenderGrid *currentAncestor,
                            Maybe<style::GridTrackSizingDirection> direction) noexcept;
  };

  KRYS_NODISCARD AncestorSubgridIterator
    AncestorSubgridsOfGridItem(const RenderBox &gridItem, style::GridTrackSizingDirection direction) noexcept;
}
