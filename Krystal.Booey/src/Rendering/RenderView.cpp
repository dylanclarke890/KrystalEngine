#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"

namespace krys::boo
{
  RenderView::RenderView(dom::Document &document, RenderStyle &&style) noexcept
      : RenderElement(Type::View, document, krys::move(style),
                      {TypeFlag::IsLayerModelObject, TypeFlag::IsBoxModelObject, TypeFlag::IsBox,
                       TypeFlag::IsRenderBlock},
                      {}),
        _frameView(*document.View())
  {
    // FIXME: We should find a way to enforce this at compile time.
    krys_debug_assert(document.View());

    // FIXME: Not inline, no preferred widths, and positioned at 0,0, once there's layout.

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
}
