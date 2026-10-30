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
