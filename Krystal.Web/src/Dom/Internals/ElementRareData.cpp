#include "Krystal.Web/Dom/Internals/ElementRareData.hpp"
#include "Krystal.Web/Animation/ElementAnimationRareData.hpp"
#include "Krystal.Web/Css/ResizeObserver/ResizeObserver.hpp"
#include "Krystal.Web/Css/TypedOM/StylePropertyMap.hpp"
#include "Krystal.Web/Dom/Collections/DOMStringMap.hpp"
#include "Krystal.Web/Dom/Collections/DOMTokenList.hpp"
#include "Krystal.Web/Dom/Nodes/PseudoElement.hpp"
#include "Krystal.Web/Html/CustomElement/CustomElement.hpp"
#include "Krystal.Web/Html/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Web/Html/CustomElement/CustomStateSet.hpp"
#include "Krystal.Web/Html/Internals/PopoverData.hpp"
#include "Krystal.Web/IntersectionObserver/IntersectionObserver.hpp"
#include "Krystal.Web/Rendering/Style/RenderStyle.hpp"

namespace krys::web::dom
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

  void ElementRareData::SetDataset(UniquePtr<DOMStringMap> &&dataset) noexcept
  {
    _dataset = krys::move(dataset);
  }

  void ElementRareData::SetAttributeStyleMap(Ref<StylePropertyMap> map) noexcept
  {
    _attributeStyleMap = krys::move(map);
  }

  void ElementRareData::SetComputedStyleMap(Ref<StylePropertyMapReadOnly> map) noexcept
  {
    _computedStyleMap = krys::move(map);
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

  void ElementRareData::SetCustomStateSet(RefPtr<html::CustomStateSet> customStateSet) noexcept
  {
    _customStateSet = krys::move(customStateSet);
  }

  void ElementRareData::SetPopoverData(UniquePtr<html::PopoverData> &&popoverData) noexcept
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

  void ElementRareData::SetBeforePseudoElement(RefPtr<PseudoElement> pseudoElement) noexcept
  {
    krys_debug_assert(!_beforePseudoElement || !pseudoElement);
    _beforePseudoElement = krys::move(pseudoElement);
  }

  void ElementRareData::SetAfterPseudoElement(RefPtr<PseudoElement> pseudoElement) noexcept
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

  Element *ElementRareData::InvokedPopover() const noexcept
  {
    return _invokedPopover.get();
  }

  void ElementRareData::SetInvokedPopover(Element *element) noexcept
  {
    _invokedPopover = CreateWeakPtr(element);
  }
}
