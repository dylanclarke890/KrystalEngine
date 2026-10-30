#pragma once

#include "Krystal.Booey/Core/RectEdges.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorBox.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  class RenderBoxModelObject;
  class RenderInline;
}

namespace krys::boo::inline_iterator
{
  class InlineBoxIterator;

  /// @brief An inline box (or the root inline box) on a line: the part of an inline renderer on that line.
  class InlineBox : public Box
  {
  public:
    explicit InlineBox(PathVariant &&path) noexcept : Box(krys::move(path))
    {
    }

    KRYS_NODISCARD const RenderBoxModelObject &Renderer() const noexcept;

    // FIXME: Remove. For intermediate porting steps only.
    KRYS_NODISCARD const LegacyInlineFlowBox *GetLegacyInlineBox() const noexcept
    {
      return Downcast<LegacyInlineFlowBox>(Box::GetLegacyInlineBox());
    }

    /// @brief Whether each edge closes the inline box (rather than continuing on another line).
    KRYS_NODISCARD RectEdges<bool> ClosedEdges() const noexcept;

    KRYS_NODISCARD InlineBoxIterator NextInlineBoxLineRightward() const noexcept;
    KRYS_NODISCARD InlineBoxIterator NextInlineBoxLineLeftward() const noexcept;
    KRYS_NODISCARD InlineBoxIterator Iterator() const noexcept;

    KRYS_NODISCARD LeafBoxIterator FirstLeafBox() const noexcept;
    KRYS_NODISCARD LeafBoxIterator LastLeafBox() const noexcept;
    KRYS_NODISCARD LeafBoxIterator EndLeafBox() const noexcept;

    KRYS_NODISCARD bool IsSplit() const noexcept;

    /// @brief The boxes in this inline box, as [begin, end) iterators.
    KRYS_NODISCARD Pair<BoxIterator, BoxIterator> Descendants() const noexcept;
  };

  /// @brief Iterates the inline boxes generated for the same inline renderer.
  class InlineBoxIterator : public BoxIterator
  {
  public:
    InlineBoxIterator() noexcept = default;
    explicit InlineBoxIterator(Box::PathVariant &&path) noexcept : BoxIterator(krys::move(path))
    {
    }
    explicit InlineBoxIterator(const Box &box) noexcept : BoxIterator(box)
    {
    }

    KRYS_NODISCARD const InlineBox &operator*() const noexcept
    {
      return Get();
    }
    KRYS_NODISCARD const InlineBox *operator->() const noexcept
    {
      return &Get();
    }

    InlineBoxIterator &TraverseInlineBoxLineRightward() noexcept;
    InlineBoxIterator &TraverseInlineBoxLineLeftward() noexcept;

  private:
    KRYS_NODISCARD const InlineBox &Get() const noexcept
    {
      return static_cast<const InlineBox &>(_box);
    }
  };

  KRYS_NODISCARD InlineBoxIterator LineLeftmostInlineBoxFor(const RenderInline &renderInline) noexcept;
  KRYS_NODISCARD InlineBoxIterator FirstRootInlineBoxFor(const RenderBlockFlow &block) noexcept;
  KRYS_NODISCARD InlineBoxIterator InlineBoxFor(const LegacyInlineFlowBox &legacyInlineFlowBox) noexcept;
  KRYS_NODISCARD InlineBoxIterator InlineBoxFor(const layout_integration::InlineContent &content,
                                                const inline_display::Box &box) noexcept;
  KRYS_NODISCARD InlineBoxIterator InlineBoxFor(const layout_integration::InlineContent &content,
                                                size_t boxIndex) noexcept;
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::inline_iterator::InlineBox)
  KRYS_NODISCARD static bool IsType(const krys::boo::inline_iterator::Box &box) noexcept
  {
    return box.IsInlineBox();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
