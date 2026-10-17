#pragma once

#include "Krystal.Booey/Layout/LayoutState.hpp"
#include "Krystal.Booey/Rendering/RenderGrid.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::layout
{
  class ElementBox;
}

namespace krys::boo::layout_integration
{
  /// @brief Lays out a grid container's content with the grid formatting context and copies the result back
  /// to the render tree.
  class GridLayout
  {
    KRYS_NON_COPY_MOVABLE(GridLayout);

  private:
    CheckedPtr<layout::ElementBox> _gridBox;
    WeakPtr<layout::LayoutState> _layoutState;

  public:
    explicit GridLayout(RenderGrid &renderGrid) noexcept;
    ~GridLayout() noexcept;

    /// @brief Whether the grid formatting context supports the grid container's content.
    KRYS_NODISCARD static bool CanUseFor(const RenderGrid &renderGrid) noexcept;

    void UpdateFormattingContextGeometries() noexcept;

    void Layout() noexcept;

  private:
    void UpdateGridItemRenderers() noexcept;
    void UpdateFormattingContextRootRenderer() noexcept;

    KRYS_NODISCARD const layout::ElementBox &GridBox() const noexcept
    {
      return *_gridBox;
    }
    KRYS_NODISCARD layout::ElementBox &GridBox() noexcept
    {
      return *_gridBox;
    }

    KRYS_NODISCARD const RenderGrid &GridBoxRenderer() const noexcept;
    KRYS_NODISCARD RenderGrid &GridBoxRenderer() noexcept;

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
