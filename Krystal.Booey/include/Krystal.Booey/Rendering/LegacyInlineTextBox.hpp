#pragma once

#include "Krystal.Booey/Graphics/TextRun.hpp"
#include "Krystal.Booey/Rendering/LegacyInlineBox.hpp"
#include "Krystal.Booey/Rendering/TextBoxSelectableRange.hpp"
#include "Krystal.Core/Types/String.hpp"
#include <utility>

namespace krys::boo
{
  class FontCascade;
  class RenderSVGInlineText;

  namespace inline_iterator
  {
    class BoxLegacyPath;
  }

  /// @brief A box holding a run of an SVG text renderer's text on a line.
  class LegacyInlineTextBox : public LegacyInlineBox
  {
    friend class inline_iterator::BoxLegacyPath;
    KRYS_NON_COPY_MOVABLE(LegacyInlineTextBox);

  private:
    LegacyInlineTextBox *_prevTextBox {nullptr}; // The previous box that also uses our RenderObject
    LegacyInlineTextBox *_nextTextBox {nullptr}; // The next box that also uses our RenderObject

    uint32 _start {0};
    uint32 _len {0};

  public:
    explicit LegacyInlineTextBox(RenderSVGInlineText &) noexcept;
    ~LegacyInlineTextBox() noexcept override;

    KRYS_NODISCARD RenderSVGInlineText &Renderer() const noexcept;
    KRYS_NODISCARD const RenderStyle &LineStyle() const noexcept;

    KRYS_NODISCARD LegacyInlineTextBox *PrevTextBox() const noexcept
    {
      return _prevTextBox;
    }
    KRYS_NODISCARD LegacyInlineTextBox *NextTextBox() const noexcept
    {
      return _nextTextBox;
    }
    void SetNextTextBox(LegacyInlineTextBox *n) noexcept
    {
      _nextTextBox = n;
    }
    void SetPreviousTextBox(LegacyInlineTextBox *p) noexcept
    {
      _prevTextBox = p;
    }

    KRYS_NODISCARD bool HasTextContent() const noexcept;

    KRYS_NODISCARD uint32 Start() const noexcept
    {
      return _start;
    }
    KRYS_NODISCARD uint32 End() const noexcept
    {
      return _start + _len;
    }
    KRYS_NODISCARD uint32 Len() const noexcept
    {
      return _len;
    }

    void SetStart(uint32 start) noexcept
    {
      _start = start;
    }
    void SetLen(uint32 len) noexcept
    {
      _len = len;
    }

    void OffsetRun(int32 d) noexcept
    {
      krys_debug_assert(!IsDirty());
      krys_debug_assert(d > 0 || _start >= static_cast<uint32>(-d));
      _start += d;
    }

    KRYS_NODISCARD TextBoxSelectableRange SelectableRange() const noexcept;

    void MarkDirty(bool dirty = true) noexcept final;

    void SetLogicalOverflowRect(const LayoutRect &) noexcept;

    virtual void DirtyOwnLineBoxes() noexcept
    {
      DirtyLineBoxes();
    }

    KRYS_NODISCARD virtual LayoutRect LocalSelectionRect(uint32 startPos, uint32 endPos) const noexcept;
    KRYS_NODISCARD std::pair<uint32, uint32> SelectionStartEnd() const noexcept;

    KRYS_NODISCARD RenderObject::HighlightState SelectionState() const noexcept final;

    KRYS_NODISCARD bool IsLineBreak() const noexcept final;

    KRYS_NODISCARD int32 CaretMinOffset() const noexcept final;
    KRYS_NODISCARD int32 CaretMaxOffset() const noexcept final;

    KRYS_NODISCARD bool HasMarkers() const noexcept;

  private:
    KRYS_NODISCARD LayoutUnit SelectionTop() const noexcept;
    KRYS_NODISCARD LayoutUnit SelectionBottom() const noexcept;
    KRYS_NODISCARD LayoutUnit SelectionHeight() const noexcept;

    void DeleteLine() noexcept final;

    KRYS_NODISCARD bool IsInlineTextBox() const noexcept final
    {
      return true;
    }

    /// @brief The x position relative to the left start of the text line.
    KRYS_NODISCARD float TextPos() const noexcept;

    KRYS_NODISCARD const FontCascade &LineFont() const noexcept;

    /// @brief The effective text for the run.
    KRYS_NODISCARD utf8_stringview Text() const noexcept;
    KRYS_NODISCARD TextRun CreateTextRun() const noexcept;
  };

  KRYS_NODISCARD LayoutRect SnappedSelectionRect(const LayoutRect &, float logicalRight,
                                                 WritingMode) noexcept;
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::LegacyInlineTextBox)
  KRYS_NODISCARD static bool IsType(const krys::boo::LegacyInlineBox &box) noexcept
  {
    return box.IsInlineTextBox();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
