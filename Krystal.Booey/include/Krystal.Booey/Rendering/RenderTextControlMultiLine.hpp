#pragma once

#include "Krystal.Booey/Rendering/RenderTextControl.hpp"

namespace krys::boo::html
{
  class HTMLTextAreaElement;
}

namespace krys::boo
{
  /// @brief The renderer of a `<textarea>`: lines of text that wrap and scroll, sized by its `cols` and
  /// `rows`.
  class RenderTextControlMultiLine final : public RenderTextControl
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderTextControlMultiLine);

  public:
    RenderTextControlMultiLine(html::HTMLTextAreaElement &element, RenderStyle &&style) noexcept;
    ~RenderTextControlMultiLine() noexcept override;

    KRYS_NODISCARD html::HTMLTextAreaElement &TextAreaElement() const noexcept;

  private:
    bool NodeAtPoint(const HitTestRequest &request, HitTestResult &result,
                     const HitTestLocation &locationInContainer, const LayoutPoint &accumulatedOffset,
                     HitTestAction hitTestAction) noexcept override;

    KRYS_NODISCARD float GetAverageCharWidth() noexcept override;
    KRYS_NODISCARD LayoutUnit PreferredContentLogicalWidth(float charWidth) const noexcept override;
    KRYS_NODISCARD LayoutUnit
      ComputeControlLogicalHeight(LayoutUnit lineHeight, LayoutUnit nonContentHeight) const noexcept override;

    void LayoutExcludedChildren(RelayoutChildren relayoutChildren) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderTextControlMultiLine)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderTextControlMultiLine();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
