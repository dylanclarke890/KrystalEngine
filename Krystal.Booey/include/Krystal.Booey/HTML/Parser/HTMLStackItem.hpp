#pragma once

#include "Krystal.Booey/DOM/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/HTML/Enums/Namespace.hpp"
#include "Krystal.Booey/HTML/Enums/TagName.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLTokenAtom.hpp"

namespace krys::boo::html
{
  class HTMLStackItem
  {
  private:
    TagName _name {TagName::Unknown};
    Namespace _namespace {Namespace::Unknown};
    RefPtr<Element> _element;
    ParsedAttributeList _attributes;

  public:
    HTMLStackItem() noexcept = default;

    HTMLStackItem(TagName tagName, Namespace tagNamespace, Element &element,
                  ParsedAttributeList &&attributes) noexcept
        : _name(tagName), _namespace(tagNamespace), _element(ShareRef(element)),
          _attributes(krys::move(attributes))
    {
    }

    HTMLStackItem(TagName tagName, Namespace tagNamespace, Element &element,
                  const ParsedAttributeList &attributes) noexcept
        : _name(tagName), _namespace(tagNamespace), _element(ShareRef(element)), _attributes(attributes)
    {
    }

    KRYS_NODISCARD TagName TagName() const noexcept
    {
      return _name;
    }

    KRYS_NODISCARD Namespace Namespace() const noexcept
    {
      return _namespace;
    }

    KRYS_NODISCARD Element &Element() const noexcept
    {
      return *_element;
    }

    KRYS_NODISCARD const ParsedAttributeList &Attributes() const noexcept
    {
      return _attributes;
    }

    /// @brief Replaces the DOM element reference stored in this item while keeping the tag metadata.
    void UpdateElement(HTML::Element &newElement) noexcept
    {
      _element = ShareRef(newElement);
    }
  };
}