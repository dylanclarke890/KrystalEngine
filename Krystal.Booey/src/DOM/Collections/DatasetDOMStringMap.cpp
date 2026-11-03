#include "Krystal.Booey/DOM/Collections/DatasetDOMStringMap.hpp"
#include "Krystal.Booey/DOM/Nodes/Attr.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
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
    // To get a DOMStringMap's name-value pairs (only the names here):
    // 1. Let list be an empty list of name-value pairs.
    List<DOMString> names;

    // 2. For each content attribute on the DOMStringMap's associated element whose first five characters are
    //    the string "data-" and whose remaining characters (if any) do not include any ASCII upper alphas, in
    //    the order that those attributes are listed in the element's attribute list, add a name-value pair to
    //    list whose name is the attribute's name with the first five characters removed and whose value is
    //    the attribute's value.
    // 3. For each name in list, for each U+002D HYPHEN-MINUS character (-) in the name that is followed by an
    //    ASCII lower alpha, remove the U+002D HYPHEN-MINUS character (-) and replace the character that
    //    followed it by the same character converted to ASCII uppercase.
    for (const auto &attribute : _element.AttributeList())
    {
      auto localName = attribute->LocalName().View();
      if (IsDatasetAttributeName(localName))
      {
        names.push_back(DatasetAttributeNameToPropertyName(localName));
      }
    }

    // 4. Return list.
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

  ExceptionOr<void> DatasetDOMStringMap::SetNamedItem(DOMStringView name, DOMString value) noexcept
  {
    html::CEReactionsScope ceReactions;

    // 1. If name contains a U+002D HYPHEN-MINUS character (-) followed by an ASCII lower alpha, then throw a
    //    "SyntaxError" DOMException.
    if (!IsValidDatasetPropertyName(name))
    {
      return Exception {ExceptionCode::SyntaxError};
    }

    // 2. For each ASCII upper alpha in name, insert a U+002D HYPHEN-MINUS character (-) before the character
    //    and replace the character with the same character converted to ASCII lowercase.
    // 3. Insert the string data- at the front of name.
    // 4. If name is not a valid attribute local name, then throw an "InvalidCharacterError" DOMException.
    // 5. Set an attribute value for the DOMStringMap's associated element using name and value.
    // (setAttribute() checks the name; it has no ASCII uppercase letters to lowercase.)
    return _element.SetAttribute(DOMStringAtom(DatasetPropertyNameToAttributeName(name)), krys::move(value));
  }

  void DatasetDOMStringMap::DeleteNamedProperty(DOMStringView name) noexcept
  {
    html::CEReactionsScope ceReactions;

    // 1. For each ASCII upper alpha in name, insert a U+002D HYPHEN-MINUS character (-) before the character
    //    and replace the character with the same character converted to ASCII lowercase.
    // 2. Insert the string data- at the front of name.
    // 3. Remove an attribute by name given name and the DOMStringMap's associated element.
    _element.RemoveAttribute(DOMStringAtom(DatasetPropertyNameToAttributeName(name)));
  }
}
