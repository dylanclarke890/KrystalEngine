#pragma once

#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Rendering/RenderBoxModelObject.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  namespace html
  {
    class HTMLElement;
  }

  /// @brief The line break of a `<br>`, or the line break opportunity of a `<wbr>`.
  // FIXME: Selection (canBeSelectionLeaf) comes with selection.
  class RenderLineBreak final : public RenderBoxModelObject
  {
    KRYS_NON_COPY_MOVABLE(RenderLineBreak);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderLineBreak);

  public:
    void Paint(PaintInfo &, const LayoutPoint &) noexcept final
    {
    }

    RenderLineBreak(html::HTMLElement &element, RenderStyle &&style) noexcept;
    ~RenderLineBreak() noexcept override;

    // FIXME: The lies here keep render tree dump based test results unchanged.
    KRYS_NODISCARD utf8_stringview RenderName() const noexcept final
    {
      return IsWBR() ? u8"RenderWordBreak" : u8"RenderBR";
    }

    KRYS_NODISCARD bool IsBR() const noexcept
    {
      return !HasWBRLineBreakFlag();
    }
    KRYS_NODISCARD bool IsWBR() const noexcept
    {
      return HasWBRLineBreakFlag();
    }
    KRYS_NODISCARD bool IsLineBreakOpportunity() const noexcept
    {
      return IsWBR();
    }

    KRYS_NODISCARD IntRect LinesBoundingBox() const noexcept;

    KRYS_NODISCARD int32 CaretMinOffset() const noexcept final
    {
      return 0;
    }
    KRYS_NODISCARD int32 CaretMaxOffset() const noexcept final
    {
      return 1;
    }

    KRYS_NODISCARD dom::PositionWithAffinity PositionForPoint(const LayoutPoint &point,
                                                              HitTestSource source) noexcept final;

    void BoundingRects(List<LayoutRect> &rects, const LayoutPoint &accumulatedOffset) const noexcept override;
    void AbsoluteQuads(List<FloatQuad> &quads, bool *wasFixed = nullptr) const noexcept override;

  protected:
    KRYS_NODISCARD RepaintRects LocalRectsForRepaint(RepaintOutlineBounds) const noexcept final
    {
      return {};
    }

  private:
    KRYS_NODISCARD bool RequiresLayer() const noexcept final
    {
      return false;
    }
    KRYS_NODISCARD LayoutUnit MarginTop() const noexcept final
    {
      return 0_lu;
    }
    KRYS_NODISCARD LayoutUnit MarginBottom() const noexcept final
    {
      return 0_lu;
    }
    KRYS_NODISCARD LayoutUnit MarginLeft() const noexcept final
    {
      return 0_lu;
    }
    KRYS_NODISCARD LayoutUnit MarginRight() const noexcept final
    {
      return 0_lu;
    }
    KRYS_NODISCARD LayoutUnit MarginBefore(WritingMode) const noexcept final
    {
      return 0_lu;
    }
    KRYS_NODISCARD LayoutUnit MarginAfter(WritingMode) const noexcept final
    {
      return 0_lu;
    }
    KRYS_NODISCARD LayoutUnit MarginStart(WritingMode) const noexcept final
    {
      return 0_lu;
    }
    KRYS_NODISCARD LayoutUnit MarginEnd(WritingMode) const noexcept final
    {
      return 0_lu;
    }
    KRYS_NODISCARD LayoutUnit OffsetWidth() const noexcept final
    {
      return LayoutUnit(LinesBoundingBox().Width());
    }
    KRYS_NODISCARD LayoutUnit OffsetHeight() const noexcept final
    {
      return LayoutUnit(LinesBoundingBox().Height());
    }
    KRYS_NODISCARD LayoutRect BorderBoundingBox() const noexcept final
    {
      return LayoutRect(LayoutPoint(), LinesBoundingBox().Size());
    }
    /// @brief Line breaks aren't sticky.
    KRYS_NODISCARD LayoutRect FrameRectForStickyPositioning() const noexcept final
    {
      krys_debug_assert(false);
      return {};
    }

    KRYS_NODISCARD bool CanHaveChildren() const noexcept final
    {
      return false;
    }

    void UpdateFromStyle() noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderLineBreak)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderLineBreak();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
