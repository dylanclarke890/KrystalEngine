#include "Krystal.Booey/DOM/DatasetDOMStringMap.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo::dom
{
  namespace
  {
    constexpr DOMStringView DatasetAttributePrefix = u8"data-";

    // Whether the attribute is a data attribute: data- and then no ASCII uppercase letters.
    KRYS_NODISCARD bool IsDatasetAttributeName(DOMStringView name) noexcept
    {
      if (!name.starts_with(DatasetAttributePrefix))
      {
        return false;
      }

      for (auto character : name.substr(DatasetAttributePrefix.size()))
      {
        if (text::IsASCIIUpper(character))
        {
          return false;
        }
      }

      return true;
    }

    // The attribute's name without data-, with each - before an ASCII lowercase letter removed and the
    // letter uppercased.
    KRYS_NODISCARD DOMString DatasetAttributeNameToPropertyName(DOMStringView name) noexcept
    {
      DOMString propertyName;
      for (size_t i = DatasetAttributePrefix.size(); i < name.size(); ++i)
      {
        auto character = name[i];
        if (character == u8'-' && i + 1 < name.size() && text::IsASCIILower(name[i + 1]))
        {
          propertyName += static_cast<char8>(text::ToASCIIUpper(name[i + 1]));
          ++i;
        }
        else
        {
          propertyName += character;
        }
      }
      return propertyName;
    }

    // Whether the name has no - before an ASCII lowercase letter.
    KRYS_NODISCARD bool IsValidDatasetPropertyName(DOMStringView name) noexcept
    {
      for (size_t i = 0; i < name.size(); ++i)
      {
        if (name[i] == u8'-' && i + 1 < name.size() && text::IsASCIILower(name[i + 1]))
        {
          return false;
        }
      }
      return true;
    }

    // data-, then the name with a - before each ASCII uppercase letter, which is lowercased.
    KRYS_NODISCARD DOMString DatasetPropertyNameToAttributeName(DOMStringView name) noexcept
    {
      DOMString attributeName(DatasetAttributePrefix);
      attributeName.reserve(DatasetAttributePrefix.size() + name.size());
      for (auto character : name)
      {
        if (text::IsASCIIUpper(character))
        {
          attributeName += u8'-';
          attributeName += static_cast<char8>(text::ToASCIILower(character));
        }
        else
        {
          attributeName += character;
        }
      }
      return attributeName;
    }

    // The data attribute whose property name is name.
    KRYS_NODISCARD const Attr *FindDatasetAttribute(const Element &element, DOMStringView name) noexcept
    {
      for (const auto &attribute : element.AttributeList())
      {
        auto localName = attribute->LocalName().View();
        if (IsDatasetAttributeName(localName) && DatasetAttributeNameToPropertyName(localName) == name)
        {
          return attribute.get();
        }
      }
      return nullptr;
    }
  }

  bool DatasetDOMStringMap::IsSupportedPropertyName(DOMStringView name) const noexcept
  {
    return FindDatasetAttribute(_element, name) != nullptr;
  }

  List<DOMString> DatasetDOMStringMap::SupportedPropertyNames() const noexcept
  {
    List<DOMString> names;
    for (const auto &attribute : _element.AttributeList())
    {
      auto localName = attribute->LocalName().View();
      if (IsDatasetAttributeName(localName))
      {
        names.push_back(DatasetAttributeNameToPropertyName(localName));
      }
    }
    return names;
  }

  Maybe<DOMString> DatasetDOMStringMap::NamedItem(DOMStringView name) const noexcept
  {
    if (auto *attribute = FindDatasetAttribute(_element, name))
    {
      return attribute->Value();
    }
    return null;
  }

  ExceptionOr<void> DatasetDOMStringMap::SetNamedItem(DOMStringView name, DOMString &&value) noexcept
  {
    html::CEReactionsScope ceReactions;
    if (!IsValidDatasetPropertyName(name))
    {
      return Exception {ExceptionCode::SyntaxError};
    }
    return _element.SetAttribute(DOMStringAtom(DatasetPropertyNameToAttributeName(name)), krys::move(value));
  }

  void DatasetDOMStringMap::DeleteNamedProperty(DOMStringView name) noexcept
  {
    html::CEReactionsScope ceReactions;
    _element.RemoveAttribute(DOMStringAtom(DatasetPropertyNameToAttributeName(name)));
  }
}
