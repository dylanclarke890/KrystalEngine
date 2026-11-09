/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
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
