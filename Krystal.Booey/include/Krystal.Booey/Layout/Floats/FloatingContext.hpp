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
#include "Krystal.Booey/Layout/FormattingContexts/FormattingConstraints.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutElementBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::layout
{
  class FloatAvoider;
  class LayoutState;

  /// @brief Adjusts the position of a box in the current formatting context by taking the floating boxes
  /// into account.
  ///
  /// Note that a FloatingContext's inline direction always matches the root's inline direction but it may not
  /// match the PlacedFloats's inline direction (i.e. PlacedFloats may be constructed by a parent BFC with
  /// mismatching inline direction).
  class FloatingContext
  {
  public:
    struct BlockAxisPositionWithClearance
    {
      LayoutUnit Position;
      Maybe<LayoutUnit> Clearance;
    };

    struct Constraints
    {
      Maybe<PointInContextRoot> Start;
      Maybe<PointInContextRoot> End;
    };

    enum class MayBeAboveLastFloat : bool
    {
      No,
      Yes
    };

  private:
    struct AbsoluteCoordinateValuesForFloatAvoider;

    CheckedRef<const ElementBox> _formattingContextRoot;
    const CheckedRef<const LayoutState> _layoutState;
    const PlacedFloats &_placedFloats;

  public:
    FloatingContext(const ElementBox &formattingContextRoot, const LayoutState &layoutState,
                    const PlacedFloats &placedFloats) noexcept;

    KRYS_NODISCARD const PlacedFloats &GetPlacedFloats() const noexcept
    {
      return _placedFloats;
    }

    KRYS_NODISCARD LayoutPoint
      PositionForFloat(const Box &layoutBox, const BoxGeometry &boxGeometry,
                       const HorizontalConstraints &horizontalConstraints) const noexcept;
    KRYS_NODISCARD LayoutPoint
      PositionForNonFloatingFloatAvoider(const Box &layoutBox, const BoxGeometry &boxGeometry) const noexcept;

    KRYS_NODISCARD Maybe<BlockAxisPositionWithClearance>
      GetBlockAxisPositionWithClearance(const Box &layoutBox, const BoxGeometry &boxGeometry) const noexcept;

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _placedFloats.GetList().empty();
    }

    KRYS_NODISCARD Constraints GetConstraints(LayoutUnit candidateTop, LayoutUnit candidateBottom,
                                              MayBeAboveLastFloat mayBeAboveLastFloat) const noexcept;

    KRYS_NODISCARD PlacedFloats::Item MakeFloatItem(const Box &floatBox, const BoxGeometry &boxGeometry,
                                                    Maybe<size_t> line = null) const noexcept;

    KRYS_NODISCARD bool IsStartPositioned(const Box &floatBox) const noexcept;

  private:
    KRYS_NODISCARD bool
      IsFloatingCandidateStartPositionedInBlockFormattingContext(const Box &floatBox) const noexcept;
    KRYS_NODISCARD boo::Clear ClearInBlockFormattingContext(const Box &clearBox) const noexcept;

    KRYS_NODISCARD const ElementBox &Root() const noexcept
    {
      return *_formattingContextRoot;
    }

    // FIXME: Turn this into an actual geometry cache.
    KRYS_NODISCARD const LayoutState &ContainingBlockGeometries() const noexcept;

    void FindPositionForFormattingContextRoot(
      FloatAvoider &floatAvoider, BoxGeometry::HorizontalEdges containingBlockContentBoxEdges) const noexcept;

    KRYS_NODISCARD AbsoluteCoordinateValuesForFloatAvoider
      AbsoluteCoordinates(const Box &floatAvoider, LayoutPoint borderBoxTopLeft) const noexcept;
    KRYS_NODISCARD LayoutPoint MapTopLeftToBlockFormattingContextRoot(
      const Box &layoutBox, LayoutPoint borderBoxTopLeft) const noexcept;
    KRYS_NODISCARD Point
      MapPointFromFloatingContextRootToBlockFormattingContextRoot(Point position) const noexcept;
  };
}
