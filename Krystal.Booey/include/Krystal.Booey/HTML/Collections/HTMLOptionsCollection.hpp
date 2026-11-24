#pragma once

#include "Krystal.Booey/DOM/Collections/LiveHTMLCollection.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo
{
  class HTMLElement;
  class HTMLOptGroupElement;
  class HTMLOptionElement;
  class HTMLSelectElement;
}

namespace krys::boo::html
{
  /// @brief A select's `options`: the options in its list of options.
  /// @see https://html.spec.whatwg.org/#htmloptionscollection
  class HTMLOptionsCollection final : public dom::LiveHTMLCollection
  {
  public:
    using OptionOrOptGroupElement = Variant<RefPtr<HTMLOptionElement>, RefPtr<HTMLOptGroupElement>>;
    using HTMLElementOrInt = Variant<RefPtr<HTMLElement>, int32>;

  private:
    Ref<HTMLSelectElement> _selectElement;

  public:
    explicit HTMLOptionsCollection(HTMLSelectElement &select) noexcept;

    /// @brief The select element whose options the collection holds.
    KRYS_NODISCARD HTMLSelectElement &SelectElement() const noexcept
    {
      return *_selectElement;
    }

    /// @see https://html.spec.whatwg.org/#dom-htmloptionscollection-length
    ExceptionOr<void> Length(uint32 length) noexcept;

    using LiveHTMLCollection::Length;

    /// @see https://html.spec.whatwg.org/#dom-htmloptionscollection-setter
    ExceptionOr<void> SetItem(uint32 index, HTMLOptionElement *option) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-htmloptionscollection-add
    ExceptionOr<void> Add(const OptionOrOptGroupElement &element) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-htmloptionscollection-add
    ExceptionOr<void> Add(const OptionOrOptGroupElement &element,
                          const Maybe<HTMLElementOrInt> &before) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-htmloptionscollection-remove
    void Remove(int32 index) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-htmloptionscollection-selectedindex
    KRYS_NODISCARD int32 SelectedIndex() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-htmloptionscollection-selectedindex
    void SelectedIndex(int32 index) noexcept;
  };
}
