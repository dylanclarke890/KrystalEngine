#pragma once

#include "Krystal.Booey/DOM/HTMLCollection.hpp"

namespace krys::boo::html
{
  class HTMLTableElement;
  class HTMLTableRowElement;

  /// @brief A table's rows: those of its `thead`s, then those that are its children or in its `tbody`s, then
  /// those of its `tfoot`s, each in tree order.
  /// @see https://html.spec.whatwg.org/#dom-table-rows
  class HTMLTableRowsCollection final : public dom::HTMLCollection
  {
  private:
    Ref<HTMLTableElement> _table;

  public:
    explicit HTMLTableRowsCollection(HTMLTableElement &table) noexcept;

    /// @brief The row after @p previous in @p table's rows, or its first row if @p previous is null.
    KRYS_NODISCARD static HTMLTableRowElement *RowAfter(HTMLTableElement &table,
                                                        HTMLTableRowElement *previous) noexcept;

    /// @brief The last of @p table's rows.
    KRYS_NODISCARD static HTMLTableRowElement *LastRow(HTMLTableElement &table) noexcept;

    KRYS_NODISCARD RefPtr<dom::Element> Item(size_t index) noexcept override;
    KRYS_NODISCARD RefPtr<const dom::Element> Item(size_t index) const noexcept override;

    KRYS_NODISCARD RefPtr<dom::Element> operator[](size_t index) noexcept override;
    KRYS_NODISCARD RefPtr<const dom::Element> operator[](size_t index) const noexcept override;

    KRYS_NODISCARD RefPtr<dom::Element> NamedItem(dom::DOMStringView name) noexcept override;
    KRYS_NODISCARD RefPtr<const dom::Element> NamedItem(dom::DOMStringView name) const noexcept override;

    KRYS_NODISCARD RefPtr<dom::Element> operator[](dom::DOMStringView name) noexcept override;
    KRYS_NODISCARD RefPtr<const dom::Element> operator[](dom::DOMStringView name) const noexcept override;

    KRYS_NODISCARD size_t Length() const noexcept override;

  protected:
    KRYS_NODISCARD List<dom::DOMString> SupportedPropertyNames() const noexcept override;
  };
}
