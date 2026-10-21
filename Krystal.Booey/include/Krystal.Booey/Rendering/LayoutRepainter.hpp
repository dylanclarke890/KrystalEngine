#pragma once

#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  class RenderLayerModelObject;

  /// @brief Repaints a renderer after its layout: its rects before layout are taken when it's made, and
  /// `RepaintAfterLayout` repaints what changed between them and the rects after.
  class LayoutRepainter
  {
    KRYS_NON_COPY_MOVABLE(LayoutRepainter);

  public:
    enum class CheckForRepaint : uint8
    {
      No,
      Yes
    };
    enum class ShouldAlwaysIssueFullRepaint : uint8
    {
      No,
      Yes
    };

  private:
    RenderElement &_renderer;
    const RenderLayerModelObject *_repaintContainer {nullptr};
    // We store these values as LayoutRects, but the final invalidations will be pixel snapped
    RenderObject::RepaintRects _oldRects;
    bool _checkForRepaint {true};
    bool _forceFullRepaint {false};
    RepaintOutlineBounds _repaintOutlineBounds;

  public:
    explicit LayoutRepainter(RenderElement &renderer, Maybe<CheckForRepaint> checkForRepaintOverride = null,
                             Maybe<ShouldAlwaysIssueFullRepaint> shouldAlwaysIssueFullRepaint = null,
                             RepaintOutlineBounds repaintOutlineBounds = RepaintOutlineBounds::Yes) noexcept;

    /// @brief Repaints what layout changed.
    /// @return Whether it repainted.
    bool RepaintAfterLayout() noexcept;
  };
}
