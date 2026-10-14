#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
#include "Krystal.Booey/Core/IterationStatus.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Color.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Font.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyInitialValues.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyShorthand.hpp"
#include "Krystal.Booey/CSS/Properties/ShorthandSerialiser.hpp"
#include "Krystal.Booey/CSS/Properties/StylePropertiesInlines.hpp"
#include "Krystal.Booey/CSS/Values/ColorValue.hpp"
#include "Krystal.Booey/CSS/Values/CustomPropertyValue.hpp"
#include "Krystal.Booey/CSS/Values/PendingSubstitutionValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include <bitset>
#include <print>

namespace krys::boo::css
{
  // FIXME: Generate this from the property definitions and use it for PropertyShorthandList too.
  constexpr size_t MaxShorthandsForLonghand = 4uz;

  Ref<ImmutableStyleProperties> StyleProperties::ImmutableCopyIfNeeded() const noexcept
  {
    if (_isMutable)
    {
      return Downcast<MutableStyleProperties>(*this).ImmutableDeduplicatedCopy();
    }

    return ShareRef(const_cast<ImmutableStyleProperties &>(Downcast<ImmutableStyleProperties>(*this)));
  }

  CSSOMString SerialiseLonghandValue(const SerialisationContext &context, PropertyId property,
                                     const Value &value) noexcept
  {
    // Longhands set by mask and background shorthands can have comma-separated lists with implicit initial
    // values in them. We need to serialize those lists with the actual values, not as "initial". Doing this
    // for all CSSValueList with comma separators is better than checking the property is one of those
    // longhands. Serializing this way is harmless for other properties; those won't have any implicit initial
    // values.
    if (auto *list = DynamicDowncast<ValueList>(value);
        list && list->Separator() == ValueSeparator::CommaSeparator)
    {
      CSSOMString result;
      CSSOMStringView separator = u8"";
      for (auto &individualValue : *list)
      {
        result.append(std::exchange(separator, u8", "));
        result.append(SerialiseLonghandValue(context, property, individualValue));
      }

      return result;
    }

    return value.IsImplicitInitialValue() ? CSSOMString(InitialValueTextForLonghand(property))
                                          : value.CssText(context);
  }

  inline CSSOMString StyleProperties::SerialiseLonghandValue(const SerialisationContext &context,
                                                             PropertyId property) const noexcept
  {
    return css::SerialiseLonghandValue(context, property, GetPropertyCSSValue(property).get());
  }

  inline Maybe<CSSOMString> StyleProperties::SerialiseShorthandValue(const SerialisationContext &context,
                                                                     PropertyId property) const noexcept
  {
    return css::SerialiseShorthandValue(context, *this, property);
  }

  CSSOMString StyleProperties::GetPropertyValue(PropertyId property) const noexcept
  {
    return IsLonghand(property)
             ? SerialiseLonghandValue(DefaultSerialisationContext(), property)
             : SerialiseShorthandValue(DefaultSerialisationContext(), property).value_or(u8"");
  }

  Maybe<boo::Color> StyleProperties::PropertyAsColor(PropertyId property) const noexcept
  {
    auto value = GetPropertyCSSValue(property);
    if (!value)
    {
      return null;
    }

    if (value->IsColorValue())
    {
      return ColorValue::GetAbsoluteColor(*value);
    }

    auto serialisationString = css::SerialiseLonghandValue(DefaultSerialisationContext(), property, *value);
    return PropertyParserHelpers::DeprecatedParseColorRawWithoutContext(serialisationString);
  }

  Maybe<ValueId> StyleProperties::PropertyAsValueId(PropertyId property) const noexcept
  {
    return LonghandValueId(property, GetPropertyCSSValue(property).get());
  }

  CSSOMString StyleProperties::GetCustomPropertyValue(CSSOMStringView propertyName) const noexcept
  {
    if (auto value = GetCustomPropertyCSSValue(propertyName))
    {
      return value->CssText(DefaultSerialisationContext());
    }

    return CSSOMString();
  }

  RefPtr<Value> StyleProperties::GetPropertyCSSValue(PropertyId property) const noexcept
  {
    auto foundPropertyIndex = FindPropertyIndex(property);
    if (!foundPropertyIndex)
    {
      return nullptr;
    }

    auto reference = PropertyAt(*foundPropertyIndex);
    auto *value = reference.Value();
    // System fonts are represented as CSSPrimitiveValue for various font subproperties, but these must
    // serialize as the empty string. It might be better to implement this as a special CSSValue type instead
    // of turning them into null here.
    if (reference.ShorthandId() == PropertyId::Font
        && PropertyParserHelpers::IsSystemFontShorthand(GetValueId(value)))
    {
      return nullptr;
    }

    return ShareRefPtr(const_cast<Value *>(value));
  }

  RefPtr<Value> StyleProperties::GetCustomPropertyCSSValue(CSSOMStringView propertyName) const noexcept
  {
    auto foundPropertyIndex = FindCustomPropertyIndex(propertyName);
    if (!foundPropertyIndex)
    {
      return nullptr;
    }

    return ShareRefPtr(const_cast<Value *>(PropertyAt(*foundPropertyIndex).Value()));
  }

