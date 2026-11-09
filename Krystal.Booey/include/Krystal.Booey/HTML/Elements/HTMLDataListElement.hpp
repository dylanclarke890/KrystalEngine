#pragma once

#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class HTMLCollection;
}

namespace krys::boo::html
{
  class HTMLOptionElement;
}

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmldatalistelement
  class HTMLDataListElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLDataListElement);

  private:
    // A [SameObject] collection, held weakly as it holds the datalist.
    WeakPtr<dom::HTMLCollection> _options;

  public:
    HTMLDataListElement(dom::Document &document) noexcept;

    /// @brief The option elements that are descendants of the datalist.
    /// @see https://html.spec.whatwg.org/#dom-datalist-options
    KRYS_NODISCARD Ref<dom::HTMLCollection> Options() noexcept;

    /// @brief The options that are suggestions: those that aren't disabled and have a value, in tree order.
    /// @see https://html.spec.whatwg.org/#concept-input-list
    KRYS_NODISCARD List<Ref<HTMLOptionElement>> Suggestions() const noexcept;

    /// @brief Tells the inputs whose `list` names the datalist that its options changed.
    void OptionElementChildrenChanged() noexcept;

  private:
    void OnChildrenChanged() noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLDataListElement)

  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::html::HTMLElement>(target).IsHTMLDataListElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLDataListElement();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();