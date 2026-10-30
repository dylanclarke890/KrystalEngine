#include "Krystal.Booey/Rendering/RenderSelectionGeometry.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"

namespace krys::boo
{
  RenderSelectionGeometryBase::RenderSelectionGeometryBase(RenderObject &renderer) noexcept
      : _renderer(renderer), _repaintContainer(renderer.ContainerForRepaint().Renderer),
        _state(renderer.SelectionState())
  {
  }

  void RenderSelectionGeometryBase::RepaintRectangle(const LayoutRect &repaintRect) noexcept
  {
    _renderer.RepaintUsingContainer(_repaintContainer, LayoutRect(EnclosingIntRect(repaintRect)));
  }

  RenderSelectionGeometry::RenderSelectionGeometry(RenderObject &renderer, bool clipToVisibleContent) noexcept
      : RenderSelectionGeometryBase(renderer)
  {
    if (renderer.CanUpdateSelectionOnRootLineBoxes())
    {
      if (auto *textRenderer = DynamicDowncast<RenderText>(renderer))
      {
        _rect = textRenderer->CollectSelectionGeometriesForLineBoxes(_repaintContainer, clipToVisibleContent,
                                                                     _collectedSelectionQuads);
      }
      else
      {
        _rect = renderer.SelectionRectForRepaint(_repaintContainer, clipToVisibleContent);
      }
    }
  }

  void RenderSelectionGeometry::Repaint() noexcept
  {
    RepaintRectangle(_rect);
  }

  RenderBlockSelectionGeometry::RenderBlockSelectionGeometry(RenderBlock &renderer) noexcept
      : RenderSelectionGeometryBase(renderer),
        _rects(renderer.CanUpdateSelectionOnRootLineBoxes()
                 ? renderer.SelectionGapRectsForRepaint(_repaintContainer)
                 : GapRects())
  {
  }

  void RenderBlockSelectionGeometry::Repaint() noexcept
  {
    RepaintRectangle(_rects);
  }
}
