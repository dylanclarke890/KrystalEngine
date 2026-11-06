#pragma once

#include "Krystal.Booey/DOM/Collections/LiveHTMLCollection.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::html
{
  class HTMLElement;
  class HTMLFormElement;
  class RadioNodeList;

  /// @brief A form's `elements`: its listed elements, wherever they are in the tree.
  /// @see https://html.spec.whatwg.org/#htmlformcontrolscollection
  class HTMLFormControlsCollection final : public dom::LiveHTMLCollection
  {
  private:
    Ref<HTMLFormElement> _ownerNode;
    /// @brief The last item found, to go on from without searching the form's listed elements for it.
    mutable HTMLElement *_cachedElement {nullptr};
    mutable size_t _cachedElementOffset {0};
    /// @brief The items with each ID and name, once a name has been looked up.
    mutable HashMap<dom::DOMString, List<dom::Element *>> _itemsById;
    mutable HashMap<dom::DOMString, List<dom::Element *>> _itemsByName;
    mutable bool _hasNamedItemMaps {false};

  public:
    explicit HTMLFormControlsCollection(HTMLFormElement &ownerNode) noexcept;

    using LiveHTMLCollection::NamedItem;
    KRYS_NODISCARD RefPtr<dom::Element> NamedItem(dom::DOMStringView name) noexcept override;

    void InvalidateCache() noexcept override;

    /// @brief The elements whose ID or name is @p name: those by ID first, then those by name.
    KRYS_NODISCARD List<Ref<dom::Element>> NamedItems(dom::DOMStringView name) const noexcept;

    /// @brief The element whose ID or name is @p name, or a RadioNodeList of them if there are several.
    /// @see https://html.spec.whatwg.org/#dom-htmlformcontrolscollection-nameditem
    KRYS_NODISCARD Maybe<Variant<RefPtr<RadioNodeList>, RefPtr<dom::Element>>>
      NamedItemOrItems(dom::DOMStringView name) const noexcept;

    KRYS_NODISCARD HTMLFormElement &OwnerNode() const noexcept
    {
      return *_ownerNode;
    }

  protected:
    KRYS_NODISCARD dom::Element *CollectionBegin() const noexcept override;
    KRYS_NODISCARD dom::Element *CollectionNext(dom::Element &element) const noexcept override;

  private:
    /// @brief The first enumeratable listed element from @p offset on, if any.
    KRYS_NODISCARD HTMLElement *EnumeratableFrom(size_t offset) const noexcept;
  };
}
