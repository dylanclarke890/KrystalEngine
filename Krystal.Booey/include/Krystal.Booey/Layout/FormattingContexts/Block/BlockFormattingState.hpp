/*
 * Copyright (C) 2018 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Layout/Floats/PlacedFloats.hpp"
#include "Krystal.Booey/Layout/FormattingState.hpp"
#include "Krystal.Booey/Layout/MarginTypes.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::layout
{
  /// @brief The state for a particular block formatting context tree.
  class BlockFormattingState : public FormattingState
  {
    KRYS_NON_COPY_MOVABLE(BlockFormattingState);

  public:
    // Since we layout the out-of-flow boxes at the end of the formatting context layout, it's okay to store
    // them in the formatting state -as opposed to the containing block level.
    using OutOfFlowBoxList = List<CheckedRef<const Box>>;

  private:
    PlacedFloats _placedFloats;
    OutOfFlowBoxList _outOfFlowBoxes;
    HashMap<const Box *, UsedVerticalMargin> _usedVerticalMargins;
    HashSet<const Box *> _clearanceSet;

  public:
    BlockFormattingState(LayoutState &layoutState, const ElementBox &blockFormattingContextRoot) noexcept;
    ~BlockFormattingState() noexcept;

    KRYS_NODISCARD const PlacedFloats &GetPlacedFloats() const noexcept
    {
      return _placedFloats;
    }
    KRYS_NODISCARD PlacedFloats &GetPlacedFloats() noexcept
    {
      return _placedFloats;
    }

    void AddOutOfFlowBox(const Box &outOfFlowBox) noexcept
    {
      _outOfFlowBoxes.push_back(ShareCheckedRef(outOfFlowBox));
    }
    void SetOutOfFlowBoxes(OutOfFlowBoxList &&outOfFlowBoxes) noexcept
    {
      _outOfFlowBoxes = krys::move(outOfFlowBoxes);
    }
    KRYS_NODISCARD const OutOfFlowBoxList &OutOfFlowBoxes() const noexcept
    {
      return _outOfFlowBoxes;
    }

    void SetUsedVerticalMargin(const Box &layoutBox, const UsedVerticalMargin &usedVerticalMargin) noexcept
    {
      _usedVerticalMargins[&layoutBox] = usedVerticalMargin;
    }
    KRYS_NODISCARD UsedVerticalMargin GetUsedVerticalMargin(const Box &layoutBox) const noexcept
    {
      auto iterator = _usedVerticalMargins.find(&layoutBox);
      return iterator != _usedVerticalMargins.end() ? iterator->second : UsedVerticalMargin {};
    }
    KRYS_NODISCARD bool HasUsedVerticalMargin(const Box &layoutBox) const noexcept
    {
      return _usedVerticalMargins.contains(&layoutBox);
    }

    void SetHasClearance(const Box &layoutBox) noexcept
    {
      _clearanceSet.insert(&layoutBox);
    }
    void ClearHasClearance(const Box &layoutBox) noexcept
    {
      _clearanceSet.erase(&layoutBox);
    }
    KRYS_NODISCARD bool HasClearance(const Box &layoutBox) const noexcept
    {
      return _clearanceSet.contains(&layoutBox);
    }

    void ShrinkToFit() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::layout::BlockFormattingState)
  KRYS_NODISCARD static bool IsType(const krys::boo::layout::FormattingState &formattingState) noexcept
  {
    return formattingState.IsBlockFormattingState();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
