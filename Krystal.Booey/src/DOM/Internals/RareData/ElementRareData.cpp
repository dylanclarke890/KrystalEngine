#include "Krystal.Booey/DOM/Internals/RareData/ElementRareData.hpp"
#include "Krystal.Booey/Animation/ElementAnimationRareData.hpp"
#include "Krystal.Booey/DOM/PseudoElement.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElement.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/CustomStateSet.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"

namespace krys::boo::dom
{
  ElementRareData::ElementRareData() noexcept = default;

  ElementRareData::~ElementRareData() noexcept
  {
    krys_debug_assert(!_beforePseudoElement);
    krys_debug_assert(!_afterPseudoElement);
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
