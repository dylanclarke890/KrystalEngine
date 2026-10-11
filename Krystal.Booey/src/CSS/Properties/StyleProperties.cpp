#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
#include "Krystal.Booey/Core/IterationStatus.hpp"
#include "Krystal.Booey/CSS/CSSStyleProperties.hpp"
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

namespace krys::boo::css
{
  // FIXME: Generate this from CSSProperties.json and use it for StylePropertyShorthandVector too.
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
      CSSOMString separator = u8"";

      for (auto &individualValue : *list)
      {
        result.append(std::exchange(separator, u8", "));
        result.append(SerialiseLonghandValue(context, property, individualValue));
      }

      return result;
    }

    return value.IsImplicitInitialValue() ? InitialValueTextForLonghand(property) : value.CssText(context);
  }

  inline CSSOMString StyleProperties::SerialiseLonghandValue(const SerialisationContext &context,
                                                             PropertyId propertyId) const noexcept
  {
    return krys::boo::css::SerialiseLonghandValue(context, propertyId, GetPropertyCSSValue(propertyId).get());
  }

  inline CSSOMString StyleProperties::SerialiseShorthandValue(const SerialisationContext &context,
                                                              PropertyId propertyId) const noexcept
  {
    return krys::boo::css::SerialiseShorthandValue(context, *this, propertyId);
  }

  CSSOMString StyleProperties::GetPropertyValue(PropertyId propertyId) const noexcept
  {
    return IsLonghand(propertyId) ? SerialiseLonghandValue(DefaultSerialisationContext(), propertyId)
                                  : SerialiseShorthandValue(DefaultSerialisationContext(), propertyId);
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
      return ColorValue::AbsoluteColor(*value);
    }

    auto serializationString =
      krys::boo::css::SerialiseLonghandValue(DefaultSerialisationContext(), property, *value);

    return PropertyParserHelpers::DeprecatedParseColorRawWithoutContext(serializationString);
  }

  Maybe<ValueId> StyleProperties::PropertyAsValueId(PropertyId property) const noexcept
  {
    return LonghandValueId(property, GetPropertyCSSValue(property).get());
  }

  CSSOMString StyleProperties::GetCustomPropertyValue(const CSSOMString &propertyName) const noexcept
  {
    RefPtr<Value> value = GetCustomPropertyCSSValue(propertyName);
    if (value)
    {
      return value->CssText(DefaultSerialisationContext());
    }

    return CSSOMString();
  }

  RefPtr<Value> StyleProperties::GetPropertyCSSValue(PropertyId propertyID) const noexcept
  {
    auto foundPropertyIndex = FindPropertyIndex(propertyID);
    if (!foundPropertyIndex.has_value())
    {
      return nullptr;
    }

    auto property = PropertyAt(*foundPropertyIndex);
    auto value = property.Value();

    // System fonts are represented as CSSPrimitiveValue for various font subproperties, but these must
    // serialize as the empty string. It might be better to implement this as a special CSSValue type instead
    // of turning them into null here.
    if (property.ShorthandId() == PropertyId::Font
        && PropertyParserHelpers::IsSystemFontShorthand(GetValueId(value)))
    {
      return nullptr;
    }

    return ShareRefPtr(value);
  }

  RefPtr<Value> StyleProperties::GetCustomPropertyCSSValue(const CSSOMString &propertyName) const noexcept
  {
    auto foundPropertyIndex = FindCustomPropertyIndex(propertyName);
    if (!foundPropertyIndex.has_value())
    {
      return nullptr;
    }

    return ShareRefPtr(PropertyAt(*foundPropertyIndex).Value());
  }

  bool StyleProperties::PropertyIsImportant(PropertyId propertyId) const noexcept
  {
    if (IsLonghand(propertyId))
    {
      auto index = FindPropertyIndex(propertyId);
      return index ? PropertyAt(*index).IsImportant() : false;
    }

    krys_assert(ShorthandForProperty(propertyId).Size());
    for (auto longhand : ShorthandForProperty(propertyId))
    {
      if (!PropertyIsImportant(longhand))
      {
        return false;
      }
    }

    return true;
  }

  bool StyleProperties::CustomPropertyIsImportant(const CSSOMString &propertyName) const noexcept
  {
    auto index = FindCustomPropertyIndex(propertyName);
    return index ? PropertyAt(*index).IsImportant() : false;
  }

  CSSOMStringView StyleProperties::GetPropertyShorthand(PropertyId propertyId) const noexcept
  {
    auto index = FindPropertyIndex(propertyId);
    return index ? ToString(PropertyAt(*index).ShorthandId()) : CSSOMStringView {};
  }

  bool StyleProperties::IsPropertyImplicit(PropertyId propertyId) const noexcept
  {
    auto index = FindPropertyIndex(propertyId);
    return index ? PropertyAt(*index).IsImplicit() : false;
  }

  CSSOMString StyleProperties::AsText(const SerialisationContext &context) const noexcept
  {
    return AsTextInternal(context);
  }

  CSSOMStringAtom StyleProperties::AsTextAtom(const SerialisationContext &context) const noexcept
  {
    return AsTextInternal(context);
  }

  constexpr static bool CanUseShorthandForLonghand(PropertyId shorthandId, PropertyId longhandId) noexcept
  {
    krys_assert(IsShorthand(shorthandId));
    krys_assert(IsLonghand(longhandId));
    switch (shorthandId)
    {
      // We are not yet using the PropertyId::Font shorthand here because our editing code is currently
      // incompatible.
      case PropertyId::Font: return false;

      // Avoid legacy shorthands according to https://www.w3.org/TR/css-cascade-5/#legacy-shorthand
      case PropertyId::PageBreakAfter:
      case PropertyId::PageBreakBefore:
      case PropertyId::PageBreakInside:
      case PropertyId::WebkitBackgroundSize:
      case PropertyId::WebkitMaskPosition:
      {
        return false;
      }
      // No other browser currently supports text-decoration-skip, so it's currently more web compatible to
      // avoid collapsing text-decoration-skip-ink, its only longhand.
      case PropertyId::TextDecorationSkip:
      {
        return false;
      }
      case PropertyId::Mask: // FIXME: more mask nonsense.
      {
        return longhandId != PropertyId::MaskComposite && longhandId != PropertyId::MaskMode
               && longhandId != PropertyId::MaskSize;
      }

      case PropertyId::FontVariant: // FIXME: If font-variant-ligatures is none, this depends on the value of
                                    // the longhand.
      case PropertyId::ColumnRule: // FIXME: These shorthands are avoided for unknown legacy reasons, probably
                                   // shouldn't be avoided.
      case PropertyId::Columns:
      case PropertyId::Container:
      case PropertyId::FontSynthesis:
      case PropertyId::GridArea:
      case PropertyId::GridColumn:
      case PropertyId::GridRow:
      case PropertyId::MaskPosition:
      case PropertyId::Offset:
      case PropertyId::TextEmphasis:
      {
        return false;
      }
      default:
      {
        return true;
      }
    }
  }

  CSSOMString StyleProperties::AsTextInternal(const SerialisationContext &context) const noexcept
  {
    CSSOMString result;

    constexpr size_t shorthandPropertyCount =
      ToUnderlying(LastShorthandProperty) - ToUnderlying(FirstShorthandProperty) + 1uz;
    std::bitset<shorthandPropertyCount> shorthandPropertyUsed;
    std::bitset<shorthandPropertyCount> shorthandPropertyAppeared;

    size_t numDecls = 0uz;
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

      CSSOMString value;
      bool alreadyUsedShorthand = false;
      for (auto &shorthandPropertyId : shorthands)
      {
        krys_debug_assert(IsShorthand(shorthandPropertyId));
        size_t shorthandPropertyIndex =
          ToUnderlying(shorthandPropertyId) - ToUnderlying(FirstShorthandProperty);

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
        if (!value.empty())
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

      if (value.empty())
      {
        value = krys::boo::css::SerialiseLonghandValue(context, propertyId, *property.Value());
      }

      if (numDecls++)
      {
        result.push_back(' ');
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
      result.append(value);
      if (property.IsImportant())
      {
        result.append(u8" !important");
      }
      result.push_back(';');
    }

    krys_assert(!numDecls ^ !result.empty());
    return result;
  }

  bool StyleProperties::HasCSSOMWrapper() const noexcept
  {
    auto *mutableProperties = DynamicDowncast<MutableStyleProperties>(*this);
    return mutableProperties && mutableProperties->_cssomWrapper;
  }

  bool StyleProperties::TraverseSubresources(const Func<bool(const CachedResource &)> &handler) const noexcept
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
      if (value.MayDependOnBaseURL())
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

  bool StyleProperties::PropertyMatches(PropertyId propertyId, const Value *propertyValue) const noexcept
  {
    auto index = FindPropertyIndex(propertyId);
    return index ? PropertyAt(*index).Value()->Equals(*propertyValue) : false;
  }

  Ref<MutableStyleProperties> StyleProperties::MutableCopy() const noexcept
  {
    return AdoptRef(*new MutableStyleProperties(*this));
  }

  Ref<MutableStyleProperties>
    StyleProperties::CopyProperties(Span<const PropertyId> properties) const noexcept
  {
    SmallList<Property> copy {};
    for (auto &property : properties)
    {
      if (auto value = GetPropertyCSSValue(property))
      {
        copy.push_back(Property(property, krys::move(value)));
      }
    }

    return MutableStyleProperties::Create(krys::move(copy));
  }

  CSSStyleProperties *MutableStyleProperties::StyleProperties() noexcept
  {
    return _cssomWrapper.get();
  }

  CSSStyleProperties &MutableStyleProperties::EnsureCSSStyleProperties() noexcept
  {
    if (_cssomWrapper)
    {
      krys_debug_assert(!static_cast<CSSStyleProperties *>(_cssomWrapper.get())->ParentRule());
      krys_debug_assert(!_cssomWrapper->ParentElement());
      return *_cssomWrapper;
    }

    LazyInitialize(_cssomWrapper, CreateUniqueWithoutRefCountedCheck<PropertySetCSSStyleProperties>(*this));
    return *_cssomWrapper;
  }

  size_t StyleProperties::AverageSizeInBytes() noexcept
  {
    // Please update this if the storage scheme changes so that this longer reflects the actual size.
    return ImmutableStyleProperties::ObjectSize(4);
  }

  // See the function above if you need to update this.
  struct SameSizeAsStyleProperties : public RefCounted<SameSizeAsStyleProperties>
  {
    size_t bitfield;
  };
  static_assert(sizeof(StyleProperties) == sizeof(SameSizeAsStyleProperties),
                "style property set should stay small");

#ifndef NDEBUG
  void StyleProperties::ShowStyle() const noexcept
  {
    std::println(stderr, "{}", AsText(DefaultSerialisationContext()));
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
    CSSOMString name;
    name.append(CssName());
    name.append(u8": ");
    name.append(krys::boo::css::SerialiseLonghandValue(context, Id(), *_value));

    if (IsImportant())
    {
      name.append(u8" !important");
    }

    name.append(u8";");
  }
}