  bool StyleProperties::PropertyIsImportant(PropertyId property) const noexcept
  {
    if (IsLonghand(property))
    {
      auto foundPropertyIndex = FindPropertyIndex(property);
      if (foundPropertyIndex)
      {
        return PropertyAt(*foundPropertyIndex).IsImportant();
      }

      return false;
    }

    krys_debug_assert(ShorthandForProperty(property).Size());
    for (auto longhand : ShorthandForProperty(property))
    {
      if (!PropertyIsImportant(longhand))
      {
        return false;
      }
    }

    return true;
  }

  bool StyleProperties::CustomPropertyIsImportant(CSSOMStringView propertyName) const noexcept
  {
    auto foundPropertyIndex = FindCustomPropertyIndex(propertyName);
    if (foundPropertyIndex)
    {
      return PropertyAt(*foundPropertyIndex).IsImportant();
    }

    return false;
  }

  CSSOMStringView StyleProperties::GetPropertyShorthand(PropertyId property) const noexcept
  {
    auto foundPropertyIndex = FindPropertyIndex(property);
    if (!foundPropertyIndex)
    {
      return {};
    }

    return ToString(PropertyAt(*foundPropertyIndex).ShorthandId());
  }

  bool StyleProperties::IsPropertyImplicit(PropertyId property) const noexcept
  {
    auto foundPropertyIndex = FindPropertyIndex(property);
    if (!foundPropertyIndex)
    {
      return false;
    }

    return PropertyAt(*foundPropertyIndex).IsImplicit();
  }

  CSSOMString StyleProperties::AsText(const SerialisationContext &context) const noexcept
  {
    return AsTextInternal(context);
  }

  CSSOMStringAtom StyleProperties::AsTextAtom(const SerialisationContext &context) const noexcept
  {
    return CSSOMStringAtom(AsTextInternal(context));
  }

  KRYS_NODISCARD constexpr static bool CanUseShorthandForLonghand(PropertyId shorthandId,
                                                                  PropertyId longhandId) noexcept
  {
    krys_debug_assert(IsShorthand(shorthandId));
    krys_debug_assert(IsLonghand(longhandId));
    switch (shorthandId)
    {
      // We are not yet using the font shorthand here because our editing code is currently incompatible.
      case PropertyId::Font:                 return false;

      // Avoid legacy shorthands according to https://www.w3.org/TR/css-cascade-5/#legacy-shorthand
      case PropertyId::PageBreakAfter:
      case PropertyId::PageBreakBefore:
      case PropertyId::PageBreakInside:
      case PropertyId::WebkitBackgroundSize:
      case PropertyId::WebkitMaskPosition:   return false;

      // No other browser currently supports text-decoration-skip, so it's currently more web
      // compatible to avoid collapsing text-decoration-skip-ink, its only longhand.
      case PropertyId::TextDecorationSkip:   return false;

      // FIXME: more mask nonsense.
      case PropertyId::Mask:
      {
        return longhandId != PropertyId::MaskComposite && longhandId != PropertyId::MaskMode
               && longhandId != PropertyId::MaskSize;
      }

      // FIXME: If font-variant-ligatures is none, this depends on the value of the longhand.
      case PropertyId::FontVariant:
      // FIXME: These shorthands are avoided for unknown legacy reasons, probably shouldn't be avoided.
      case PropertyId::ColumnRule:
      case PropertyId::Columns:
      case PropertyId::Container:
      case PropertyId::FontSynthesis:
      case PropertyId::GridArea:
      case PropertyId::GridColumn:
      case PropertyId::GridRow:
      case PropertyId::MaskPosition:
      case PropertyId::Offset:
      case PropertyId::TextEmphasis:  return false;

      default:                        return true;
    }
  }

