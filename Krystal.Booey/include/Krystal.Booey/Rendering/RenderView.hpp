#pragma once

#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class LocalFrameView;
  class RenderCounter;

  /// @brief Counters whose text needs updating, keyed by address; the weak pointer tells whether the counter
  /// is still alive.
  using RenderCounterSet = HashMap<const RenderCounter *, WeakPtr<RenderCounter>>;

  /// @brief The root of a document's render tree, shown in the document's view.
  // FIXME: Layout, painting, layers, selection, printing and the repaint and compositing bookkeeping come
  // with layout and painting.
  class RenderView final : public RenderBlockFlow
  {
    KRYS_NON_COPY_MOVABLE(RenderView);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderView);

  private:
    LocalFrameView &_frameView;
    uint64 _rendererCount {1};
    bool _hasQuotesNeedingUpdate {false};
    RenderCounterSet _countersNeedingUpdate;

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

    void Layout() noexcept override;

    KRYS_NODISCARD bool RequiresLayer() const noexcept override
    {
      return true;
    }

    KRYS_NODISCARD LogicalExtentComputedValues
      ComputeLogicalHeight(LayoutUnit logicalHeight, LayoutUnit logicalTop) const noexcept override;
    void UpdateLogicalWidth() noexcept override;
    KRYS_NODISCARD LayoutUnit
      AvailableLogicalHeight(AvailableLogicalHeightType heightType) const noexcept override;

    // The same as the FrameView's layoutHeight/layoutWidth but with null check guards.
    KRYS_NODISCARD int32 ViewHeight() const noexcept;
    KRYS_NODISCARD int32 ViewWidth() const noexcept;
    KRYS_NODISCARD int32 ViewLogicalWidth() const noexcept;
    KRYS_NODISCARD int32 ViewLogicalHeight() const noexcept;

    KRYS_NODISCARD LayoutUnit PageOrViewLogicalHeight() const noexcept;

    KRYS_NODISCARD bool ShouldUsePrintingLayout() const noexcept
    {
      // FIXME: Printing, once there is any.
      return false;
    }

    KRYS_NODISCARD bool HasQuotesNeedingUpdate() const noexcept
    {
      return _hasQuotesNeedingUpdate;
    }
    void SetHasQuotesNeedingUpdate(bool b) noexcept
    {
      _hasQuotesNeedingUpdate = b;
    }

    void AddCounterNeedingUpdate(RenderCounter &renderer) noexcept;
    KRYS_NODISCARD RenderCounterSet TakeCountersNeedingUpdate() noexcept;

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
