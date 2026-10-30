#include "Krystal.Booey/Rendering/LayoutRepainter.hpp"
#include "Krystal.Booey/Rendering/RenderLayerModelObject.hpp"

namespace krys::boo
{
  LayoutRepainter::LayoutRepainter(RenderElement &renderer, Maybe<CheckForRepaint> checkForRepaintOverride,
                                   Maybe<ShouldAlwaysIssueFullRepaint> shouldAlwaysIssueFullRepaint,
                                   RepaintOutlineBounds repaintOutlineBounds) noexcept
      : _renderer(renderer),
        _checkForRepaint(checkForRepaintOverride ? *checkForRepaintOverride == CheckForRepaint::Yes
                                                 : renderer.CheckForRepaintDuringLayout()),
        _forceFullRepaint(shouldAlwaysIssueFullRepaint
                          && *shouldAlwaysIssueFullRepaint == ShouldAlwaysIssueFullRepaint::Yes),
        _repaintOutlineBounds(repaintOutlineBounds)
  {
    if (!_checkForRepaint)
    {
      return;
    }

    _repaintContainer = _renderer.ContainerForRepaint().Renderer;
    _oldRects = _renderer.RectsForRepaintingAfterLayout(_repaintContainer, _repaintOutlineBounds);
  }

  bool LayoutRepainter::RepaintAfterLayout() noexcept
  {
    if (!_checkForRepaint)
    {
      return false;
    }

    auto requiresFullRepaint =
      _forceFullRepaint || _renderer.SelfNeedsLayout() ? RequiresFullRepaint::Yes : RequiresFullRepaint::No;
    // Outline bounds are not used if we're doing a full repaint.
    auto newRects = _renderer.RectsForRepaintingAfterLayout(
      _repaintContainer,
      (requiresFullRepaint == RequiresFullRepaint::Yes) ? RepaintOutlineBounds::No : _repaintOutlineBounds);
    return _renderer.RepaintAfterLayoutIfNeeded(_repaintContainer, requiresFullRepaint, _oldRects, newRects);
  }
}
