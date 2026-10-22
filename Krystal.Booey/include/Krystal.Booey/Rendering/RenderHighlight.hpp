#pragma once

#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.Booey/Rendering/TextBoxSelectableRange.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include <utility>

namespace krys::boo
{
  class RenderText;

  /// @brief The renderer after @p offset in @p renderer: its child at that index, or the next renderer after
  /// its children.
  KRYS_NODISCARD RenderObject *RendererAfterOffset(const RenderObject &renderer, uint32 offset) noexcept;

  /// @brief A range of the render tree: from an offset in a start renderer to an offset in an end renderer.
  class RenderRange
  {
  private:
    WeakPtr<RenderObject> _start;
    WeakPtr<RenderObject> _end;
    uint32 _startOffset {0};
    uint32 _endOffset {0};

  public:
    RenderRange() noexcept = default;
    RenderRange(RenderObject *start, RenderObject *end, uint32 startOffset, uint32 endOffset) noexcept
        : _start(CreateWeakPtr(start)), _end(CreateWeakPtr(end)), _startOffset(startOffset),
          _endOffset(endOffset)
    {
    }

    KRYS_NODISCARD RenderObject *Start() const noexcept
    {
      return _start.get();
    }
    KRYS_NODISCARD RenderObject *End() const noexcept
    {
      return _end.get();
    }
    KRYS_NODISCARD uint32 StartOffset() const noexcept
    {
      return _startOffset;
    }
    KRYS_NODISCARD uint32 EndOffset() const noexcept
    {
      return _endOffset;
    }

    KRYS_NODISCARD friend bool operator==(const RenderRange &a, const RenderRange &b) noexcept
    {
      return a.Start() == b.Start() && a.End() == b.End() && a._startOffset == b._startOffset
             && a._endOffset == b._endOffset;
    }
  };

  /// @brief Walks the renderers of a range in pre-order.
  // FIXME: Column spanners (RenderMultiColumnSpannerPlaceholder), once there are multiple columns.
  class RenderRangeIterator
  {
  private:
    RenderObject *_current {nullptr};

  public:
    explicit RenderRangeIterator(RenderObject *start) noexcept : _current(start)
    {
    }

    KRYS_NODISCARD RenderObject *Current() const noexcept
    {
      return _current;
    }
    RenderObject *Next() noexcept;
  };

  /// @brief A highlighted range of the render tree, and which part of it each renderer and text box holds.
  // FIXME: Highlight ranges (the Highlight API's setRenderRange(const HighlightRange &)), with
  // `::highlight()`.
  class RenderHighlight
  {
  protected:
    RenderRange _renderRange;
    const bool _isSelection {false};

  public:
    enum IsSelectionTag
    {
      IsSelection
    };

    RenderHighlight() noexcept = default;
    explicit RenderHighlight(IsSelectionTag) noexcept : _isSelection(true)
    {
    }

    void SetRenderRange(const RenderRange &renderRange) noexcept;
    KRYS_NODISCARD const RenderRange &Get() const noexcept
    {
      return _renderRange;
    }

    KRYS_NODISCARD RenderObject *Start() const noexcept
    {
      return _renderRange.Start();
    }
    KRYS_NODISCARD RenderObject *End() const noexcept
    {
      return _renderRange.End();
    }

    KRYS_NODISCARD uint32 StartOffset() const noexcept
    {
      return _renderRange.StartOffset();
    }
    KRYS_NODISCARD uint32 EndOffset() const noexcept
    {
      return _renderRange.EndOffset();
    }

    KRYS_NODISCARD RenderObject::HighlightState
      HighlightStateForRenderer(const RenderObject &renderer) const noexcept;
    KRYS_NODISCARD RenderObject::HighlightState
      HighlightStateForTextBox(const RenderText &renderer,
                               const TextBoxSelectableRange &textBoxRange) const noexcept;
    KRYS_NODISCARD std::pair<uint32, uint32>
      RangeForTextBox(const RenderText &renderer, const TextBoxSelectableRange &textBoxRange) const noexcept;
  };
}
