#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorInlineBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationLineLayout.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::inline_iterator
{
  const RenderBoxModelObject &InlineBox::Renderer() const noexcept
  {
    return Downcast<RenderBoxModelObject>(Box::Renderer());
  }

  RectEdges<bool> InlineBox::ClosedEdges() const noexcept
  {
    // FIXME: Layout knows the answer to this question so we should consult it.
    RectEdges<bool> closedEdges {true};
    if (Style().GetBoxDecorationBreak() == BoxDecorationBreak::Clone)
    {
      return closedEdges;
    }
    // FIXME: Continuations, once there are any.
    auto writingMode = Style().GetWritingMode();
    bool isFirst = !NextInlineBoxLineLeftward();
    bool isLast = !NextInlineBoxLineRightward();
    closedEdges.SetStart(isFirst, writingMode);
    closedEdges.SetEnd(isLast, writingMode);
    return closedEdges;
  }

  InlineBoxIterator InlineBox::NextInlineBoxLineRightward() const noexcept
  {
    return InlineBoxIterator(*this).TraverseInlineBoxLineRightward();
  }

  InlineBoxIterator InlineBox::NextInlineBoxLineLeftward() const noexcept
  {
    return InlineBoxIterator(*this).TraverseInlineBoxLineLeftward();
  }

  InlineBoxIterator InlineBox::Iterator() const noexcept
  {
    return InlineBoxIterator {*this};
  }

  LeafBoxIterator InlineBox::FirstLeafBox() const noexcept
  {
    return SwitchOn(_pathVariant,
                    [](const auto &path) { return LeafBoxIterator {path.FirstLeafBoxForInlineBox()}; });
  }

  LeafBoxIterator InlineBox::LastLeafBox() const noexcept
  {
    return SwitchOn(_pathVariant,
                    [](const auto &path) { return LeafBoxIterator {path.LastLeafBoxForInlineBox()}; });
  }

  LeafBoxIterator InlineBox::EndLeafBox() const noexcept
  {
    if (auto last = LastLeafBox())
    {
      return last->NextLineRightwardOnLine();
    }
    return {};
  }

  bool InlineBox::IsSplit() const noexcept
  {
    return NextInlineBoxLineRightward() || NextInlineBoxLineLeftward();
  }

  Pair<BoxIterator, BoxIterator> InlineBox::Descendants() const noexcept
  {
    BoxIterator begin(*this);
    begin.TraverseLineRightwardOnLine();

    BoxIterator end(*this);
    end.TraverseLineRightwardOnLineSkippingChildren();

    return {begin, end};
  }

  InlineBoxIterator &InlineBoxIterator::TraverseInlineBoxLineRightward() noexcept
  {
    SwitchOn(_box._pathVariant, [](auto &path) { path.TraverseNextInlineBox(); });
    return *this;
  }

  InlineBoxIterator &InlineBoxIterator::TraverseInlineBoxLineLeftward() noexcept
  {
    SwitchOn(_box._pathVariant, [](auto &path) { path.TraversePreviousInlineBox(); });
    return *this;
  }

  InlineBoxIterator LineLeftmostInlineBoxFor(const RenderInline &renderInline) noexcept
  {
    if (auto *lineLayout = layout_integration::LineLayout::Containing(renderInline))
    {
      return lineLayout->FirstInlineBoxFor(renderInline);
    }
    return InlineBoxIterator {BoxLegacyPath {renderInline.FirstLegacyInlineBox()}};
  }

  InlineBoxIterator FirstRootInlineBoxFor(const RenderBlockFlow &block) noexcept
  {
    if (auto *lineLayout = block.InlineLayout())
    {
      return lineLayout->FirstRootInlineBox();
    }
    return InlineBoxIterator {BoxLegacyPath {block.LegacyRootBox()}};
  }

  InlineBoxIterator InlineBoxFor(const LegacyInlineFlowBox &legacyInlineFlowBox) noexcept
  {
    return InlineBoxIterator {BoxLegacyPath {&legacyInlineFlowBox}};
  }

  InlineBoxIterator InlineBoxFor(const layout_integration::InlineContent &content,
                                 const inline_display::Box &box) noexcept
  {
    return InlineBoxFor(content, content.IndexForBox(box));
  }

  InlineBoxIterator InlineBoxFor(const layout_integration::InlineContent &content, size_t boxIndex) noexcept
  {
    krys_debug_assert(content.DisplayContent().BoxList[boxIndex].IsInlineBox());
    return InlineBoxIterator {BoxModernPath {content, boxIndex}};
  }
}
