#pragma once

#include "Krystal.Booey/Graphics/LayoutPoint.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/ListHashSet.hpp"

namespace krys::boo
{
  class RenderBlock;
  class RenderBlockFlow;
  class RenderBox;
  class RenderInline;
  struct PaintInfo;

  namespace inline_display
  {
    struct Box;
  }

  namespace layout
  {
    class ElementBox;
  }
}

namespace krys::boo::layout_integration
{
  class InlineContent;

  /// @brief Paints a block's inline content for one paint phase: its display boxes in order (text, inline
  /// boxes, atomic inlines and blocks in inline), culled to the paint rect.
  /// FIXME: Accessibility bounds come with accessibility.
  class InlineContentPainter
  {
    KRYS_NON_COPY_MOVABLE(InlineContentPainter);

  private:
    PaintInfo &_paintInfo;
    const LayoutPoint _paintOffset;
    LayoutRect _damageRect;
    const RenderInline *_inlineBoxWithLayer {nullptr};
    const InlineContent &_inlineContent;
    const RenderBlockFlow &_root;
    ListHashSet<const RenderInline *> _outlineObjects;

  public:
    /// @param inlineBoxWithLayer The inline box whose layer is being painted, if any: only its content is
    /// painted.
    InlineContentPainter(PaintInfo &paintInfo, const LayoutPoint &paintOffset,
                         const RenderInline *inlineBoxWithLayer, const InlineContent &inlineContent,
                         const RenderBlockFlow &root) noexcept;

    void Paint() noexcept;

  private:
    void PaintDisplayBox(const inline_display::Box &box) noexcept;
    void PaintEllipsis(size_t lineIndex) noexcept;
    KRYS_NODISCARD LayoutPoint FlippedContentOffsetIfNeeded(const RenderBox &childRenderer) const noexcept;
    KRYS_NODISCARD const RenderBlock &Root() const noexcept;
  };

  /// @brief Which display boxes a layer's painting includes: everything outside inline boxes with
  /// self-painting layers (which paint themselves), or only the content of the inline box whose layer is
  /// being painted.
  class LayerPaintScope
  {
  private:
    const layout::ElementBox *const _inlineBoxWithLayer;
    const layout::ElementBox *_currentExcludedInlineBox {nullptr};

  public:
    explicit LayerPaintScope(const RenderInline *inlineBoxWithLayer) noexcept;

    /// @brief Whether @p box is painted, noting an inline box with a self-painting layer as excluded.
    KRYS_NODISCARD bool TestIsIncludesAndUpdate(const inline_display::Box &box) noexcept;
  };
}
