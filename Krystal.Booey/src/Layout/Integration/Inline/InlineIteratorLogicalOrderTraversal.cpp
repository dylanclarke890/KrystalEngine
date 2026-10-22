#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorLogicalOrderTraversal.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include <algorithm>
#include <ranges>

namespace krys::boo::inline_iterator
{
  namespace
  {
    KRYS_NODISCARD TextLogicalOrderCache MakeTextLogicalOrderCacheIfNeeded(const RenderText &text) noexcept
    {
      if (!text.NeedsVisualReordering())
      {
        return {};
      }

      auto cache = CreateUnique<TextLogicalOrderCacheData>();
      for (auto textBox = LineLeftmostTextBoxFor(text); textBox; textBox.TraverseNextTextBox())
      {
        cache->Boxes.push_back(textBox);
      }

      if (cache->Boxes.empty())
      {
        return nullptr;
      }

      std::ranges::sort(cache->Boxes, [&](auto &a, auto &b) { return a->Start() < b->Start(); });

      return cache;
    }

    void UpdateTextLogicalOrderCacheIfNeeded(const TextBoxIterator &textBox,
                                             TextLogicalOrderCache &cache) noexcept
    {
      if (!cache && !(cache = MakeTextLogicalOrderCacheIfNeeded(textBox->Renderer())))
      {
        return;
      }

      if (cache->Index < cache->Boxes.size() && cache->Boxes[cache->Index] == textBox)
      {
        return;
      }

      auto found = std::ranges::find(cache->Boxes, textBox);
      if (found == cache->Boxes.end())
      {
        cache = {};
        UpdateTextLogicalOrderCacheIfNeeded(textBox, cache);
        return;
      }
      cache->Index = static_cast<size_t>(found - cache->Boxes.begin());
    }
  }

  std::pair<TextBoxIterator, TextLogicalOrderCache>
    FirstTextBoxInLogicalOrderFor(const RenderText &text) noexcept
  {
    if (auto cache = MakeTextLogicalOrderCacheIfNeeded(text))
    {
      auto first = cache->Boxes.front();
      return {first, krys::move(cache)};
    }

    return {LineLeftmostTextBoxFor(text), nullptr};
  }

  TextBoxIterator NextTextBoxInLogicalOrder(const TextBoxIterator &textBox,
                                            TextLogicalOrderCache &cache) noexcept
  {
    UpdateTextLogicalOrderCacheIfNeeded(textBox, cache);

    if (!cache)
    {
      return textBox->NextTextBox();
    }

    cache->Index++;

    if (cache->Index < cache->Boxes.size())
    {
      return cache->Boxes[cache->Index];
    }

    return {};
  }

  namespace
  {
    KRYS_NODISCARD LineLogicalOrderCache MakeLineLogicalOrderCache(const LineBoxIterator &lineBox) noexcept
    {
      auto cache = CreateUnique<LineLogicalOrderCacheData>();

      cache->LineBox = lineBox;
      cache->Boxes = LeafBoxesInLogicalOrder(lineBox, [](auto span) { std::ranges::reverse(span); });

      return cache;
    }

    void UpdateLineLogicalOrderCacheIfNeeded(const LeafBoxIterator &box,
                                             LineLogicalOrderCache &cache) noexcept
    {
      auto lineBox = box->GetLineBox();
      if (!cache || cache->LineBox != lineBox)
      {
        cache = MakeLineLogicalOrderCache(lineBox);
      }

      if (cache->Index < cache->Boxes.size() && cache->Boxes[cache->Index] == box)
      {
        return;
      }

      auto found = std::ranges::find(cache->Boxes, box);
      krys_debug_assert(found != cache->Boxes.end());
      cache->Index = static_cast<size_t>(found - cache->Boxes.begin());
    }
  }

  LeafBoxIterator FirstLeafOnLineInLogicalOrder(const LineBoxIterator &lineBox,
                                                LineLogicalOrderCache &cache) noexcept
  {
    cache = MakeLineLogicalOrderCache(lineBox);

    if (cache->Boxes.empty())
    {
      return {};
    }

    cache->Index = 0;
    return cache->Boxes.front();
  }

  LeafBoxIterator LastLeafOnLineInLogicalOrder(const LineBoxIterator &lineBox,
                                               LineLogicalOrderCache &cache) noexcept
  {
    cache = MakeLineLogicalOrderCache(lineBox);

    if (cache->Boxes.empty())
    {
      return {};
    }

    cache->Index = cache->Boxes.size() - 1;
    return cache->Boxes.back();
  }

  LeafBoxIterator NextLeafOnLineInLogicalOrder(const LeafBoxIterator &box,
                                               LineLogicalOrderCache &cache) noexcept
  {
    UpdateLineLogicalOrderCacheIfNeeded(box, cache);

    cache->Index++;

    if (cache->Index < cache->Boxes.size())
    {
      return cache->Boxes[cache->Index];
    }

    return {};
  }

  LeafBoxIterator PreviousLeafOnLineInLogicalOrder(const LeafBoxIterator &box,
                                                   LineLogicalOrderCache &cache) noexcept
  {
    UpdateLineLogicalOrderCacheIfNeeded(box, cache);

    if (!cache->Index)
    {
      return {};
    }

    cache->Index--;

    return cache->Boxes[cache->Index];
  }

  LeafBoxIterator FirstLeafOnLineInLogicalOrderWithNode(const LineBoxIterator &lineBox,
                                                        LineLogicalOrderCache &cache) noexcept
  {
    auto box = FirstLeafOnLineInLogicalOrder(lineBox, cache);
    while (box && !box->Renderer().GetNode())
    {
      box = NextLeafOnLineInLogicalOrder(box, cache);
    }
    return box;
  }

  LeafBoxIterator LastLeafOnLineInLogicalOrderWithNode(const LineBoxIterator &lineBox,
                                                       LineLogicalOrderCache &cache) noexcept
  {
    auto box = LastLeafOnLineInLogicalOrder(lineBox, cache);
    while (box && !box->Renderer().GetNode())
    {
      box = PreviousLeafOnLineInLogicalOrder(box, cache);
    }
    return box;
  }
}
