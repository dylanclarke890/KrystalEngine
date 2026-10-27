#pragma once

#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGBlock.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGTextChunk.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGTextLayoutAttributesBuilder.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo::inline_iterator
{
  class InlineBoxIterator;
}

namespace krys::boo::svg
{
  class SVGTextElement;
}

namespace krys::boo
{
  class RenderSVGInlineText;
  class SVGRootInlineBox;
  class SVGTextLayoutEngine;

  /// @brief The renderer of a `<text>`: a block whose inline content is laid out character by character by
  /// the SVG text layout.
  class RenderSVGText final : public RenderSVGBlock
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderSVGText);

  private:
    bool _needsReordering : 1 {false};
    bool _needsPositioningValuesUpdate : 1 {false};
    bool _needsTransformUpdate : 1 {true};
    bool _needsTextMetricsUpdate : 1 {false};
    // Needed to distinguish between when we perform a full pass of layout and EverHadLayout (which can be set
    // be content visibility for skipped content).
    bool _hasPerformedLayout : 1 {false};
    AffineTransform _localTransform;
    SVGTextLayoutAttributesBuilder _layoutAttributesBuilder;
    List<SVGTextLayoutAttributes *> _layoutAttributes;
    FloatRect _objectBoundingBox;

  public:
    RenderSVGText(svg::SVGTextElement &, RenderStyle &&) noexcept;
    ~RenderSVGText() noexcept override;

    KRYS_NODISCARD svg::SVGTextElement &TextElement() const noexcept;

    KRYS_NODISCARD bool IsChildAllowed(const RenderObject &, const RenderStyle &) const noexcept override;

    void SetNeedsPositioningValuesUpdate() noexcept
    {
      _needsPositioningValuesUpdate = true;
    }
    void SetNeedsTextMetricsUpdate() noexcept
    {
      _needsTextMetricsUpdate = true;
    }
    void SetNeedsTransformUpdate() noexcept override
    {
      _needsTransformUpdate = true;
    }

    KRYS_NODISCARD static RenderSVGText *LocateRenderSVGTextAncestor(RenderObject &) noexcept;
    KRYS_NODISCARD static const RenderSVGText *LocateRenderSVGTextAncestor(const RenderObject &) noexcept;

    KRYS_NODISCARD bool NeedsReordering() const noexcept
    {
      return _needsReordering;
    }
    KRYS_NODISCARD List<SVGTextLayoutAttributes *> &LayoutAttributes() noexcept
    {
      return _layoutAttributes;
    }

    void SubtreeChildWasAdded(RenderObject *) noexcept;
    void SubtreeChildWillBeRemoved(RenderObject *,
                                   SmallList<SVGTextLayoutAttributes *, 2> &affectedAttributes) noexcept;
    void SubtreeChildWasRemoved(const SmallList<SVGTextLayoutAttributes *, 2> &affectedAttributes) noexcept;
    void SubtreeTextDidChange(RenderSVGInlineText *) noexcept;

    KRYS_NODISCARD FloatRect ObjectBoundingBox() const noexcept final
    {
      return _objectBoundingBox;
    }
    KRYS_NODISCARD FloatRect StrokeBoundingBox() const noexcept final;
    KRYS_NODISCARD bool IsObjectBoundingBoxValid() const noexcept;
    KRYS_NODISCARD FloatRect RepaintRectInLocalCoordinates(
      RepaintRectCalculation = RepaintRectCalculation::Fast) const noexcept final;
    KRYS_NODISCARD FloatRect DecoratedBoundingBox() const noexcept final;

    void UpdatePositionAndOverflow(const FloatRect &) noexcept;

    KRYS_NODISCARD SVGRootInlineBox *LegacyRootBox() const noexcept;

  private:
    void GraphicsElement() const = delete;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderSVGText";
    }

    void Paint(PaintInfo &, const LayoutPoint &) noexcept override;
    void PaintInlineChildren(PaintInfo &, const LayoutPoint &) noexcept override;

    bool HitTestInlineChildren(const HitTestRequest &, HitTestResult &,
                               const HitTestLocation &locationInContainer,
                               const LayoutPoint &accumulatedOffset, HitTestAction) noexcept override;

    KRYS_NODISCARD dom::PositionWithAffinity PositionForPoint(const LayoutPoint &,
                                                              HitTestSource) noexcept override;

    KRYS_NODISCARD bool RequiresLayer() const noexcept override
    {
      return false;
    }
    void Layout() noexcept override;

    void ComputePerCharacterLayoutInformation() noexcept;
    void LayoutCharactersInTextBoxes(const inline_iterator::InlineBoxIterator &,
                                     SVGTextLayoutEngine &) noexcept;
    FloatRect LayoutChildBoxes(LegacyInlineFlowBox *, SVGTextFragmentMap &) noexcept;
    void LayoutRootBox(const FloatRect &) noexcept;
    void ReorderValueListsToLogicalOrder() noexcept;

    void WillBeDestroyed() noexcept override;

    void StyleDidChange(style::Difference, const RenderStyle *oldStyle) noexcept final;

    bool NodeAtFloatPoint(const HitTestRequest &, HitTestResult &, const FloatPoint &pointInParent,
                          HitTestAction) noexcept override;
    KRYS_NODISCARD const AffineTransform &LocalToParentTransform() const noexcept override
    {
      return _localTransform;
    }
    KRYS_NODISCARD AffineTransform LocalTransform() const noexcept override
    {
      return _localTransform;
    }

    KRYS_NODISCARD bool ShouldHandleSubtreeMutations() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderSVGText)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderSVGText();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
