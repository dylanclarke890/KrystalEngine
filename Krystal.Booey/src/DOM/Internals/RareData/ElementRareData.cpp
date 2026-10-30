#include "Krystal.Booey/DOM/Internals/RareData/ElementRareData.hpp"
#include "Krystal.Booey/Animation/ElementAnimationRareData.hpp"
#include "Krystal.Booey/DOM/DatasetDOMStringMap.hpp"
#include "Krystal.Booey/DOM/DOMTokenList.hpp"
#include "Krystal.Booey/DOM/PopoverData.hpp"
#include "Krystal.Booey/DOM/PseudoElement.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElement.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/CustomStateSet.hpp"
#include "Krystal.Booey/Page/IntersectionObserver.hpp"
#include "Krystal.Booey/Page/ResizeObserver.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"

namespace krys::boo::dom
{
  ElementRareData::ElementRareData() noexcept = default;

  ElementRareData::~ElementRareData() noexcept
  {
    krys_debug_assert(!_beforePseudoElement);
    krys_debug_assert(!_afterPseudoElement);
  }

  void ElementRareData::SetIntersectionObserverData(UniquePtr<IntersectionObserverData> &&data) noexcept
  {
    _intersectionObserverData = krys::move(data);
  }

  void ElementRareData::SetResizeObserverData(UniquePtr<ResizeObserverData> &&data) noexcept
  {
    _resizeObserverData = krys::move(data);
  }

  void ElementRareData::SetPartList(UniquePtr<DOMTokenList> &&partList) noexcept
  {
    _partList = krys::move(partList);
  }

  void ElementRareData::SetDataset(UniquePtr<DatasetDOMStringMap> &&dataset) noexcept
  {
    _dataset = krys::move(dataset);
  }

  void ElementRareData::SetCustomElementReactionQueue(
    UniquePtr<html::CustomElementReactionQueue> &&queue) noexcept
  {
    _customElementReactionQueue = krys::move(queue);
  }

  void ElementRareData::SetCustomElement(UniquePtr<html::CustomElement> &&customElement) noexcept
  {
    _customElement = krys::move(customElement);
  }

  void ElementRareData::SetCustomStateSet(RefPtr<html::CustomStateSet> &&customStateSet) noexcept
  {
    _customStateSet = krys::move(customStateSet);
  }

  void ElementRareData::SetPopoverData(UniquePtr<PopoverData> &&popoverData) noexcept
  {
    _popoverData = krys::move(popoverData);
  }

  void ElementRareData::SetComputedStyle(UniquePtr<RenderStyle> style) noexcept
  {
    _computedStyle = krys::move(style);
  }

  void ElementRareData::SetDisplayContentsOrNoneStyle(UniquePtr<RenderStyle> style) noexcept
  {
    _displayContentsOrNoneStyle = krys::move(style);
  }

  void ElementRareData::SetBeforePseudoElement(RefPtr<PseudoElement> &&pseudoElement) noexcept
  {
    krys_debug_assert(!_beforePseudoElement || !pseudoElement);
    _beforePseudoElement = krys::move(pseudoElement);
  }

  void ElementRareData::SetAfterPseudoElement(RefPtr<PseudoElement> &&pseudoElement) noexcept
  {
    krys_debug_assert(!_afterPseudoElement || !pseudoElement);
    _afterPseudoElement = krys::move(pseudoElement);
  }

  ElementAnimationRareData *ElementRareData::AnimationRareData(
    const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) const noexcept
  {
    auto it = _animationRareData.find(pseudoElementIdentifier);
    return it == _animationRareData.end() ? nullptr : it->second.get();
  }

  ElementAnimationRareData &ElementRareData::EnsureAnimationRareData(
    const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept
  {
    auto it = _animationRareData.find(pseudoElementIdentifier);
    if (it == _animationRareData.end())
    {
      it =
        _animationRareData.emplace(pseudoElementIdentifier, CreateUnique<ElementAnimationRareData>()).first;
    }
    return *it->second;
  }
}
