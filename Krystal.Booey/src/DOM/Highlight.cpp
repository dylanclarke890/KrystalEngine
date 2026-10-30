#include "Krystal.Booey/DOM/Highlight.hpp"
#include "Krystal.Booey/DOM/SimpleRange.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include <algorithm>

namespace krys::boo::dom
{
  void Highlight::RepaintRange(const AbstractRange &range) noexcept
  {
    auto sortedRange = MakeSimpleRange(range);
    if (std::is_gt(TreeOrder<TreeType::ComposedTree>(sortedRange.Start, sortedRange.End)))
    {
      std::swap(sortedRange.Start, sortedRange.End);
    }
    for (auto &node : IntersectingNodes(sortedRange))
    {
      if (auto *renderer = node.Renderer())
      {
        renderer->Repaint();
      }
    }
  }

  Ref<Highlight> Highlight::Create(List<Ref<AbstractRange>> &&initialRanges) noexcept
  {
    return AdoptRef(*new Highlight(krys::move(initialRanges)));
  }

  Highlight::Highlight(List<Ref<AbstractRange>> &&initialRanges) noexcept
  {
    _highlightRanges.reserve(initialRanges.size());
    for (auto &range : initialRanges)
    {
      RepaintRange(*range);
      _highlightRanges.push_back(HighlightRange::Create(krys::move(range)));
    }
  }

  bool Highlight::Has(const AbstractRange &range) const noexcept
  {
    return std::ranges::any_of(_highlightRanges, [&](const Ref<HighlightRange> &current)
                               { return &current->Range() == &range; });
  }

  bool Highlight::Delete(const AbstractRange &range) noexcept
  {
    auto found = std::ranges::find_if(_highlightRanges, [&](const Ref<HighlightRange> &current)
                                      { return &current->Range() == &range; });
    if (found == _highlightRanges.end())
    {
      return false;
    }
    RepaintRange(range);
    _highlightRanges.erase(found);
    return true;
  }

  void Highlight::Clear() noexcept
  {
    for (auto &highlightRange : _highlightRanges)
    {
      RepaintRange(highlightRange->Range());
    }
    _highlightRanges.clear();
  }

  bool Highlight::Add(AbstractRange &range) noexcept
  {
    auto found = std::ranges::find_if(_highlightRanges, [&](const Ref<HighlightRange> &current)
                                      { return &current->Range() == &range; });
    if (found == _highlightRanges.end())
    {
      RepaintRange(range);
      _highlightRanges.push_back(HighlightRange::Create(ShareRef(range)));
      return true;
    }
    // Move to last since SetLike is an ordered set.
    auto highlightRange = krys::move(*found);
    _highlightRanges.erase(found);
    _highlightRanges.push_back(krys::move(highlightRange));
    return false;
  }

  void Highlight::Repaint() noexcept
  {
    for (auto &highlightRange : _highlightRanges)
    {
      RepaintRange(highlightRange->Range());
    }
  }

  void Highlight::SetPriority(int32 priority) noexcept
  {
    if (_priority == priority)
    {
      return;
    }
    _priority = priority;
    Repaint();
  }
}
