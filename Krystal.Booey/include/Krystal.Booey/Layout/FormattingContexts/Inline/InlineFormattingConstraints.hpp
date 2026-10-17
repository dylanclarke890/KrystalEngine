#pragma once

#include "Krystal.Booey/Graphics/LayoutSize.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/FormattingConstraints.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::layout
{
  /// @brief The space inline content is laid out in, with the formatting root's visual left and border box
  /// size.
  class ConstraintsForInlineContent : public ConstraintsForInFlowContent
  {
  private:
    LayoutUnit _visualLeft;
    LayoutSize _formattingRootBorderBoxSize;

  public:
    ConstraintsForInlineContent(const ConstraintsForInFlowContent &genericContraints, LayoutUnit visualLeft,
                                LayoutSize formattingRootBorderBoxSize) noexcept
        : ConstraintsForInFlowContent(genericContraints.Horizontal(), genericContraints.LogicalTop(),
                                      InlineContent),
          _visualLeft(visualLeft), _formattingRootBorderBoxSize(formattingRootBorderBoxSize)
    {
    }

    KRYS_NODISCARD LayoutUnit VisualLeft() const noexcept
    {
      return _visualLeft;
    }
    KRYS_NODISCARD LayoutSize FormattingRootBorderBoxSize() const noexcept
    {
      return _formattingRootBorderBoxSize;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::layout::ConstraintsForInlineContent)
  KRYS_NODISCARD static bool IsType(
    const krys::boo::layout::ConstraintsForInFlowContent &constraints) noexcept
  {
    return constraints.IsConstraintsForInlineContent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
