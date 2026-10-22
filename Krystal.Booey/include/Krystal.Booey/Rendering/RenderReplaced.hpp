#pragma once

#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Graphics/LayoutSize.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  enum class ShouldComputePreferred : bool
  {
    ComputeActual,
    ComputePreferred
  };

  /// @brief A box whose content is outside CSS: images, media, frames, form controls.
  // FIXME: Selection and hit testing; media and the other replaced elements come with their renderers.
  class RenderReplaced : public RenderBox
  {
    KRYS_NON_COPY_MOVABLE(RenderReplaced);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderReplaced);

  private:
    mutable LayoutSize _intrinsicSize;

  public:
    void Paint(PaintInfo &, const LayoutPoint &) noexcept override;

    ~RenderReplaced() noexcept override;

    KRYS_NODISCARD bool CanHaveChildren() const noexcept override
    {
      return false;
    }

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderReplaced";
    }

    KRYS_NODISCARD virtual bool ShouldRespectZeroIntrinsicWidth() const noexcept;

    KRYS_NODISCARD dom::PositionWithAffinity PositionForPoint(const LayoutPoint &point,
                                                              HitTestSource source) noexcept final;

    KRYS_NODISCARD LayoutRect ReplacedContentRect(const LayoutSize &intrinsicSize) const noexcept;
    KRYS_NODISCARD LayoutRect ReplacedContentRect() const noexcept
    {
      return ReplacedContentRect(IntrinsicSize());
    }

    bool SetNeedsLayoutIfNeededAfterIntrinsicSizeChange() noexcept;

    KRYS_NODISCARD LayoutSize IntrinsicSize() const noexcept final;
    KRYS_NODISCARD FloatSize IntrinsicRatio() const noexcept;

    KRYS_NODISCARD bool ShouldInvalidatePreferredWidths() const noexcept override;

    KRYS_NODISCARD double ComputeIntrinsicAspectRatio() const noexcept;

    KRYS_NODISCARD virtual bool PaintsContent() const noexcept
    {
      return true;
    }

    KRYS_NODISCARD LayoutUnit
      ComputeReplacedLogicalHeightUsing(const style::PreferredSize &logicalHeight) const noexcept;
    KRYS_NODISCARD LayoutUnit
      ComputeReplacedLogicalHeightUsing(const style::MinimumSize &logicalHeight) const noexcept;
    KRYS_NODISCARD LayoutUnit
      ComputeReplacedLogicalHeightUsing(const style::MaximumSize &logicalHeight) const noexcept;

    KRYS_NODISCARD virtual LayoutUnit ComputeReplacedLogicalWidth(
      ShouldComputePreferred shouldComputePreferred = ShouldComputePreferred::ComputeActual) const noexcept;
    KRYS_NODISCARD virtual LayoutUnit
      ComputeReplacedLogicalHeight(Maybe<LayoutUnit> estimatedUsedWidth = null) const noexcept;

    KRYS_NODISCARD bool ReplacedMinLogicalHeightComputesAsNone() const noexcept;

    void ComputeReplacedOutOfFlowPositionedLogicalWidth(
      LogicalExtentComputedValues &computedValues) const noexcept;
    void ComputeReplacedOutOfFlowPositionedLogicalHeight(
      LogicalExtentComputedValues &computedValues) const noexcept;
    KRYS_NODISCARD bool ReplacedMaxLogicalHeightComputesAsNone() const noexcept;

  protected:
    RenderReplaced(Type type, dom::Element &element, RenderStyle &&style,
                   EnumSet<ReplacedFlag> flags = {}) noexcept;

    KRYS_NODISCARD RepaintRects
      LocalRectsForRepaint(RepaintOutlineBounds repaintOutlineBounds) const noexcept override;
    RenderReplaced(Type type, dom::Element &element, RenderStyle &&style, const LayoutSize &intrinsicSize,
                   EnumSet<ReplacedFlag> flags = {}) noexcept;
    RenderReplaced(Type type, dom::Document &document, RenderStyle &&style, const LayoutSize &intrinsicSize,
                   EnumSet<ReplacedFlag> flags = {}) noexcept;

    void Layout() noexcept override;

    KRYS_NODISCARD bool ShouldPaint(PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept;

    /// @brief Paints the replaced content, in the foreground and selection phases, inside the box's rounded
    /// content box.
    virtual void PaintReplaced(PaintInfo &, const LayoutPoint &) noexcept
    {
    }

    void ComputeIntrinsicLogicalWidths(LayoutUnit &minLogicalWidth,
                                       LayoutUnit &maxLogicalWidth) const noexcept final;

    KRYS_NODISCARD virtual LayoutUnit MinimumReplacedHeight() const noexcept
    {
      return 0_lu;
    }

    void StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept override;

    KRYS_NODISCARD virtual FloatSize ComputeIntrinsicSize() const noexcept;
    KRYS_NODISCARD virtual FloatSize PreferredAspectRatio() const noexcept;

    void SetIntrinsicSize(const LayoutSize &intrinsicSize) noexcept
    {
      _intrinsicSize = intrinsicSize;
    }
    virtual void IntrinsicSizeChanged() noexcept;
    KRYS_NODISCARD virtual bool HasRelativeIntrinsicLogicalWidth() const noexcept
    {
      return false;
    }

    void WillBeDestroyed() noexcept override;

    KRYS_NODISCARD LayoutUnit ComputeReplacedLogicalWidthRespectingMinMaxWidth(
      LayoutUnit logicalWidth,
      ShouldComputePreferred shouldComputePreferred = ShouldComputePreferred::ComputeActual) const noexcept;

  private:
    KRYS_NODISCARD LayoutUnit ComputeConstrainedLogicalWidth() const noexcept;

    template <typename SizeType>
    KRYS_NODISCARD LayoutUnit ComputeReplacedLogicalWidthUsing(const SizeType &logicalWidth) const noexcept;
    template <typename SizeType>
    KRYS_NODISCARD LayoutUnit
      ComputeReplacedLogicalHeightUsingGeneric(const SizeType &logicalHeight) const noexcept;
    KRYS_NODISCARD LayoutUnit
      ComputeReplacedLogicalHeightRespectingMinMaxHeight(LayoutUnit logicalHeight) const noexcept;
    KRYS_NODISCARD bool
      ReplacedMinMaxLogicalHeightComputesAsNone(const auto &logicalHeight,
                                                const auto &initialLogicalHeight) const noexcept;

    void ComputeAspectRatioAdjustedIntrinsicLogicalWidths(LayoutUnit &minLogicalWidth,
                                                          LayoutUnit &maxLogicalWidth) const noexcept;

    KRYS_NODISCARD virtual RenderBox *EmbeddedContentBox() const noexcept
    {
      return nullptr;
    }

    void ComputePreferredLogicalWidths() noexcept final;

    void ComputeAspectRatioInformationForRenderBox(RenderBox *contentRenderer, FloatSize &constrainedSize,
                                                   FloatSize &preferredAspectRatio) const noexcept;
    void ComputeIntrinsicSizesConstrainedByTransferredMinMaxSizes(RenderBox *contentRenderer,
                                                                  FloatSize &intrinsicSize,
                                                                  FloatSize &intrinsicRatio) const noexcept;

    KRYS_NODISCARD bool HasReplacedLogicalHeight() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderReplaced)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderReplaced();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
