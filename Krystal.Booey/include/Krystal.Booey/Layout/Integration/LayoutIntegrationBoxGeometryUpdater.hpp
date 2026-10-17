#pragma once

#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingConstraints.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLineTypes.hpp"
#include "Krystal.Booey/Layout/LayoutState.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxGeometry.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class RenderBlock;
  class RenderBox;
  class RenderBoxModelObject;
  class RenderElement;
  class RenderInline;
  class RenderLineBreak;
  class RenderListMarker;
}

namespace krys::boo::layout
{
  class ElementBox;
  class LayoutState;
}

namespace krys::boo::layout_integration
{
  /// @brief Copies the geometry the render tree computed (margins, borders, padding, sizes and baselines) to
  /// the layout boxes of a formatting context before the layout formatting context runs.
  class BoxGeometryUpdater
  {
  private:
    WeakPtr<layout::LayoutState> _layoutState;
    CheckedPtr<const layout::ElementBox> _rootLayoutBox;
    HashMap<const layout::ElementBox *, LayoutUnit> _nestedListMarkerOffsets;

  public:
    BoxGeometryUpdater(layout::LayoutState &layoutState, const layout::ElementBox &rootLayoutBox) noexcept;

    void Clear() noexcept;

    void SetFormattingContextRootGeometry(LayoutUnit availableLogicalWidth) noexcept;
    void SetFormattingContextContentGeometry(Maybe<LayoutUnit> availableLogicalWidth,
                                             Maybe<layout::IntrinsicWidthMode> intrinsicWidthMode) noexcept;
    void UpdateBoxGeometryAfterIntegrationLayout(const layout::ElementBox &layoutBox,
                                                 LayoutUnit availableWidth) noexcept;
    KRYS_NODISCARD layout::ConstraintsForInlineContent
      FormattingContextConstraints(LayoutUnit availableWidth) noexcept;

    KRYS_NODISCARD HashMap<const layout::ElementBox *, LayoutUnit> TakeNestedListMarkerOffsets() noexcept
    {
      return krys::move(_nestedListMarkerOffsets);
    }

  private:
    void UpdateBoxGeometry(const RenderElement &renderer, Maybe<LayoutUnit> availableWidth,
                           Maybe<layout::IntrinsicWidthMode> intrinsicWidthMode) noexcept;
    void UpdateLayoutBoxDimensions(const RenderBox &renderBox, Maybe<LayoutUnit> availableWidth,
                                   Maybe<layout::IntrinsicWidthMode> intrinsicWidthMode = null) noexcept;
    void UpdateLineBreakBoxDimensions(const RenderLineBreak &lineBreakBox) noexcept;
    void UpdateInlineBoxDimensions(const RenderInline &renderInline, Maybe<LayoutUnit> availableWidth,
                                   Maybe<layout::IntrinsicWidthMode> intrinsicWidthMode = null) noexcept;
    void SetListMarkerOffsetForMarkerOutside(const RenderListMarker &listMarker) noexcept;

    KRYS_NODISCARD layout::BoxGeometry::HorizontalEdges
      HorizontalLogicalMargin(const RenderBoxModelObject &renderer, Maybe<LayoutUnit> availableWidth,
                              WritingMode writingMode, bool retainMarginStart = true,
                              bool retainMarginEnd = true) noexcept;
    KRYS_NODISCARD layout::BoxGeometry::VerticalEdges
      VerticalLogicalMargin(const RenderBoxModelObject &renderer, Maybe<LayoutUnit> availableWidth,
                            WritingMode writingMode) noexcept;
    KRYS_NODISCARD layout::BoxGeometry::Edges LogicalBorder(const RenderBoxModelObject &renderer,
                                                            WritingMode writingMode,
                                                            bool isIntrinsicWidthMode = false,
                                                            bool retainBorderStart = true,
                                                            bool retainBorderEnd = true) noexcept;
    KRYS_NODISCARD layout::BoxGeometry::Edges LogicalPadding(const RenderBoxModelObject &renderer,
                                                             Maybe<LayoutUnit> availableWidth,
                                                             WritingMode writingMode,
                                                             bool retainPaddingStart = true,
                                                             bool retainPaddingEnd = true) noexcept;

    KRYS_NODISCARD layout::LayoutState &GetLayoutState() noexcept
    {
      return *_layoutState;
    }
    KRYS_NODISCARD const layout::LayoutState &GetLayoutState() const noexcept
    {
      return *_layoutState;
    }
    KRYS_NODISCARD const layout::ElementBox &RootLayoutBox() const noexcept
    {
      return *_rootLayoutBox;
    }
    KRYS_NODISCARD const RenderBlock &RootRenderer() const noexcept;
    KRYS_NODISCARD WritingMode GetWritingMode() const noexcept;
  };
}
