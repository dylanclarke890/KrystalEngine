#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::dom
{
  class Element;

  /// @brief An element's `data-*` attributes, named as properties: `data-foo-bar` is `fooBar`.
  /// @see https://html.spec.whatwg.org/#domstringmap
  class DatasetDOMStringMap
  {
    KRYS_NON_COPY_MOVABLE(DatasetDOMStringMap);

  private:
    Element &_element;

  public:
    explicit DatasetDOMStringMap(Element &element) noexcept : _element(element)
    {
    }

    /// @brief Whether a `data-*` attribute of the element has the property name @p name.
    KRYS_NODISCARD bool IsSupportedPropertyName(DOMStringView name) const noexcept;

    /// @brief The property names of the element's `data-*` attributes, in the order of its attribute list.
    /// @see https://html.spec.whatwg.org/#concept-domstringmap-pairs
    KRYS_NODISCARD List<DOMString> SupportedPropertyNames() const noexcept;

    /// @brief The value of the `data-*` attribute with the property name @p name, if there is one.
    /// @see https://html.spec.whatwg.org/#dom-domstringmap-nameditem
    KRYS_NODISCARD Maybe<DOMString> NamedItem(DOMStringView name) const noexcept;

    /// @brief Sets the `data-*` attribute with the property name @p name to @p value: a `SyntaxError` if
    /// @p name has a `-` before an ASCII lowercase letter, an `InvalidCharacterError` if the attribute name
    /// isn't valid.
    /// @see https://html.spec.whatwg.org/#dom-domstringmap-setitem
    ExceptionOr<void> SetNamedItem(DOMStringView name, DOMString &&value) noexcept;

    /// @brief Removes the `data-*` attribute with the property name @p name.
    /// @see https://html.spec.whatwg.org/#dom-domstringmap-removeitem
    void DeleteNamedProperty(DOMStringView name) noexcept;

    KRYS_NODISCARD Element &GetElement() const noexcept
    {
      return _element;
    }
  };
}
