#pragma once

#include "Krystal.Booey/CSS/Calc/RandomCachingKeyMap.hpp"
#include "Krystal.Booey/DOM/Internals/RareData/ParentNodeRareData.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Booey/Style/PseudoElementIdentifier.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class ElementAnimationRareData;
  class RenderStyle;
}

namespace krys::boo::html
{
  class CustomElement;
  class CustomElementReactionQueue;
  class CustomStateSet;
}

namespace krys::boo::dom
{
  class Element;
  class PopoverData;
  class PseudoElement;

  /// @brief The elements set through an element's element reflection attributes, by attribute name.
  /// @see https://html.spec.whatwg.org/#explicitly-set-attr-element
  using ExplicitlySetAttrElementsMap = HashMap<DOMStringAtom, WeakPtr<Element>>;

  class ElementRareData : public ParentNodeRareData
  {
    struct PseudoElementIdentifierHash
    {
      KRYS_NODISCARD size_t operator()(const Maybe<style::PseudoElementIdentifier> &identifier) const noexcept
      {
        return identifier ? DefaultHash<style::PseudoElementIdentifier>::Hash(*identifier) + 1 : 0;
      }
    };

  private:
    uint16 _childIndex {0};
    IntPoint _savedLayerScrollPosition;
    UniquePtr<RenderStyle> _computedStyle;
    UniquePtr<RenderStyle> _displayContentsOrNoneStyle;
    RefPtr<PseudoElement> _beforePseudoElement;
    RefPtr<PseudoElement> _afterPseudoElement;
    HashMap<Maybe<style::PseudoElementIdentifier>, Ref<css::calc::RandomCachingKeyMap>,
            PseudoElementIdentifierHash>
      _randomCachingKeyMaps;
    HashMap<Maybe<style::PseudoElementIdentifier>, UniquePtr<ElementAnimationRareData>,
            PseudoElementIdentifierHash>
      _animationRareData;
    UniquePtr<html::CustomElementReactionQueue> _customElementReactionQueue;
    UniquePtr<html::CustomElement> _customElement;
    RefPtr<html::CustomStateSet> _customStateSet;
    UniquePtr<PopoverData> _popoverData;
    WeakPtr<Element> _invokedPopover;
    ExplicitlySetAttrElementsMap _explicitlySetAttrElementsMap;

  public:
    ElementRareData() noexcept;
    ~ElementRareData() noexcept;

    KRYS_NODISCARD html::CustomElementReactionQueue *CustomElementReactionQueue() const noexcept
    {
      return _customElementReactionQueue.get();
    }

    void SetCustomElementReactionQueue(UniquePtr<html::CustomElementReactionQueue> &&queue) noexcept;

    KRYS_NODISCARD html::CustomElement *CustomElement() const noexcept
    {
      return _customElement.get();
    }

    void SetCustomElement(UniquePtr<html::CustomElement> &&customElement) noexcept;

    KRYS_NODISCARD html::CustomStateSet *CustomStateSet() const noexcept
    {
      return _customStateSet.get();
    }

    void SetCustomStateSet(RefPtr<html::CustomStateSet> &&customStateSet) noexcept;

    KRYS_NODISCARD PopoverData *GetPopoverData() const noexcept
    {
      return _popoverData.get();
    }

    void SetPopoverData(UniquePtr<PopoverData> &&popoverData) noexcept;

    KRYS_NODISCARD Element *InvokedPopover() const noexcept
    {
      return _invokedPopover.get();
    }

    void SetInvokedPopover(Element *element) noexcept
    {
      _invokedPopover = CreateWeakPtr(element);
    }

    KRYS_NODISCARD ExplicitlySetAttrElementsMap &GetExplicitlySetAttrElementsMap() noexcept
    {
      return _explicitlySetAttrElementsMap;
    }

    /// @brief The element's index among its parent's element children, plus one, cached by selector matching
    /// (zero if unknown).
    KRYS_NODISCARD uint32 ChildIndex() const noexcept
    {
      return _childIndex;
    }

    void SetChildIndex(uint32 index) noexcept
    {
      _childIndex = static_cast<uint16>(index);
    }

    /// @brief The element's style resolved outside style resolution (`Element::ComputedStyle`).
    KRYS_NODISCARD RenderStyle *ComputedStyle() const noexcept
    {
      return _computedStyle.get();
    }

    void SetComputedStyle(UniquePtr<RenderStyle> style) noexcept;

    /// @brief The style of an element with `display: contents` or `display: none`, which has no renderer to
    /// hold it.
    KRYS_NODISCARD RenderStyle *DisplayContentsOrNoneStyle() const noexcept
    {
      return _displayContentsOrNoneStyle.get();
    }

    void SetDisplayContentsOrNoneStyle(UniquePtr<RenderStyle> style) noexcept;

    KRYS_NODISCARD PseudoElement *BeforePseudoElement() const noexcept
    {
      return _beforePseudoElement.get();
    }

    KRYS_NODISCARD PseudoElement *AfterPseudoElement() const noexcept
    {
      return _afterPseudoElement.get();
    }

    void SetBeforePseudoElement(RefPtr<PseudoElement> &&pseudoElement) noexcept;

    KRYS_NODISCARD IntPoint SavedLayerScrollPosition() const noexcept
    {
      return _savedLayerScrollPosition;
    }
    void SetSavedLayerScrollPosition(IntPoint position) noexcept
    {
      _savedLayerScrollPosition = position;
    }
    void SetAfterPseudoElement(RefPtr<PseudoElement> &&pseudoElement) noexcept;

    /// @brief The `random()` base values of the element (or one of its pseudo-elements).
    KRYS_NODISCARD Ref<css::calc::RandomCachingKeyMap>
      EnsureRandomCachingKeyMap(const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept
    {
      auto it = _randomCachingKeyMaps.find(pseudoElementIdentifier);
      if (it == _randomCachingKeyMaps.end())
      {
        it = _randomCachingKeyMaps.emplace(pseudoElementIdentifier, css::calc::RandomCachingKeyMap::Create())
               .first;
      }
      return it->second;
    }

    KRYS_NODISCARD bool HasRandomCachingKeyMap() const noexcept
    {
      return !_randomCachingKeyMaps.empty();
    }

    KRYS_NODISCARD bool HasAnimationRareData() const noexcept
    {
      return !_animationRareData.empty();
    }
    /// @brief The animation state of the element (or one of its pseudo-elements), if it has any.
    KRYS_NODISCARD ElementAnimationRareData *
      AnimationRareData(const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) const noexcept;
    KRYS_NODISCARD ElementAnimationRareData &
      EnsureAnimationRareData(const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept;
  };
}
