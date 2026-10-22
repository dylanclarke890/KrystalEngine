#pragma once

#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Rendering/GapRects.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  class RenderBlock;
  class RenderLayerModelObject;

  /// @brief A renderer's selection state and repaint container, captured when the selection changes.
  class RenderSelectionGeometryBase
  {
    KRYS_NON_COPYABLE(RenderSelectionGeometryBase);

  protected:
    RenderObject &_renderer;
    const RenderLayerModelObject *_repaintContainer;

  private:
    RenderObject::HighlightState _state;

  public:
    explicit RenderSelectionGeometryBase(RenderObject &renderer) noexcept;

    KRYS_NODISCARD const RenderLayerModelObject *RepaintContainer() const noexcept
    {
      return _repaintContainer;
    }
    KRYS_NODISCARD RenderObject::HighlightState State() const noexcept
    {
      return _state;
    }

  protected:
    void RepaintRectangle(const LayoutRect &repaintRect) noexcept;
  };

  // This struct is used when the selection changes to cache the old and new state of the selection for each
  // RenderObject.
  class RenderSelectionGeometry : public RenderSelectionGeometryBase
  {
  private:
    List<FloatQuad> _collectedSelectionQuads; // relative to repaint container
    LayoutRect _rect;                         // relative to repaint container

  public:
    RenderSelectionGeometry(RenderObject &renderer, bool clipToVisibleContent) noexcept;

    void Repaint() noexcept;
    KRYS_NODISCARD const List<FloatQuad> &CollectedSelectionQuads() const noexcept
    {
      return _collectedSelectionQuads;
    }
    KRYS_NODISCARD LayoutRect Rect() const noexcept
    {
      return _rect;
    }
  };

  // This struct is used when the selection changes to cache the old and new state of the selection for each
  // RenderBlock.
  class RenderBlockSelectionGeometry : public RenderSelectionGeometryBase
  {
  private:
    GapRects _rects; // relative to repaint container

  public:
    explicit RenderBlockSelectionGeometry(RenderBlock &renderer) noexcept;

    void Repaint() noexcept;
    KRYS_NODISCARD GapRects Rects() const noexcept
    {
      return _rects;
    }
  };
}
