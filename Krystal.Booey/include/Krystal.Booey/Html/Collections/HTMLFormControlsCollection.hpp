#pragma once

#include "Krystal.Booey/Dom/Collections/LiveHTMLCollection.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo
{
  class HTMLElement;
  class HTMLFormElement;
}

namespace krys::boo::html
{
  class RadioNodeList;

  /// @brief A form's `elements`: its listed elements, wherever they are in the tree.
  /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#htmlformcontrolscollection
  class HTMLFormControlsCollection final : public LiveHTMLCollection
  {
  private:
    Ref<HTMLFormElement> _ownerNode;
    /// @brief The last item found, to go on from without searching the form's listed elements for it.
    mutable HTMLElement *_cachedElement {nullptr};
    mutable size_t _cachedElementOffset {0};
    /// @brief The items with each ID and name, once a name has been looked up.
    mutable HashMap<DOMString, List<Element *>> _itemsById;
    mutable HashMap<DOMString, List<Element *>> _itemsByName;
    mutable bool _hasNamedItemMaps {false};

  public:
    explicit HTMLFormControlsCollection(HTMLFormElement &ownerNode) noexcept;

    using LiveHTMLCollection::NamedItem;

    KRYS_NODISCARD RefPtr<Element> NamedItem(DOMStringView name) noexcept override;

    void InvalidateCache() noexcept override;

    /// @brief The elements whose ID or name is @p name: those by ID first, then those by name.
    KRYS_NODISCARD List<Ref<Element>> NamedItems(DOMStringView name) const noexcept;

    /// @brief The element whose ID or name is @p name, or a RadioNodeList of them if there are several.
    /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#dom-htmlformcontrolscollection-nameditem
    KRYS_NODISCARD Maybe<Variant<RefPtr<RadioNodeList>, RefPtr<Element>>>
      NamedItemOrItems(DOMStringView name) const noexcept;

    KRYS_NODISCARD HTMLFormElement &OwnerNode() const noexcept
    {
      return *_ownerNode;
    }

  protected:
    KRYS_NODISCARD Element *CollectionBegin() const noexcept override;

    KRYS_NODISCARD Element *CollectionNext(Element &element) const noexcept override;

  private:
    /// @brief The first enumeratable listed element from @p offset on, if any.
    KRYS_NODISCARD HTMLElement *EnumeratableFrom(size_t offset) const noexcept;
  };
}
