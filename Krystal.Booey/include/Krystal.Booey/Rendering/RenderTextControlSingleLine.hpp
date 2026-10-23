#pragma once

#include "Krystal.Booey/Rendering/RenderTextControl.hpp"

namespace krys::boo::html
{
  class HTMLElement;
  class HTMLInputElement;
}

namespace krys::boo
{
  class RenderTextControlInnerBlock;

  /// @brief The renderer of a text field: one line of text, centred in the field's content box, scrolled
  /// sideways by the inner text block (whose scroll position the field reports as its own).
  class RenderTextControlSingleLine : public RenderTextControl
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderTextControlSingleLine);

  public:
    RenderTextControlSingleLine(Type type, html::HTMLInputElement &element, RenderStyle &&style) noexcept;
    ~RenderTextControlSingleLine() noexcept override;

    KRYS_NODISCARD RenderTextControlInnerBlock *InnerTextRenderer() const noexcept;

    KRYS_NODISCARD bool InnerTextElementHasNonVisibleOverflow() const noexcept;

    // Subclassed to forward to our inner div.
    KRYS_NODISCARD int32 ScrollLeft() const noexcept override;
    KRYS_NODISCARD int32 ScrollTop() const noexcept override;
    KRYS_NODISCARD int32 ScrollWidth() const noexcept override;
    KRYS_NODISCARD int32 ScrollHeight() const noexcept override;
    void SetScrollLeft(int32 newLeft, const ScrollPositionChangeOptions &options) noexcept override;
    void SetScrollTop(int32 newTop, const ScrollPositionChangeOptions &options) noexcept override;
    void SetScrollPosition(const ScrollPosition &position,
                           const ScrollPositionChangeOptions &options) noexcept override;
    bool Scroll(ScrollDirection direction, ScrollGranularity granularity, uint32 stepCount = 1,
                dom::Element **stopElement = nullptr) noexcept final;
    bool LogicalScroll(ScrollLogicalDirection direction, ScrollGranularity granularity, uint32 stepCount = 1,
                       dom::Element **stopElement = nullptr) noexcept final;

  protected:
    KRYS_NODISCARD html::HTMLElement *ContainerElement() const noexcept;
    KRYS_NODISCARD html::HTMLElement *InnerBlockElement() const noexcept;
    KRYS_NODISCARD html::HTMLInputElement &InputElement() const noexcept;

  private:
    KRYS_NODISCARD bool HasControlClip() const noexcept override;
    KRYS_NODISCARD LayoutRect ControlClipRect(const LayoutPoint &additionalOffset) const noexcept override;

    void Layout() noexcept override;

    bool NodeAtPoint(const HitTestRequest &request, HitTestResult &result,
                     const HitTestLocation &locationInContainer, const LayoutPoint &accumulatedOffset,
                     HitTestAction hitTestAction) noexcept override;

    void Autoscroll(const IntPoint &positionInWindow) noexcept override;

    KRYS_NODISCARD float GetAverageCharWidth() noexcept override;
    KRYS_NODISCARD LayoutUnit PreferredContentLogicalWidth(float charWidth) const noexcept override;
    KRYS_NODISCARD LayoutUnit
      ComputeControlLogicalHeight(LayoutUnit lineHeight, LayoutUnit nonContentHeight) const noexcept override;

    void StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept override;

    KRYS_NODISCARD html::HTMLElement *InnerSpinButtonElement() const noexcept;
  };

  /// @brief The renderer of a text control's inner text element: a block that always has a line, so the caret
  /// has somewhere to be when the control is empty.
  class RenderTextControlInnerBlock final : public RenderBlockFlow
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderTextControlInnerBlock);

  public:
    RenderTextControlInnerBlock(dom::Element &element, RenderStyle &&style) noexcept;
    ~RenderTextControlInnerBlock() noexcept override;

  private:
    KRYS_NODISCARD bool HasLineIfEmpty() const noexcept override
    {
      return true;
    }

    KRYS_NODISCARD bool CanBeProgramaticallyScrolled() const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderTextControlSingleLine)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderTextControlSingleLine();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderTextControlInnerBlock)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderTextControlInnerBlock();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
