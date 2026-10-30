#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/CSSStyleProperties.hpp"
#include "Krystal.Booey/CSS/Parser/Parser.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/ParserMode.hpp"
#include "Krystal.Booey/CSS/Properties/ImmutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyShorthand.hpp"
#include "Krystal.Booey/CSS/Properties/StylePropertiesInlines.hpp"
#include "Krystal.Booey/CSS/Values/CustomPropertyValue.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/DOM/StyledElement.hpp"
#include <algorithm>
#include <iterator>

namespace krys::boo::css
{
  MutableStyleProperties::MutableStyleProperties(css::ParserMode mode) noexcept : StyleProperties(mode)
  {
  }

  MutableStyleProperties::MutableStyleProperties(List<Property> &&properties) noexcept
      : StyleProperties(css::ParserMode::HTMLStandard),
        _propertyVector(std::make_move_iterator(properties.begin()),
                        std::make_move_iterator(properties.end()))
  {
  }

  MutableStyleProperties::~MutableStyleProperties() noexcept = default;

  MutableStyleProperties::MutableStyleProperties(const StyleProperties &other) noexcept
      : StyleProperties(other.ParserMode())
  {
    if (auto *mutableProperties = DynamicDowncast<MutableStyleProperties>(other))
    {
      _propertyVector = mutableProperties->_propertyVector;
    }
    else
    {
      for (auto property : Downcast<ImmutableStyleProperties>(other))
      {
        _propertyVector.push_back(property.ToProperty());
      }
    }
  }

  CSSStyleProperties *MutableStyleProperties::CssStyleProperties() noexcept
  {
    return _cssomWrapper.get();
  }

  CSSStyleProperties &MutableStyleProperties::EnsureCSSStyleProperties() noexcept
  {
    if (_cssomWrapper)
    {
      krys_debug_assert(!_cssomWrapper->ParentRule());
      krys_debug_assert(!_cssomWrapper->ParentElement());
      return *_cssomWrapper;
    }
    _cssomWrapper = CreateUnique<PropertySetCSSStyleProperties>(*this);
    return *_cssomWrapper;
  }

  CSSStyleProperties &
    MutableStyleProperties::EnsureInlineCSSStyleProperties(dom::StyledElement &parentElement) noexcept
  {
    if (!_cssomWrapper)
    {
      _cssomWrapper = CreateUnique<InlineCSSStyleProperties>(*this, parentElement);
    }
    krys_debug_assert(_cssomWrapper->ParentElement() == &parentElement);
    return *_cssomWrapper;
  }

  Ref<MutableStyleProperties> MutableStyleProperties::Create() noexcept
  {
    return AdoptRef(*new MutableStyleProperties(css::ParserMode::HTMLQuirks));
  }

  Ref<MutableStyleProperties> MutableStyleProperties::Create(css::ParserMode mode) noexcept
  {
    return AdoptRef(*new MutableStyleProperties(mode));
  }

  Ref<MutableStyleProperties> MutableStyleProperties::Create(List<Property> &&properties) noexcept
  {
    return AdoptRef(*new MutableStyleProperties(krys::move(properties)));
  }

  Ref<MutableStyleProperties> MutableStyleProperties::CreateEmpty() noexcept
  {
    return AdoptRef(*new MutableStyleProperties(List<Property> {}));
  }

  Ref<ImmutableStyleProperties> MutableStyleProperties::ImmutableCopy() const noexcept
  {
    return ImmutableStyleProperties::Create(
      Span<const Property>(_propertyVector.data(), _propertyVector.size()), ParserMode());
  }

  Ref<ImmutableStyleProperties> MutableStyleProperties::ImmutableDeduplicatedCopy() const noexcept
  {
    return ImmutableStyleProperties::CreateDeduplicating(
      Span<const Property>(_propertyVector.data(), _propertyVector.size()), ParserMode());
  }

  bool MutableStyleProperties::RemoveShorthandProperty(PropertyId property, CSSOMString *returnText) noexcept
  {
    // FIXME: Use SerialiseShorthandValue here to return the value of the removed shorthand as we do when
    // removing a longhand.
    if (returnText)
    {
      *returnText = CSSOMString();
    }

    return RemoveProperties(ShorthandForProperty(property).Properties());
  }

  bool MutableStyleProperties::RemovePropertyAtIndex(Maybe<uint32> index, CSSOMString *returnText) noexcept
  {
    if (!index)
    {
      if (returnText)
      {
        *returnText = CSSOMString();
      }

      return false;
    }

    if (returnText)
    {
      auto property = PropertyAt(*index);
      *returnText =
        css::SerialiseLonghandValue(DefaultSerialisationContext(), property.Id(), *property.Value());
    }

    // A more efficient removal strategy would involve marking entries as empty
    // and sweeping them when the vector grows too big.
    _propertyVector.erase(_propertyVector.begin() + *index);
    return true;
  }

