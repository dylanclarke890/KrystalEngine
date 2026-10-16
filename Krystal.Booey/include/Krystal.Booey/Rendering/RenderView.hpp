#pragma once

#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  class LocalFrameView;

  /// @brief The root of a document's render tree, shown in the document's view.
  // FIXME: Layout, painting, layers, selection, printing and the repaint, compositing and quote/counter
  // bookkeeping come with layout and painting.
  class RenderView final : public RenderBlockFlow
  {
    KRYS_NON_COPY_MOVABLE(RenderView);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderView);

  private:
    LocalFrameView &_frameView;
    uint64 _rendererCount {1};
    // FIXME: Stands in for the layout bits renderers have, until there's layout: a new render view needs
    // layout, and style changes don't mark it.
    bool _needsLayout {true};

  public:
    RenderView(dom::Document &document, RenderStyle &&style) noexcept;
    ~RenderView() noexcept override;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderView";
    }

    KRYS_NODISCARD bool IsChildAllowed(const RenderObject &child,
                                       const RenderStyle &) const noexcept override;

    KRYS_NODISCARD float ZoomFactor() const noexcept;

    KRYS_NODISCARD LocalFrameView &FrameView() const noexcept
    {
      return _frameView;
    }

    KRYS_NODISCARD uint64 RendererCount() const noexcept
    {
      return _rendererCount;
    }
    void DidCreateRenderer() noexcept
    {
      ++_rendererCount;
    }
    void WillDestroyRenderer() noexcept
    {
      --_rendererCount;
    }

    KRYS_NODISCARD bool NeedsLayout() const noexcept
    {
      return _needsLayout;
    }
    void SetNeedsLayout() noexcept
    {
      _needsLayout = true;
    }
    void ClearNeedsLayout() noexcept
    {
      _needsLayout = false;
    }

    KRYS_NODISCARD FloatSize SizeForCSSSmallViewportUnits() const noexcept;
    KRYS_NODISCARD FloatSize SizeForCSSLargeViewportUnits() const noexcept;
    KRYS_NODISCARD FloatSize SizeForCSSDynamicViewportUnits() const noexcept;
    KRYS_NODISCARD FloatSize SizeForCSSDefaultViewportUnits() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderView)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderView();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
