#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Calc/TreeEvaluation.hpp"

namespace krys::boo
{
  RenderInline::RenderInline(Type type, dom::Element &element, RenderStyle &&style) noexcept
      : RenderBoxModelObject(type, element, krys::move(style), TypeFlag::IsRenderInline, {})
  {
    SetChildrenInline(true);
    krys_debug_assert(IsRenderInline());
  }

  RenderInline::RenderInline(Type type, dom::Document &document, RenderStyle &&style) noexcept
      : RenderBoxModelObject(type, document, krys::move(style), TypeFlag::IsRenderInline, {})
  {
    SetChildrenInline(true);
    krys_debug_assert(IsRenderInline());
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderInline::~RenderInline() noexcept = default;

  void RenderInline::UpdateFromStyle() noexcept
  {
    RenderBoxModelObject::UpdateFromStyle();

    // FIXME: Support transforms and reflections on inline flows someday.
    SetHasTransformRelatedProperty(false);
  }

  utf8_stringview RenderInline::RenderName() const noexcept
  {
    if (IsRelativelyPositioned())
    {
      return u8"RenderInline (relative positioned)";
    }
    if (IsStickilyPositioned())
    {
      return u8"RenderInline (sticky positioned)";
    }
    // FIXME: Temporary hack while the new generated content system is being implemented.
    if (IsPseudoElement())
    {
      return u8"RenderInline (generated)";
    }
    if (IsAnonymous())
    {
      return u8"RenderInline (generated)";
    }
    return u8"RenderInline";
  }

  LayoutUnit RenderInline::InnerPaddingBoxWidth() const noexcept
  {
    krys_unreachable();
  }

  LayoutUnit RenderInline::InnerPaddingBoxHeight() const noexcept
  {
    krys_unreachable();
  }

  bool IsEmptyInline(const RenderInline &renderer) noexcept
  {
    for (auto &current : ChildrenOfType<RenderObject>(renderer))
    {
      if (current.IsFloatingOrOutOfFlowPositioned())
      {
        continue;
      }
      if (auto *text = DynamicDowncast<RenderText>(current))
      {
        if (!text->ContainsOnlyCollapsibleWhitespace())
        {
          return false;
        }
        continue;
      }
      auto *renderInline = DynamicDowncast<RenderInline>(current);
      if (!renderInline || !IsEmptyInline(*renderInline))
      {
        return false;
      }
    }
    return true;
  }

  namespace
  {
    KRYS_NODISCARD LayoutUnit ComputeMargin(const RenderInline *renderer, const style::MarginEdge &margin,
                                            const style::ZoomFactor &zoomFactor) noexcept
    {
      return style::EvaluateMinimum<LayoutUnit>(
        margin,
        [&] { return std::max<LayoutUnit>(0_lu, renderer->ContainingBlock()->ContentBoxLogicalWidth()); },
        zoomFactor);
    }
  }

  LayoutUnit RenderInline::MarginLeft() const noexcept
  {
    return ComputeMargin(this, Style().MarginLeft(), Style().UsedZoomForLength());
  }

  LayoutUnit RenderInline::MarginRight() const noexcept
  {
    return ComputeMargin(this, Style().MarginRight(), Style().UsedZoomForLength());
  }

  LayoutUnit RenderInline::MarginTop() const noexcept
  {
    return ComputeMargin(this, Style().MarginTop(), Style().UsedZoomForLength());
  }

  LayoutUnit RenderInline::MarginBottom() const noexcept
  {
    return ComputeMargin(this, Style().MarginBottom(), Style().UsedZoomForLength());
  }

  LayoutUnit RenderInline::MarginStart(WritingMode writingMode) const noexcept
  {
    return ComputeMargin(this, Style().MarginStart(writingMode), Style().UsedZoomForLength());
  }

  LayoutUnit RenderInline::MarginEnd(WritingMode writingMode) const noexcept
  {
    return ComputeMargin(this, Style().MarginEnd(writingMode), Style().UsedZoomForLength());
  }

  LayoutUnit RenderInline::MarginBefore(WritingMode writingMode) const noexcept
  {
    return ComputeMargin(this, Style().MarginBefore(writingMode), Style().UsedZoomForLength());
  }

  LayoutUnit RenderInline::MarginAfter(WritingMode writingMode) const noexcept
  {
    return ComputeMargin(this, Style().MarginAfter(writingMode), Style().UsedZoomForLength());
  }

  IntRect RenderInline::LinesBoundingBox() const noexcept
  {
    // FIXME: The bounding box of the inline's line boxes, once there's inline layout.
    return {};
  }

  bool RenderInline::RequiresLayer() const noexcept
  {
    // FIXME: Running accelerated animations too, once there are any.
    return IsInFlowPositioned() || CreatesGroup() || HasClipPath()
           || Style().GetWillChange().CanCreateStackingContext()
           || RequiresRenderingConsolidationForViewTransition();
  }
}
