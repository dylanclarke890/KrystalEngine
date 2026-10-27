#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationInlineContent.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBox.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::layout_integration
{
  InlineContent::InlineContent(const RenderBlockFlow &formattingContextRoot) noexcept
      : _formattingContextRoot(ShareCheckedRef(formattingContextRoot))
  {
  }

  bool InlineContent::HasContentfulInFlowBox() const noexcept
  {
    krys_debug_assert(_displayContent.BoxList.empty() || _displayContent.BoxList[0].IsRootInlineBox());
    for (auto &line : _displayContent.LineList)
    {
      if (line.HasContentfulInFlowBox())
      {
        return true;
      }
    }
    return false;
  }

  bool InlineContent::HasContentfulInlineLevelBox() const noexcept
  {
    for (auto &line : _displayContent.LineList)
    {
      if (line.HasContentfulInlineLevelBox())
      {
        return true;
      }
    }
    return false;
  }

  Span<const inline_display::Box> InlineContent::BoxesForRect(const LayoutRect &rect) const noexcept
  {
    auto &lines = _displayContent.LineList;
    auto &boxes = _displayContent.BoxList;
    if (boxes.empty())
    {
      return {};
    }

    // FIXME: Do the flips.
    if (FormattingContextRoot().GetWritingMode().IsBlockFlipped())
    {
      return {boxes.data(), boxes.size()};
    }

    if (lines.front().InkOverflow().MaxY() > rect.Y().ToFloat()
        && lines.back().InkOverflow().Y() < rect.MaxY().ToFloat())
    {
      return {boxes.data(), boxes.size()};
    }

    // The optimization below relies on line paint bounds not exeeding those of the neighboring lines
    if (_hasMultilinePaintOverlap)
    {
      return {boxes.data(), boxes.size()};
    }

    auto height = lines.back().LineBoxBottom() - lines.front().LineBoxTop();
    auto averageLineHeight = height / static_cast<float>(lines.size());

    auto approximateLine = [&](LayoutUnit y)
    {
      y = std::max(y, 0_lu);
      return std::min(static_cast<size_t>(y.ToFloat() / averageLineHeight), lines.size() - 1);
    };

    auto startLine = approximateLine(rect.Y());
    for (; startLine; --startLine)
    {
      if (lines[startLine - 1].InkOverflow().MaxY() < rect.Y().ToFloat())
      {
        break;
      }
    }

    auto endLine = approximateLine(rect.MaxY());
    for (; endLine < lines.size() - 1; ++endLine)
    {
      if (lines[endLine + 1].InkOverflow().Y() > rect.MaxY().ToFloat())
      {
        break;
      }
    }

    auto firstBox = lines[startLine].FirstBoxIndex();
    auto lastBox = lines[endLine].FirstBoxIndex() + lines[endLine].BoxCount();
    return {boxes.data() + firstBox, lastBox - firstBox};
  }

  size_t InlineContent::IndexForBox(const inline_display::Box &box) const noexcept
  {
    auto index = static_cast<size_t>(&box - _displayContent.BoxList.data());
    krys_assert(index < _displayContent.BoxList.size());
    return index;
  }

  const inline_display::Box *InlineContent::FirstBoxForLayoutBox(const layout::Box &layoutBox) const noexcept
  {
    auto index = FirstBoxIndexForLayoutBox(layoutBox);
    return index ? &_displayContent.BoxList[*index] : nullptr;
  }

  Maybe<size_t> InlineContent::FirstBoxIndexForLayoutBox(const layout::Box &layoutBox) const noexcept
  {
    constexpr auto CacheThreshold = 16;
    auto &boxes = _displayContent.BoxList;

    if (boxes.size() < CacheThreshold)
    {
      for (size_t i = 0; i < boxes.size(); ++i)
      {
        auto &box = boxes[i];
        if (&box.LayoutBox() == &layoutBox)
        {
          return i;
        }
      }
      return null;
    }

    if (!_firstBoxIndexCache)
    {
      _firstBoxIndexCache = CreateUnique<FirstBoxIndexCache>();
      for (size_t i = 0; i < boxes.size(); ++i)
      {
        auto &box = boxes[i];
        if (box.IsRootInlineBox())
        {
          continue;
        }
        _firstBoxIndexCache->emplace(&box.LayoutBox(), i);
      }
    }

    auto it = _firstBoxIndexCache->find(&layoutBox);
    if (it == _firstBoxIndexCache->end())
    {
      return null;
    }

    return it->second;
  }

  const inline_display::Box *
    InlineContent::BlockLevelBoxForLine(const inline_display::Line &line) const noexcept
  {
    if (!line.HasBlockLevelBox())
    {
      return nullptr;
    }
    auto &lastBox = DisplayContent().BoxList[line.LastBoxIndex()];
    krys_debug_assert(lastBox.IsBlockLevelBox());
    return lastBox.IsBlockLevelBox() ? &lastBox : nullptr;
  }

  bool InlineContent::IsInlineBoxWrapperForBlockLevelBox(const inline_display::Box &box) const noexcept
  {
    if (!box.IsInlineBox() || !_hasBlockLevelBoxes)
    {
      return false;
    }
    return LineForBox(box).HasBlockLevelBox();
  }

  const List<size_t> &
    InlineContent::NonRootInlineBoxIndexesForLayoutBox(const layout::Box &layoutBox) const noexcept
  {
    krys_debug_assert(layoutBox.IsElementBox());

    auto &boxes = _displayContent.BoxList;
    if (!_inlineBoxIndexCache)
    {
      _inlineBoxIndexCache = CreateUnique<InlineBoxIndexCache>();
      for (size_t i = 0; i < boxes.size(); ++i)
      {
        auto &box = boxes[i];
        if (!box.IsNonRootInlineBox())
        {
          continue;
        }
        (*_inlineBoxIndexCache)[&box.LayoutBox()].push_back(i);
      }
      for (auto &[box, indexes] : *_inlineBoxIndexCache)
      {
        indexes.shrink_to_fit();
      }
    }

    auto it = _inlineBoxIndexCache->find(&layoutBox);
    if (it == _inlineBoxIndexCache->end())
    {
      static const List<size_t> emptyList;
      return emptyList;
    }
    return it->second;
  }

  void InlineContent::ReleaseCaches() noexcept
  {
    _firstBoxIndexCache = nullptr;
    _inlineBoxIndexCache = nullptr;
  }

  const List<SVGTextFragment> &InlineContent::SvgTextFragments(size_t index) const noexcept
  {
    krys_assert(_svgTextFragmentsForBoxes.size() > index);
    return _svgTextFragmentsForBoxes[index];
  }

  void InlineContent::ShrinkToFit() noexcept
  {
    _displayContent.BoxList.shrink_to_fit();
    _displayContent.LineList.shrink_to_fit();
  }
}
