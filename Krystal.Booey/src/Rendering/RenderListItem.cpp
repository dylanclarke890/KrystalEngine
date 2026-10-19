#include "Krystal.Booey/Rendering/RenderListItem.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/PseudoElement.hpp"
#include "Krystal.Booey/HTML/HTMLOListElement.hpp"
#include "Krystal.Booey/HTML/HTMLUListElement.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/RenderListMarker.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/PseudoElementIdentifier.hpp"

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD CounterDirectives ListItemDirectives(const RenderStyle &style) noexcept
    {
      auto &map = style.GetCounterDirectives().Map;
      auto it = map.find(u8"list-item");
      return it != map.end() ? it->second : CounterDirectives {};
    }

    KRYS_NODISCARD html::HTMLOListElement *AsOrderedList(dom::Element *element) noexcept
    {
      auto *htmlElement = DynamicDowncast<html::HTMLElement>(element);
      return htmlElement ? DynamicDowncast<html::HTMLOListElement>(*htmlElement) : nullptr;
    }

    // Returns the enclosing list with respect to the DOM order.
    KRYS_NODISCARD dom::Element *EnclosingList(const RenderListItem &listItem) noexcept
    {
      auto *element = listItem.GetElement();
      auto *pseudoElement = DynamicDowncast<dom::PseudoElement>(element);
      auto *parent = pseudoElement ? pseudoElement->HostElement() : element->ParentElement();
      for (auto *ancestor = parent; ancestor; ancestor = ancestor->ParentElement())
      {
        if (IsHTMLListElement(*ancestor)
            || (ancestor->Renderer() && ancestor->Renderer()->ShouldApplyStyleContainment()))
        {
          return ancestor;
        }
      }

      // If there's no actual list element, then the parent element acts as our
      // list for purposes of determining what other list items should be numbered as
      // part of the same list.
      return parent;
    }

    KRYS_NODISCARD RenderListItem *NextListItemHelper(const dom::Element &list,
                                                      const dom::Element &element) noexcept
    {
      const dom::Element *current = &element;
      auto advance = [&]
      {
        if (!current->RenderOrDisplayContentsStyle())
        {
          current = dom::TreeQueries::NextElementIncludingPseudoSkippingChildren(*current, &list);
        }
        else
        {
          current = dom::TreeQueries::NextElementIncludingPseudo(*current, &list);
        }
      };
      advance();
      while (current)
      {
        auto *item = DynamicDowncast<RenderListItem>(current->Renderer());
        if (!item)
        {
          advance();
          continue;
        }
        auto *otherList = EnclosingList(*item);
        if (!otherList)
        {
          advance();
          continue;
        }

        // This item is part of our current list, so it's what we're looking for.
        if (&list == otherList)
        {
          return item;
        }

        // We found ourself inside another list; skip the rest of its contents.
        current = dom::TreeQueries::NextElementIncludingPseudoSkippingChildren(*current, &list);
      }

      return nullptr;
    }

    KRYS_NODISCARD RenderListItem *NextListItem(const dom::Element &list, const RenderListItem &item) noexcept
    {
      return NextListItemHelper(list, *item.GetElement());
    }

    KRYS_NODISCARD RenderListItem *FirstListItem(const dom::Element &list) noexcept
    {
      return NextListItemHelper(list, list);
    }

    KRYS_NODISCARD RenderListItem *PreviousListItem(const dom::Element &list,
                                                    const RenderListItem &item) noexcept
    {
      const dom::Element *current = item.GetElement();
      auto advance = [&]
      {
        current = dom::TreeQueries::PreviousElementIncludingPseudo(*current, &list);
      };
      advance();
      while (current)
      {
        auto *previousItem = DynamicDowncast<RenderListItem>(current->Renderer());
        if (!previousItem)
        {
          advance();
          continue;
        }
        auto *otherList = EnclosingList(*previousItem);
        if (!otherList)
        {
          advance();
          continue;
        }

        // This item is part of our current list, so we found what we're looking for.
        if (&list == otherList)
        {
          return previousItem;
        }

        // We found ourself inside another list; skip the rest of its contents by
        // advancing to it. However, since the list itself might be a list item,
        // don't advance past it.
        current = otherList;
      }
      return nullptr;
    }
  }

  RenderListItem::RenderListItem(dom::Element &element, RenderStyle &&style) noexcept
      : RenderBlockFlow(Type::ListItem, element, krys::move(style))
  {
    krys_debug_assert(IsRenderListItem());
    SetInline(false);
  }

  RenderListItem::~RenderListItem() noexcept
  {
    // Do not add any code here. Add it to willBeDestroyed() instead.
    krys_debug_assert(!_marker);
  }

  RenderListMarker *RenderListItem::MarkerRenderer() const noexcept
  {
    return _marker.get();
  }

  void RenderListItem::SetMarkerRenderer(RenderListMarker &marker) noexcept
  {
    _marker = CreateWeakPtr(&marker);
  }

  RenderStyle RenderListItem::ComputeMarkerStyle() const noexcept
  {
    if (!Is<dom::PseudoElement>(GetElement()))
    {
      if (auto *markerStyle = GetCachedPseudoStyle({PseudoElementType::Marker}, &Style()))
      {
        return RenderStyle::Clone(*markerStyle);
      }
    }

    // The marker always inherits from the list item, regardless of where it might end
    // up (e.g., in some deeply nested line box). See CSS3 spec.
    auto markerStyle = RenderStyle::Create();
    markerStyle.InheritFrom(Style());

    // In the case of a ::before or ::after pseudo-element, we manually apply the properties
    // otherwise set in the user-agent stylesheet since we don't support ::before::marker or
    // ::after::marker. See bugs.webkit.org/b/218897.
    auto fontDescription = Style().GetFontDescription();
    fontDescription.SetVariantNumericSpacing(FontVariantNumericSpacing::TabularNumbers);
    markerStyle.SetFontDescription(krys::move(fontDescription));
    markerStyle.SetUnicodeBidi(UnicodeBidi::Isolate);
    markerStyle.SetWhiteSpaceCollapse(WhiteSpaceCollapse::Preserve);
    markerStyle.SetTextWrapMode(TextWrapMode::NoWrap);
    markerStyle.SetTextTransform(css::keywords::None {});
    return markerStyle;
  }

  bool IsHTMLListElement(const dom::Node &node) noexcept
  {
    auto *element = DynamicDowncast<html::HTMLElement>(node);
    return element && (Is<html::HTMLUListElement>(*element) || Is<html::HTMLOListElement>(*element));
  }

  void RenderListItem::UpdateItemValuesForOrderedList(const html::HTMLOListElement &list) noexcept
  {
    for (auto *listItem = FirstListItem(list); listItem; listItem = NextListItem(list, *listItem))
    {
      listItem->UpdateValue();
    }
  }

  uint32 RenderListItem::ItemCountForOrderedList(const html::HTMLOListElement &list) noexcept
  {
    uint32 itemCount = 0;
    for (auto *listItem = FirstListItem(list); listItem; listItem = NextListItem(list, *listItem))
    {
      ++itemCount;
    }
    return itemCount;
  }

  void RenderListItem::UpdateValueNow() const noexcept
  {
    auto *list = EnclosingList(*this);
    auto *orderedList = AsOrderedList(list);

    // The start item is either the closest item before this one in the list that already has a value,
    // or the first item in the list if none have before this have values yet.
    // FIXME: This should skip over items with counter-reset.
    auto *startItem = this;
    if (list)
    {
      auto *item = this;
      while ((item = PreviousListItem(*list, *item)))
      {
        startItem = item;
        if (item->_value)
        {
          break;
        }
      }
    }

    int32 defaultIncrement = orderedList && orderedList->IsReversed() ? -1 : 1;
    auto valueForItem = [&](int32 previousValue, const CounterDirectives &directives)
    {
      if (directives.SetValue)
      {
        return *directives.SetValue;
      }
      int32 increment = directives.IncrementValue.value_or(defaultIncrement);
      if (directives.ResetValue)
      {
        return *directives.ResetValue + increment;
      }
      return previousValue + increment;
    };

    auto &startValue = startItem->_value;
    if (!startValue)
    {
      // Take in account enclosing list counter-reset.
      // FIXME: This can be a lot more simple when lists use presentational hints.
      if (list && list->Renderer())
      {
        auto listDirectives = ListItemDirectives(list->Renderer()->Style());
        if (listDirectives.ResetValue)
        {
          startValue = *listDirectives.ResetValue;
        }
        else
        {
          startValue = orderedList ? orderedList->StartValue() - defaultIncrement : 0;
        }
      }
      auto directives = ListItemDirectives(startItem->Style());
      startValue = valueForItem(startValue.value_or(0), directives);
    }

    int32 value = *startValue;

    for (auto *item = startItem; item != this;)
    {
      item = NextListItem(*list, *item);
      auto directives = ListItemDirectives(item->Style());
      item->_value = valueForItem(value, directives);
      // counter-reset creates a new nested counter, so it should not be counted towards the current counter.
      if (!directives.ResetValue)
      {
        value = *item->_value;
      }
    }
  }

  void RenderListItem::UpdateValue() noexcept
  {
    _value = null;
    if (_marker)
    {
      _marker->SetNeedsLayoutAndPreferredWidthsUpdate();
    }
  }

  void RenderListItem::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    RenderBlockFlow::StyleDidChange(diff, oldStyle);

    if (diff == style::DifferenceResult::Layout && oldStyle
        && ListItemDirectives(*oldStyle) != ListItemDirectives(Style()))
    {
      CounterDirectivesChanged();
    }
  }

  void RenderListItem::ComputePreferredLogicalWidths() noexcept
  {
    // FIXME: RenderListMarker::updateInlineMargins() mutates margin style which affects preferred widths.
    if (_marker && _marker->NeedsPreferredLogicalWidthsUpdate())
    {
      _marker->UpdateInlineMarginsAndContent();
    }

    RenderBlockFlow::ComputePreferredLogicalWidths();
  }

  utf8_string RenderListItem::MarkerTextWithoutSuffix() const noexcept
  {
    if (!_marker)
    {
      return {};
    }
    return _marker->TextWithoutSuffix();
  }

  utf8_string RenderListItem::MarkerTextWithSuffix() const noexcept
  {
    if (!_marker)
    {
      return {};
    }
    return _marker->TextWithSuffix();
  }

  void RenderListItem::CounterDirectivesChanged() noexcept
  {
    if (_marker)
    {
      _marker->SetNeedsLayoutAndPreferredWidthsUpdate();
    }

    UpdateValue();
    auto *list = EnclosingList(*this);
    if (!list)
    {
      return;
    }
    auto *item = this;
    while ((item = NextListItem(*list, *item)))
    {
      item->UpdateValue();
    }
  }

  void RenderListItem::UpdateListMarkerNumbers() noexcept
  {
    auto *list = EnclosingList(*this);
    if (!list)
    {
      return;
    }

    bool isInReversedOrderedList = false;
    if (auto *orderedList = AsOrderedList(list))
    {
      orderedList->ItemCountChanged();
      isInReversedOrderedList = orderedList->IsReversed();
    }

    // If an item has been marked for update before, we know that all following items have, too.
    // This gives us the opportunity to stop and avoid marking the same nodes again.
    auto *item = this;
    auto subsequentListItem = isInReversedOrderedList ? PreviousListItem : NextListItem;
    while ((item = subsequentListItem(*list, *item)) && item->_value)
    {
      item->UpdateValue();
    }
  }

  bool RenderListItem::IsInReversedOrderedList() const noexcept
  {
    auto *list = AsOrderedList(EnclosingList(*this));
    return list && list->IsReversed();
  }
  void RenderListItem::Paint(PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept
  {
    if (!LogicalHeight() && HasNonVisibleOverflow())
    {
      return;
    }

    RenderBlockFlow::Paint(paintInfo, paintOffset);
  }
}
