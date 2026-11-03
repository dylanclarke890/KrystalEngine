#pragma once

#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class HTMLCollection;
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