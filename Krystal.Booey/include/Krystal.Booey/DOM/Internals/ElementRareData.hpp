#pragma once

#include "Krystal.Booey/CSS/Calc/RandomCachingKeyMap.hpp"
#include "Krystal.Booey/DOM/ContentVisibility/ContentRelevancy.hpp"
#include "Krystal.Booey/DOM/Internals/NodeRareData.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutUnit.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomStateSet.hpp"
#include "Krystal.Booey/Style/PseudoElementIdentifier.hpp"
#include "Krystal.Core/Containers/EnumSet.hpp"
#include "Krystal.Core/Containers/HashMap.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Geometry/Point.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Memory/UniquePtr.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  class DOMStringMap;
  class DOMTokenList;
  class Element;
  class ElementAnimationRareData;
  struct IntersectionObserverData;
  class PseudoElement;
  class RenderStyle;
  struct ResizeObserverData;
  class StylePropertyMap;
  class StylePropertyMapReadOnly;
}

namespace krys::boo::html
{
  class CustomElement;
  class CustomElementReactionQueue;
  class CustomStateSet;
}

namespace krys::boo::dom
{
  class PopoverData;

  /// @brief The elements set through an element's element reflection attributes, by attribute name.
  /// @see https://html.spec.whatwg.org/#explicitly-set-attr-element
  using ExplicitlySetAttrElementsMap = HashMap<DOMStringAtom, WeakPtr<Element>>;

  /// @brief The elements set through an element's `FrozenArray<Element>?` reflection attributes, by attribute
  /// name.
  /// @see https://html.spec.whatwg.org/#explicitly-set-attr-elements
  using ExplicitlySetAttrElementListsMap = HashMap<DOMStringAtom, List<WeakPtr<Element>>>;

  /// @brief The data an element needs only some of the time, made when it's first needed
  /// (`Element::EnsureElementRareData`).
  class ElementRareData : public NodeRareData
  {
    /// @brief Hashes a pseudo-element, or the element itself (null).
    struct PseudoElementIdentifierHash
    {
      KRYS_NODISCARD size_t operator()(const Maybe<style::PseudoElementIdentifier> &identifier) const noexcept
      {
        return identifier ? FastHash<style::PseudoElementIdentifier> {}(*identifier) + 1 : 0;
      }
    };

  private:
    uint16 _childIndex {0};
    PointI _savedLayerScrollPosition;
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
    ExplicitlySetAttrElementListsMap _explicitlySetAttrElementListsMap;
    UniquePtr<IntersectionObserverData> _intersectionObserverData;
    UniquePtr<ResizeObserverData> _resizeObserverData;
    UniquePtr<DOMTokenList> _partList;
    UniquePtr<DOMStringMap> _dataset;
    RefPtr<StylePropertyMap> _attributeStyleMap;
    RefPtr<StylePropertyMapReadOnly> _computedStyleMap;
    Maybe<LayoutUnit> _lastRememberedLogicalWidth;
    Maybe<LayoutUnit> _lastRememberedLogicalHeight;
    Maybe<EnumSet<ContentRelevancy>> _contentRelevancy;

  public:
    ElementRareData() noexcept;

    ~ElementRareData() noexcept override;

    /// @brief The element's custom element reaction queue, once a reaction has been enqueued for it.
    /// @see https://html.spec.whatwg.org/#custom-element-reaction-queue
    KRYS_NODISCARD html::CustomElementReactionQueue *CustomElementReactionQueue() const noexcept
    {
      return _customElementReactionQueue.get();
    }

    void SetCustomElementReactionQueue(UniquePtr<html::CustomElementReactionQueue> &&queue) noexcept;

    /// @brief The state and behaviour of a custom element, once it's been defined.
    KRYS_NODISCARD html::CustomElement *CustomElement() const noexcept
    {
      return _customElement.get();
    }

    void SetCustomElement(UniquePtr<html::CustomElement> &&customElement) noexcept;

    /// @brief The custom element's states, made when first asked for (`Element::EnsureCustomStateSet`).
    KRYS_NODISCARD html::CustomStateSet *CustomStateSet() const noexcept
    {
      return _customStateSet.get();
    }

    void SetCustomStateSet(RefPtr<html::CustomStateSet> customStateSet) noexcept;

    /// @brief The state of the element's `popover` attribute.
    KRYS_NODISCARD PopoverData *GetPopoverData() const noexcept
    {
      return _popoverData.get();
    }

    void SetPopoverData(UniquePtr<PopoverData> &&popoverData) noexcept;

    /// @brief The popover the element (a button) showed and is still the popover trigger of, if any.
    KRYS_NODISCARD Element *InvokedPopover() const noexcept;

    void SetInvokedPopover(Element *element) noexcept;

