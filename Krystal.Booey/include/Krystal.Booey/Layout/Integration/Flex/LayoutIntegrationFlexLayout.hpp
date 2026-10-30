#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Layout/LayoutState.hpp"
#include "Krystal.Booey/Rendering/RenderFlexibleBox.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class RenderBlock;
  class RenderStyle;
}

namespace krys::boo::layout
{
  class ElementBox;
}

namespace krys::boo::layout_integration
{
  /// @brief Lays out a flex container's content with the flex formatting context and copies the result back
  /// to the render tree.
  // FIXME: Painting and hit testing (paint, hitTest).
  class FlexLayout
  {
    KRYS_NON_COPY_MOVABLE(FlexLayout);

  private:
    CheckedPtr<layout::ElementBox> _flexBox;
    WeakPtr<layout::LayoutState> _layoutState;

  public:
    explicit FlexLayout(RenderFlexibleBox &flexBoxRenderer) noexcept;
    ~FlexLayout() noexcept;

    /// @brief Whether the flex formatting context supports the flex container's content.
    KRYS_NODISCARD static bool CanUseFor(const RenderFlexibleBox &flexBox) noexcept;

    void UpdateFormattingContextGeometries() noexcept;
    void UpdateStyle(const RenderBlock &renderer, const RenderStyle &oldStyle) noexcept;
    KRYS_NODISCARD Pair<LayoutUnit, LayoutUnit> ComputeIntrinsicWidthConstraints() noexcept;

    void Layout() noexcept;

    void CollectOverflow() noexcept;
    KRYS_NODISCARD LayoutUnit ContentBoxLogicalHeight() const noexcept;

  private:
    void UpdateRenderers() noexcept;

    KRYS_NODISCARD const layout::ElementBox &FlexBox() const noexcept
    {
      return *_flexBox;
    }
    KRYS_NODISCARD layout::ElementBox &FlexBox() noexcept
    {
      return *_flexBox;
    }

    KRYS_NODISCARD const RenderFlexibleBox &FlexBoxRenderer() const noexcept;
    KRYS_NODISCARD RenderFlexibleBox &FlexBoxRenderer() noexcept;

    KRYS_NODISCARD layout::LayoutState &GetLayoutState() noexcept
    {
      return *_layoutState;
    }
    KRYS_NODISCARD const layout::LayoutState &GetLayoutState() const noexcept
    {
      return *_layoutState;
    }
  };
}