  bool MutableStyleProperties::RemoveLonghandProperty(PropertyId property, CSSOMString *returnText) noexcept
  {
    return RemovePropertyAtIndex(FindPropertyIndex(property), returnText);
  }

  bool MutableStyleProperties::RemoveProperty(PropertyId property, CSSOMString *returnText) noexcept
  {
    return IsLonghand(property) ? RemoveLonghandProperty(property, returnText)
                                : RemoveShorthandProperty(property, returnText);
  }

  bool MutableStyleProperties::RemoveCustomProperty(CSSOMStringView propertyName,
                                                    CSSOMString *returnText) noexcept
  {
    return RemovePropertyAtIndex(FindCustomPropertyIndex(propertyName), returnText);
  }

  bool MutableStyleProperties::SetProperty(PropertyId property, const CSSOMString &value,
                                           ParserContext context, IsImportant important,
                                           bool *didFailParsing) noexcept
  {
    if (!IsExposed(property, &context.PropertySettings) && !IsInternal(property))
    {
      // Allow internal properties as we use them to handle certain DOM-exposed values
      // (e.g. -webkit-font-size-delta from execCommand('FontSizeDelta')).
      return false;
    }

    // Setting the value to an empty string just removes the property in both IE and Gecko.
    // Setting it to null seems to produce less consistent results, but we treat it just the same.
    if (value.empty())
    {
      return RemoveProperty(property);
    }

    // When replacing an existing property value, this moves the property to the end of the list.
    // Firefox preserves the position, and MSIE moves the property to the beginning.
    context.Mode = ParserMode();
    auto parseResult = Parser::ParseValue(*this, property, value, important, context);
    if (didFailParsing)
    {
      *didFailParsing = parseResult == Parser::ParseResult::Error;
    }
    return parseResult == Parser::ParseResult::Changed;
  }

  bool MutableStyleProperties::SetProperty(PropertyId property, const CSSOMString &value,
                                           IsImportant important, bool *didFailParsing) noexcept
  {
    ParserContext context {ParserMode()};
    return SetProperty(property, value, context, important, didFailParsing);
  }

  bool MutableStyleProperties::SetCustomProperty(const CSSOMString &propertyName, const CSSOMString &value,
                                                 ParserContext context, IsImportant important) noexcept
  {
    // Setting the value to an empty string just removes the property in both IE and Gecko.
    // Setting it to null seems to produce less consistent results, but we treat it just the same.
    if (value.empty())
    {
      return RemoveCustomProperty(propertyName);
    }

    // When replacing an existing property value, this moves the property to the end of the list.
    // Firefox preserves the position, and MSIE moves the property to the beginning.
    context.Mode = ParserMode();
    return Parser::ParseCustomPropertyValue(*this, CSSOMStringAtom(propertyName), value, important, context)
           == Parser::ParseResult::Changed;
  }

  void MutableStyleProperties::SetProperty(PropertyId property, Ref<css::Value> &&value,
                                           IsImportant important) noexcept
  {
    if (IsLonghand(property))
    {
      SetProperty(Property(property, krys::move(value), important));
      return;
    }

    auto shorthand = ShorthandForProperty(property);
    RemoveProperties(shorthand.Properties());
    for (auto longhand : shorthand)
    {
      _propertyVector.push_back(Property(longhand, Ref<css::Value>(value), important));
    }
  }

  bool MutableStyleProperties::CanUpdateInPlace(const Property &property,
                                                const Property *toReplace) const noexcept
  {
    // If the property is in a logical property group, we can't just update the value in-place,
    // because afterwards there might be another property of the same group but different mapping logic.
    // In that case the latter might override the former, so setProperty would have no effect.
    auto id = property.Id();
    if (Property::IsInLogicalPropertyGroup(id))
    {
      krys_debug_assert(toReplace >= _propertyVector.data());
      krys_debug_assert(toReplace < _propertyVector.data() + _propertyVector.size());
      for (auto *other = toReplace + 1; other != _propertyVector.data() + _propertyVector.size(); ++other)
      {
        if (Property::AreInSameLogicalPropertyGroupWithDifferentMappingLogic(id, other->Id()))
        {
          return false;
        }
      }
    }

    return true;
  }