    /// @brief See `ExplicitlySetAttrElementsMap` and `ExplicitlySetAttrElementListsMap`.
    KRYS_NODISCARD ExplicitlySetAttrElementsMap &GetExplicitlySetAttrElementsMap() noexcept
    {
      return _explicitlySetAttrElementsMap;
    }

    KRYS_NODISCARD ExplicitlySetAttrElementListsMap &GetExplicitlySetAttrElementListsMap() noexcept
    {
      return _explicitlySetAttrElementListsMap;
    }

    /// @brief The intersection observers observing the element, and those rooted at it.
    KRYS_NODISCARD IntersectionObserverData *GetIntersectionObserverData() const noexcept
    {
      return _intersectionObserverData.get();
    }

    void SetIntersectionObserverData(UniquePtr<IntersectionObserverData> &&data) noexcept;

    /// @brief The resize observers observing the element.
    KRYS_NODISCARD ResizeObserverData *GetResizeObserverData() const noexcept
    {
      return _resizeObserverData.get();
    }

    void SetResizeObserverData(UniquePtr<ResizeObserverData> &&data) noexcept;

    /// @brief The element's `part` list, once it's been asked for.
    KRYS_NODISCARD DOMTokenList *PartList() const noexcept
    {
      return _partList.get();
    }

    void SetPartList(UniquePtr<DOMTokenList> &&partList) noexcept;

    /// @brief The element's `dataset`, once it's been asked for.
    KRYS_NODISCARD DOMStringMap *Dataset() const noexcept
    {
      return _dataset.get();
    }

    void SetDataset(UniquePtr<DOMStringMap> &&dataset) noexcept;

    /// @brief The element's `attributeStyleMap`, once it's been asked for.
    KRYS_NODISCARD StylePropertyMap *AttributeStyleMap() const noexcept
    {
      return _attributeStyleMap.get();
    }

    void SetAttributeStyleMap(Ref<StylePropertyMap> map) noexcept;

    /// @brief The element's `computedStyleMap()`, once it's been asked for.
    KRYS_NODISCARD StylePropertyMapReadOnly *ComputedStyleMap() const noexcept
    {
      return _computedStyleMap.get();
    }

    void SetComputedStyleMap(Ref<StylePropertyMapReadOnly> map) noexcept;

    /// @brief The element's last remembered size in each axis, for `contain-intrinsic-size: auto`.
    /// @see https://drafts.csswg.org/css-sizing-4/#last-remembered
    KRYS_NODISCARD Maybe<LayoutUnit> LastRememberedLogicalWidth() const noexcept
    {
      return _lastRememberedLogicalWidth;
    }

    KRYS_NODISCARD Maybe<LayoutUnit> LastRememberedLogicalHeight() const noexcept
    {
      return _lastRememberedLogicalHeight;
    }

    void SetLastRememberedLogicalWidth(LayoutUnit width) noexcept
    {
      _lastRememberedLogicalWidth = width;
    }

    void SetLastRememberedLogicalHeight(LayoutUnit height) noexcept
    {
      _lastRememberedLogicalHeight = height;
    }

    void ClearLastRememberedLogicalWidth() noexcept
    {
      _lastRememberedLogicalWidth = null;
    }

    void ClearLastRememberedLogicalHeight() noexcept
    {
      _lastRememberedLogicalHeight = null;
    }

    /// @brief Why the element is relevant to the user, once its relevancy has been determined.
    /// @see https://drafts.csswg.org/css-contain/#relevant-to-the-user
    KRYS_NODISCARD const Maybe<EnumSet<ContentRelevancy>> &GetContentRelevancy() const noexcept
    {
      return _contentRelevancy;
    }

    void SetContentRelevancy(EnumSet<ContentRelevancy> contentRelevancy) noexcept
    {
      _contentRelevancy = contentRelevancy;
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

    /// @brief The element's `::before` and `::after` pseudo-elements, or null where it has none.
    KRYS_NODISCARD PseudoElement *BeforePseudoElement() const noexcept
    {
      return _beforePseudoElement.get();
    }

    KRYS_NODISCARD PseudoElement *AfterPseudoElement() const noexcept
    {
      return _afterPseudoElement.get();
    }

    void SetBeforePseudoElement(RefPtr<PseudoElement> pseudoElement) noexcept;

    /// @brief The scroll position of the element's layer, kept while the element has no renderer.
    KRYS_NODISCARD PointI SavedLayerScrollPosition() const noexcept
    {
      return _savedLayerScrollPosition;
    }

    void SetSavedLayerScrollPosition(PointI position) noexcept
    {
      _savedLayerScrollPosition = position;
    }

    void SetAfterPseudoElement(RefPtr<PseudoElement> pseudoElement) noexcept;

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

    /// @brief Whether the element or a pseudo-element has `random()` base values or animation state.
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

    /// @brief The animation state of the element (or one of its pseudo-elements), made if it has none.
    KRYS_NODISCARD ElementAnimationRareData &
      EnsureAnimationRareData(const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept;
  };
}
