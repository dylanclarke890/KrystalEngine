#include "Krystal.Booey/Layout/Floats/PlacedFloats.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutContainingBlockChainIterator.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutInitialContainingBlock.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include <algorithm>

namespace krys::boo::layout
{
  PlacedFloats::Item::Item(const Box &layoutBox, Position position, const BoxGeometry &absoluteBoxGeometry,
                           LayoutPoint localTopLeft, Maybe<size_t> line) noexcept
      : _layoutBox(ShareCheckedPtr(&layoutBox)), _position(position),
        _absoluteBoxGeometry(absoluteBoxGeometry), _localTopLeft(localTopLeft),
        _shape(ShareRefPtr(layoutBox.Shape())), _placedByLine(line)
  {
  }

  PlacedFloats::Item::Item(Position position, const BoxGeometry &absoluteBoxGeometry,
                           LayoutPoint localTopLeft, RefPtr<const LayoutShape> &&shape) noexcept
      : _position(position), _absoluteBoxGeometry(absoluteBoxGeometry), _localTopLeft(localTopLeft),
        _shape(krys::move(shape))
  {
  }

  BoxGeometry PlacedFloats::Item::GetBoxGeometry() const noexcept
  {
    auto boxGeometry = BoxGeometry {_absoluteBoxGeometry};
    boxGeometry.SetTopLeft(_localTopLeft);
    return boxGeometry;
  }

  PlacedFloats::Item::~Item() noexcept = default;

  PlacedFloats::PlacedFloats(const ElementBox &blockFormattingContextRoot) noexcept
      : _blockFormattingContextRoot(ShareCheckedRef(blockFormattingContextRoot)),
        _writingMode(blockFormattingContextRoot.GetWritingMode())
  {
    krys_debug_assert(blockFormattingContextRoot.EstablishesBlockFormattingContext());
  }

  void PlacedFloats::Add(Item newFloatItem) noexcept
  {
    auto isStartPositioned = newFloatItem.IsStartPositioned();
    _positionTypes.Add(isStartPositioned ? PositionType::Start : PositionType::End);

    if (_list.empty())
    {
      _list.push_back(krys::move(newFloatItem));
      return;
    }

    // The integration codepath does not construct a layout box for the float item.
    krys_debug_assert(!newFloatItem.LayoutBox()
                      || std::ranges::none_of(_list, [&](auto &entry)
                                              { return entry.LayoutBox() == newFloatItem.LayoutBox(); }));

    // When adding a new float item to the list, we have to ensure that it is definitely the left(right)-most
    // item. Normally it is, but negative inline-axis margins can push the float box beyond another float box.
    // Float items in _list list should stay in inline-axis position order (left/right edge) on the same
    // block-axis position.
    auto inlineAxisMargin = newFloatItem.InlineAxisMargin();
    auto hasNegativeInlineAxisMargin =
      (isStartPositioned && inlineAxisMargin.Start < 0) || (!isStartPositioned && inlineAxisMargin.End < 0);
    if (!hasNegativeInlineAxisMargin)
    {
      _list.push_back(krys::move(newFloatItem));
      return;
    }

    for (size_t i = _list.size(); i--;)
    {
      auto &floatItem = _list[i];
      if (isStartPositioned != floatItem.IsStartPositioned())
      {
        continue;
      }

      auto isInlineAxisOrdered = [&]
      {
        if (newFloatItem.AbsoluteRectWithMargin().Top() > floatItem.AbsoluteRectWithMargin().Top())
        {
          // There's no more floats on this block axis position.
          return true;
        }
        return (isStartPositioned
                && newFloatItem.AbsoluteRectWithMargin().Right()
                     >= floatItem.AbsoluteRectWithMargin().Right())
               || (!isStartPositioned
                   && newFloatItem.AbsoluteRectWithMargin().Left()
                        <= floatItem.AbsoluteRectWithMargin().Left());
      };
      if (isInlineAxisOrdered())
      {
        _list.insert(_list.begin() + static_cast<ptrdiff_t>(i + 1), krys::move(newFloatItem));
        return;
      }
    }
    _list.insert(_list.begin(), krys::move(newFloatItem));
  }

  bool PlacedFloats::Remove(const Box &floatBox) noexcept
  {
    auto iterator = std::ranges::find_if(_list, [&floatBox](auto &placedFloatItem)
                                         { return placedFloatItem.LayoutBox() == &floatBox; });
    if (iterator == _list.end())
    {
      return false;
    }
    _list.erase(iterator);
    return true;
  }

  bool PlacedFloats::Item::IsInFormattingContextOf(const ElementBox &formattingContextRoot) const noexcept
  {
    krys_debug_assert(formattingContextRoot.EstablishesFormattingContext());
    krys_debug_assert(!Is<InitialContainingBlock>(*_layoutBox));
    for (auto &containingBlock : ContainingBlockChain(*_layoutBox))
    {
      if (&containingBlock == &formattingContextRoot)
      {
        return true;
      }
    }
    krys_debug_assert(false);
    return false;
  }

  void PlacedFloats::Clear() noexcept
  {
    _list.clear();
    _positionTypes = {};
  }

  Maybe<LayoutUnit> PlacedFloats::HighestPositionOnBlockAxis() const noexcept
  {
    auto highestBlockAxisPosition = Maybe<LayoutUnit> {};
    for (auto &floatItem : _list)
    {
      highestBlockAxisPosition =
        !highestBlockAxisPosition
          ? floatItem.AbsoluteRectWithMargin().Top()
          : std::min(*highestBlockAxisPosition, floatItem.AbsoluteRectWithMargin().Top());
    }
    return highestBlockAxisPosition;
  }

  Maybe<LayoutUnit> PlacedFloats::LowestPositionOnBlockAxis(boo::Clear type) const noexcept
  {
    // FIXME: Currently this is only called once for each formatting context root with floats per layout.
    // Cache the value if we end up calling it more frequently (and update it at append/remove).
    auto lowestBlockAxisPosition = Maybe<LayoutUnit> {};
    for (auto &floatItem : _list)
    {
      if ((type == boo::Clear::InlineStart && !floatItem.IsStartPositioned())
          || (type == boo::Clear::InlineEnd && floatItem.IsStartPositioned()))
      {
        continue;
      }
      lowestBlockAxisPosition =
        !lowestBlockAxisPosition
          ? floatItem.AbsoluteRectWithMargin().Bottom()
          : std::max(*lowestBlockAxisPosition, floatItem.AbsoluteRectWithMargin().Bottom());
    }
    return lowestBlockAxisPosition;
  }

  void PlacedFloats::ShrinkToFit() noexcept
  {
    _list.shrink_to_fit();
  }
}