  CSSOMString StyleProperties::AsTextInternal(const SerialisationContext &context) const noexcept
  {
    CSSOMString result;

    constexpr auto shorthandPropertyCount =
      static_cast<size_t>(LastShorthandProperty) - static_cast<size_t>(FirstShorthandProperty) + 1uz;
    std::bitset<shorthandPropertyCount> shorthandPropertyUsed;
    std::bitset<shorthandPropertyCount> shorthandPropertyAppeared;

    auto numDecls = 0u;
    for (auto property : *this)
    {
      auto propertyId = property.Id();
      krys_debug_assert(IsLonghand(propertyId) || propertyId == PropertyId::Custom);
      SmallList<PropertyId, MaxShorthandsForLonghand> shorthands;

      if (auto *substitutionValue = DynamicDowncast<PendingSubstitutionValue>(property.Value()))
      {
        shorthands.push_back(substitutionValue->ShorthandPropertyId());
      }
      else
      {
        for (auto &shorthand : MatchingShorthandsForLonghand(propertyId))
        {
          if (CanUseShorthandForLonghand(shorthand.Id(), propertyId))
          {
            shorthands.push_back(shorthand.Id());
          }
        }
      }

      Maybe<CSSOMString> value;
      bool alreadyUsedShorthand = false;
      for (auto &shorthandPropertyId : shorthands)
      {
        krys_debug_assert(IsShorthand(shorthandPropertyId));
        auto shorthandPropertyIndex =
          static_cast<size_t>(shorthandPropertyId) - static_cast<size_t>(FirstShorthandProperty);

        krys_debug_assert(shorthandPropertyIndex < shorthandPropertyUsed.size());
        if (shorthandPropertyUsed[shorthandPropertyIndex])
        {
          alreadyUsedShorthand = true;
          break;
        }

        if (shorthandPropertyAppeared[shorthandPropertyIndex])
        {
          continue;
        }

        shorthandPropertyAppeared.set(shorthandPropertyIndex);

        value = SerialiseShorthandValue(context, shorthandPropertyId);
        if (value)
        {
          propertyId = shorthandPropertyId;
          shorthandPropertyUsed.set(shorthandPropertyIndex);
          break;
        }
      }

      if (alreadyUsedShorthand)
      {
        continue;
      }

      if (!value)
      {
        value = css::SerialiseLonghandValue(context, propertyId, *property.Value());
      }

      if (numDecls++)
      {
        result.push_back(u8' ');
      }

      if (propertyId == PropertyId::Custom)
      {
        result.append(Downcast<CustomPropertyValue>(*property.Value()).Name().View());
      }
      else
      {
        result.append(ToString(propertyId));
      }

      result.append(u8": ");
      result.append(*value);
      result.append(property.IsImportant() ? u8" !important" : u8"");
      result.push_back(u8';');
    }

    krys_debug_assert(!numDecls ^ !result.empty());
    return result;
  }

  bool StyleProperties::HasCSSOMWrapper() const noexcept
  {
    const auto *mutableProperties = DynamicDowncast<MutableStyleProperties>(*this);
    return mutableProperties && mutableProperties->_cssomWrapper;
  }

  bool StyleProperties::TraverseSubresources(
    KRYS_NOESCAPE const Func<bool(const CachedResource &)> &handler) const noexcept
  {
    for (auto property : *this)
    {
      if (property.Value()->TraverseSubresources(handler))
      {
        return true;
      }
    }

    return false;
  }

  bool StyleProperties::MayDependOnBaseURL() const noexcept
  {
    bool result = false;
    Func<IterationStatus(Value &)> func = [&](Value &value) -> IterationStatus
    {
      if (value.MayDependOnBaseUrl())
      {
        result = true;
        return IterationStatus::Done;
      }

      return value.VisitChildren(func);
    };

    for (auto property : *this)
    {
      if (func(*property.Value()) == IterationStatus::Done)
      {
        return result;
      }
    }

    return false;
  }

  bool StyleProperties::PropertyMatches(PropertyId property, const Value *propertyValue) const noexcept
  {
    auto foundPropertyIndex = FindPropertyIndex(property);
    if (!foundPropertyIndex)
    {
      return false;
    }

    return PropertyAt(*foundPropertyIndex).Value()->Equals(*propertyValue);
  }

  Ref<MutableStyleProperties> StyleProperties::MutableCopy() const noexcept
  {
    return AdoptRef(*new MutableStyleProperties(*this));
  }

  Ref<MutableStyleProperties>
    StyleProperties::CopyProperties(Span<const PropertyId> properties) const noexcept
  {
    List<Property> list;
    for (auto property : properties)
    {
      if (auto value = GetPropertyCSSValue(property))
      {
        list.emplace_back(property, Ref<Value>(krys::move(value)));
      }
    }

    return MutableStyleProperties::Create(krys::move(list));
  }

  size_t StyleProperties::AverageSizeInBytes() noexcept
  {
    // Please update this if the storage scheme changes so that this longer reflects the actual size.
    return ImmutableStyleProperties::ObjectSize(4u);
  }

  // See the function above if you need to update this.
  struct SameSizeAsStyleProperties : public RefCounted<SameSizeAsStyleProperties>
  {
    uint32 Bitfield;
  };
  static_assert(sizeof(StyleProperties) == sizeof(SameSizeAsStyleProperties),
                "style property set should stay small");

#if KRYS_DEBUG
  void StyleProperties::ShowStyle() const noexcept
  {
    auto text = AsText(DefaultSerialisationContext());
    std::println(stderr, "{}", std::string_view(reinterpret_cast<const char *>(text.data()), text.size()));
  }
#endif

  CSSOMString StyleProperties::PropertyReference::CssName() const noexcept
  {
    if (Id() == PropertyId::Custom)
    {
      return CSSOMString(Downcast<CustomPropertyValue>(*Value()).Name().View());
    }

    return CSSOMString(ToString(Id()));
  }

  CSSOMString StyleProperties::PropertyReference::CssText(const SerialisationContext &context) const noexcept
  {
    return CssName() + u8": " + css::SerialiseLonghandValue(context, Id(), *_value)
           + (IsImportant() ? u8" !important;" : u8";");
  }
}
