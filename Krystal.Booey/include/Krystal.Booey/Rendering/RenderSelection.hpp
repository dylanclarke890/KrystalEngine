#pragma once

#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Rendering/RenderHighlight.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  class RenderView;

  /// @brief The frame's selection in the render tree: the renderers' selection states, and repainting what a
  /// change adds and removes.
  // FIXME: The selection geometry gatherer (SelectionGeometryGatherer), with service controls.
  class RenderSelection : public RenderHighlight
  {
  public:
    enum class RepaintMode : uint8
    {
      NewXOROld,
      NewMinusOld,
      Nothing
    };

  private:
    enum class ClipToVisibleContent : bool
    {
      No,
      Yes
    };

    const RenderView &_renderView;
    bool _selectionWasCaret {false};

  public:
    explicit RenderSelection(RenderView &view) noexcept;

    void Set(const RenderRange &selection, RepaintMode blockRepaintMode = RepaintMode::NewXOROld) noexcept;
    void Clear() noexcept;
    void Repaint() const noexcept;

    KRYS_NODISCARD IntRect Bounds() const noexcept
    {
      return CollectBounds(ClipToVisibleContent::No);
    }
    KRYS_NODISCARD IntRect BoundsClippedToVisibleContent() const noexcept
    {
      return CollectBounds(ClipToVisibleContent::Yes);
    }

  private:
    KRYS_NODISCARD IntRect CollectBounds(ClipToVisibleContent clipToVisibleContent) const noexcept;
    void Apply(const RenderRange &newSelection, RepaintMode blockRepaintMode) noexcept;
  };
}