  bool MutableStyleProperties::SetProperty(const Property &property, Property *slot) noexcept
  {
    krys_debug_assert(property.Id() == PropertyId::Custom || IsLonghand(property.Id()));
    auto *toReplace = slot;
    if (!slot)
    {
      if (property.Id() == PropertyId::Custom)
      {
        toReplace =
          FindCustomCSSPropertyWithName(Downcast<CustomPropertyValue>(property.Value()).Name().View());
      }
      else
      {
        toReplace = FindCSSPropertyWithId(property.Id());
      }
    }

    if (toReplace)
    {
      if (CanUpdateInPlace(property, toReplace))
      {
        if (*toReplace == property)
        {
          return false;
        }

        *toReplace = property;
        return true;
      }

      _propertyVector.erase(_propertyVector.begin() + (toReplace - _propertyVector.data()));
    }

    _propertyVector.push_back(property);
    return true;
  }

  bool MutableStyleProperties::SetProperty(PropertyId property, ValueId identifier,
                                           IsImportant important) noexcept
  {
    krys_debug_assert(IsLonghand(property));
    return SetProperty(Property(property, PrimitiveValue::Create(identifier), important));
  }

  bool MutableStyleProperties::ParseDeclaration(const CSSOMString &styleDeclaration,
                                                ParserContext context) noexcept
  {
    auto oldProperties = krys::move(_propertyVector);
    _propertyVector.clear();

    context.Mode = ParserMode();
    Parser::ParseDeclarationList(*this, styleDeclaration, context);

    // We could do better. Just changing property order does not require style invalidation.
    return oldProperties != _propertyVector;
  }

  bool MutableStyleProperties::AddParsedProperties(const ParsedPropertyList &properties) noexcept
  {
    bool anyChanged = false;
    _propertyVector.reserve(_propertyVector.size() + properties.size());
    for (const auto &property : properties)
    {
      if (AddParsedProperty(property))
      {
        anyChanged = true;
      }
    }

    return anyChanged;
  }

  bool MutableStyleProperties::AddParsedProperty(const Property &property) noexcept
  {
    if (property.Id() == PropertyId::Custom)
    {
      if (!CustomPropertyIsImportant(Downcast<CustomPropertyValue>(property.Value()).Name().View())
          || property.Important())
      {
        return SetProperty(property);
      }

      return false;
    }

    return SetProperty(property);
  }

  bool MutableStyleProperties::MergeAndOverrideOnConflict(const StyleProperties &other) noexcept
  {
    bool changed = false;
    for (auto property : other)
    {
      changed |= AddParsedProperty(property.ToProperty());
    }

    return changed;
  }

  void MutableStyleProperties::Clear() noexcept
  {
    _propertyVector.clear();
  }

  bool MutableStyleProperties::RemoveProperties(Span<const PropertyId> properties) noexcept
  {
    if (_propertyVector.empty())
    {
      return false;
    }

    auto removed =
      std::remove_if(_propertyVector.begin(), _propertyVector.end(), [&](const Property &property)
                     { return std::ranges::find(properties, property.Id()) != properties.end(); });
    if (removed == _propertyVector.end())
    {
      return false;
    }

    _propertyVector.erase(removed, _propertyVector.end());
    return true;
  }

  Maybe<uint32> MutableStyleProperties::FindPropertyIndex(PropertyId property) const noexcept
  {
    for (auto n = PropertyCount(); n--;)
    {
      if (_propertyVector[n].Id() == property)
      {
        return n;
      }
    }

    return null;
  }

  Maybe<uint32> MutableStyleProperties::FindCustomPropertyIndex(CSSOMStringView propertyName) const noexcept
  {
    for (auto n = PropertyCount(); n--;)
    {
      auto &property = _propertyVector[n];
      if (property.Id() == PropertyId::Custom)
      {
        // We found a custom property. See if the name matches.
        if (Downcast<CustomPropertyValue>(property.Value()).Name().View() == propertyName)
        {
          return n;
        }
      }
    }

    return null;
  }

  Property *MutableStyleProperties::FindCSSPropertyWithId(PropertyId property) noexcept
  {
    auto foundPropertyIndex = FindPropertyIndex(property);
    if (!foundPropertyIndex)
    {
      return nullptr;
    }

    return &_propertyVector[*foundPropertyIndex];
  }

  Property *MutableStyleProperties::FindCustomCSSPropertyWithName(CSSOMStringView propertyName) noexcept
  {
    auto foundPropertyIndex = FindCustomPropertyIndex(propertyName);
    if (!foundPropertyIndex)
    {
      return nullptr;
    }

    return &_propertyVector[*foundPropertyIndex];
  }
}
