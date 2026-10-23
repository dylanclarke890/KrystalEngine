#pragma once

#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderFlexibleBox.hpp"

namespace krys::boo::html
{
  class HTMLTextFormControlElement;
  class TextControlInnerTextElement;
}

namespace krys::boo
{
  /// @brief The renderer of a text control: a text field or a text area, sized by its `size` or its `cols`
  /// and `rows`, holding its user agent shadow tree.
  class RenderTextControl : public RenderBlockFlow
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderTextControl);

  public:
    ~RenderTextControl() noexcept override;

    KRYS_NODISCARD html::HTMLTextFormControlElement &TextFormControlElement() const noexcept;

  protected:
    RenderTextControl(Type type, html::HTMLTextFormControlElement &element, RenderStyle &&style) noexcept;

    // This convenience function should not be made public because innerTextElement may outlive the render
    // tree.
    KRYS_NODISCARD RefPtr<html::TextControlInnerTextElement> InnerTextElement() const noexcept;

    KRYS_NODISCARD int32 ScrollbarThickness() const noexcept;

    void StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept override;

    void HitInnerTextElement(HitTestResult &result, const LayoutPoint &pointInContainer,
                             const LayoutPoint &accumulatedOffset) noexcept;

    KRYS_NODISCARD float ScaleEmToUnits(int32 x) const noexcept;

    KRYS_NODISCARD virtual float GetAverageCharWidth() noexcept;
    KRYS_NODISCARD virtual LayoutUnit PreferredContentLogicalWidth(float charWidth) const noexcept = 0;
    KRYS_NODISCARD virtual LayoutUnit
      ComputeControlLogicalHeight(LayoutUnit lineHeight, LayoutUnit nonContentHeight) const noexcept = 0;

    KRYS_NODISCARD LogicalExtentComputedValues
      ComputeLogicalHeight(LayoutUnit logicalHeight, LayoutUnit logicalTop) const noexcept override;
    void LayoutExcludedChildren(RelayoutChildren relayoutChildren) noexcept override;

  private:
    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderTextControl";
    }

    void ComputeIntrinsicLogicalWidths(LayoutUnit &minLogicalWidth,
                                       LayoutUnit &maxLogicalWidth) const noexcept override;
    void ComputePreferredLogicalWidths() noexcept override;

    KRYS_NODISCARD bool CanHaveGeneratedChildren() const noexcept override
    {
      return false;
    }

    KRYS_NODISCARD bool CanBeProgramaticallyScrolled() const noexcept override
    {
      return true;
    }
  };

  /// @brief The renderer of a text field's inner container, which lays out the inner text and the decorations
  /// as a flex box, but with a block's baseline so that fields of every type line up.
  class RenderTextControlInnerContainer final : public RenderFlexibleBox
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderTextControlInnerContainer);

  public:
    RenderTextControlInnerContainer(dom::Element &element, RenderStyle &&style) noexcept;
    ~RenderTextControlInnerContainer() noexcept override;

    KRYS_NODISCARD Maybe<LayoutUnit> FirstLineBaseline() const noexcept override
    {
      return RenderBlock::FirstLineBaseline();
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderTextControl)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderTextControl();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderTextControlInnerContainer)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderTextControlInnerContainer();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
