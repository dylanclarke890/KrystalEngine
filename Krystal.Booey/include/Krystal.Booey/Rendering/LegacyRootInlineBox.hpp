#pragma once

#include "Krystal.Booey/Rendering/LegacyInlineFlowBox.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include <algorithm>

namespace krys::boo
{
  class RenderBlockFlow;

  /// @brief The box for a whole line, which holds the boxes on it.
  class LegacyRootInlineBox : public LegacyInlineFlowBox,
                              public CanMakeWeakPtr<LegacyRootInlineBox>,
                              public CanMakeCheckedPtr<LegacyRootInlineBox>
  {
    KRYS_NON_COPY_MOVABLE(LegacyRootInlineBox);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LegacyRootInlineBox);

  private:
    LayoutUnit _lineTop;
    LayoutUnit _lineBottom;

    LayoutUnit _lineBoxTop;
    LayoutUnit _lineBoxBottom;

  public:
    explicit LegacyRootInlineBox(RenderBlockFlow &) noexcept;
    ~LegacyRootInlineBox() noexcept override = default;

    KRYS_NODISCARD RenderBlockFlow &BlockFlow() const noexcept;

    KRYS_NODISCARD LegacyRootInlineBox *NextRootBox() const noexcept
    {
      return Downcast<LegacyRootInlineBox>(_nextLineBox);
    }
    KRYS_NODISCARD LegacyRootInlineBox *PrevRootBox() const noexcept
    {
      return Downcast<LegacyRootInlineBox>(_prevLineBox);
    }

    void AdjustPosition(float dx, float dy) noexcept final;

    KRYS_NODISCARD LayoutUnit LineTop() const noexcept
    {
      return _lineTop;
    }
    KRYS_NODISCARD LayoutUnit LineBottom() const noexcept
    {
      return _lineBottom;
    }
    KRYS_NODISCARD LayoutUnit LineBoxWidth() const noexcept;

    KRYS_NODISCARD LayoutUnit LineBoxTop() const noexcept
    {
      return _lineBoxTop;
    }
    KRYS_NODISCARD LayoutUnit LineBoxBottom() const noexcept
    {
      return _lineBoxBottom;
    }
    KRYS_NODISCARD LayoutUnit LineBoxHeight() const noexcept
    {
      return LineBoxBottom() - LineBoxTop();
    }

    KRYS_NODISCARD LayoutUnit SelectionTop() const noexcept;
    KRYS_NODISCARD LayoutUnit SelectionBottom() const noexcept;
    KRYS_NODISCARD LayoutUnit SelectionHeight() const noexcept
    {
      return std::max<LayoutUnit>(LayoutUnit(), SelectionBottom() - SelectionTop());
    }

    KRYS_NODISCARD LayoutUnit SelectionTopAdjustedForPrecedingBlock() const noexcept;
    KRYS_NODISCARD LayoutUnit SelectionHeightAdjustedForPrecedingBlock() const noexcept
    {
      return std::max<LayoutUnit>(LayoutUnit(), SelectionBottom() - SelectionTopAdjustedForPrecedingBlock());
    }

    void SetLineTopBottomPositions(LayoutUnit top, LayoutUnit bottom, LayoutUnit lineBoxTop,
                                   LayoutUnit lineBoxBottom) noexcept
    {
      _lineTop = top;
      _lineBottom = bottom;
      _lineBoxTop = lineBoxTop;
      _lineBoxBottom = lineBoxBottom;
    }

    KRYS_NODISCARD RenderObject::HighlightState SelectionState() const noexcept final;
    KRYS_NODISCARD const LegacyInlineBox *FirstSelectedBox() const noexcept;
    KRYS_NODISCARD const LegacyInlineBox *LastSelectedBox() const noexcept;

    void RemoveLineBoxFromRenderObject() noexcept final;

    KRYS_NODISCARD FontBaseline BaselineType() const noexcept
    {
      return _baselineType;
    }

    KRYS_NODISCARD LayoutUnit LogicalTopVisualOverflow() const noexcept
    {
      return LegacyInlineFlowBox::LogicalTopVisualOverflow(LineTop());
    }
    KRYS_NODISCARD LayoutUnit LogicalBottomVisualOverflow() const noexcept
    {
      return LegacyInlineFlowBox::LogicalBottomVisualOverflow(LineBottom());
    }

  private:
    KRYS_NODISCARD bool IsRootInlineBox() const noexcept final
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::LegacyRootInlineBox)
  KRYS_NODISCARD static bool IsType(const krys::boo::LegacyInlineBox &box) noexcept
  {
    return box.IsRootInlineBox();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
