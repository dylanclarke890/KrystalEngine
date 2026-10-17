#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/RenderCounter.hpp"

namespace krys::boo
{
  RenderView::RenderView(dom::Document &document, RenderStyle &&style) noexcept
      : RenderBlockFlow(Type::View, document, krys::move(style)), _frameView(*document.View())
  {
    // FIXME: We should find a way to enforce this at compile time.
    krys_debug_assert(document.View());

    // init RenderObject attributes
    SetInline(false);

    // FIXME: The preferred widths, once there's layout.

    SetPositionState(PositionType::Absolute); // to 0,0 :)

    krys_debug_assert(IsRenderView());
  }

  RenderView::~RenderView() noexcept
  {
    krys_debug_assert_msg(!_rendererCount, "All renderers should be in the process of being deleted.");
  }

  bool RenderView::IsChildAllowed(const RenderObject &child, const RenderStyle &) const noexcept
  {
    return child.IsRenderBox();
  }

  float RenderView::ZoomFactor() const noexcept
  {
    return _frameView.Frame().PageZoomFactor();
  }

  FloatSize RenderView::SizeForCSSSmallViewportUnits() const noexcept
  {
    return _frameView.SizeForCSSSmallViewportUnits();
  }

  FloatSize RenderView::SizeForCSSLargeViewportUnits() const noexcept
  {
    return _frameView.SizeForCSSLargeViewportUnits();
  }

  FloatSize RenderView::SizeForCSSDynamicViewportUnits() const noexcept
  {
    return _frameView.SizeForCSSDynamicViewportUnits();
  }

  FloatSize RenderView::SizeForCSSDefaultViewportUnits() const noexcept
  {
    return _frameView.SizeForCSSDefaultViewportUnits();
  }

  void RenderView::AddCounterNeedingUpdate(RenderCounter &renderer) noexcept
  {
    _countersNeedingUpdate.try_emplace(&renderer, CreateWeakPtr(&renderer));
  }

  RenderCounterSet RenderView::TakeCountersNeedingUpdate() noexcept
  {
    return std::exchange(_countersNeedingUpdate, {});
  }

  void RenderView::Layout() noexcept
  {
    // FIXME: Printing (the page size and the preferred widths it sets), and relaying out the children whose
    // height depends on the view when its size changes, once there's box geometry.

    if (!NeedsLayout())
    {
      return;
    }

    // FIXME: The layout state, and the initial containing block's size, once there's box geometry.
    RenderBlockFlow::Layout();
  }
}
