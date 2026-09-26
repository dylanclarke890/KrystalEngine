#include "Krystal.Booey/CSS/StyleProperties.hpp"
#include "Krystal.Booey/CSS/CSSStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Color.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Font.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyInitialValues.hpp"
#include "Krystal.Booey/CSS/ShorthandSerializer.hpp"
#include "Krystal.Booey/CSS/StylePropertiesInlines.hpp"
#include "Krystal.Booey/CSS/StylePropertyShorthand.hpp"
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
  constexpr size_t maxShorthandsForLonghand = 4uz;

  Ref<ImmutableStyleProperties> StyleProperties::ImmutableCopyIfNeeded() const noexcept
  {
    if (_isMutable)
    {
      return Downcast<MutableStyleProperties>(*this).ImmutableDeduplicatedCopy();
    }

    return const_cast<ImmutableStyleProperties &>(Downcast<ImmutableStyleProperties>(*this));
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
        list && list->Separator() == ValueList::CommaSeparator)
    {
      CSSOMString result;
      utf8_stringview separator = u8"";

      for (auto &individualValue : *list)
      {
        result.push_back(std::exchange(separator, u8", "),
                         SerialiseLonghandValue(context, property, individualValue));
      }

      return result;
    }

    return value.IsImplicitInitialValue() ? InitialValueTextForLonghand(property) : value.CssText(context);
  }

  inline CSSOMString StyleProperties::serializeLonghandValue(const CSS::SerializationContext &context,
                                                        PropertyId propertyID) const
  {
    return WebCore::serializeLonghandValue(context, propertyID, getPropertyCSSValue(propertyID).get());
  }

  inline String StyleProperties::serializeShorthandValue(const CSS::SerializationContext &context,
                                                         PropertyId propertyID) const
  {
    return WebCore::serializeShorthandValue(context, *this, propertyID);
  }

  String StyleProperties::getPropertyValue(PropertyId propertyID) const
  {
    return isLonghand(propertyID) ? serializeLonghandValue(CSS::defaultSerializationContext(), propertyID)
                                  : serializeShorthandValue(CSS::defaultSerializationContext(), propertyID);
  }

  std::optional<Color> StyleProperties::propertyAsColor(PropertyId property) const
  {
    auto value = getPropertyCSSValue(property);
    if (!value)
      return std::nullopt;

    if (value->isColor())
      return CSSColorValue::absoluteColor(*value);

    auto serializationString =
      WebCore::serializeLonghandValue(CSS::defaultSerializationContext(), property, *value);
    return CSSPropertyParserHelpers::deprecatedParseColorRawWithoutContext(serializationString);
  }

  std::optional<CSSValueID> StyleProperties::propertyAsValueID(PropertyId property) const
  {
    return longhandValueID(property, getPropertyCSSValue(property).get());
  }

  String StyleProperties::getCustomPropertyValue(const String &propertyName) const
  {
    RefPtr<CSSValue> value = getCustomPropertyCSSValue(propertyName);
    if (value)
      return value->cssText(CSS::defaultSerializationContext());
    return String();
  }

  RefPtr<CSSValue> StyleProperties::getPropertyCSSValue(PropertyId propertyID) const
  {
    int foundPropertyIndex = findPropertyIndex(propertyID);
    if (foundPropertyIndex == -1)
      return nullptr;
    auto property = propertyAt(foundPropertyIndex);
    auto value = property.value();
    // System fonts are represented as CSSPrimitiveValue for various font subproperties, but these must
    // serialize as the empty string. It might be better to implement this as a special CSSValue type instead
    // of turning them into null here.
    if (property.shorthandID() == CSSPropertyFont
        && CSSPropertyParserHelpers::isSystemFontShorthand(valueID(value)))
      return nullptr;
    return value;
  }

  RefPtr<CSSValue> StyleProperties::getCustomPropertyCSSValue(const String &propertyName) const
  {
    int foundPropertyIndex = findCustomPropertyIndex(propertyName);
    if (foundPropertyIndex == -1)
      return nullptr;
    return propertyAt(foundPropertyIndex).value();
  }

  bool StyleProperties::propertyIsImportant(PropertyId propertyID) const
  {
    if (isLonghand(propertyID))
    {
      int foundPropertyIndex = findPropertyIndex(propertyID);
      if (foundPropertyIndex != -1)
        return propertyAt(foundPropertyIndex).isImportant();
      return false;
    }

    ASSERT(shorthandForProperty(propertyID).length());
    for (auto longhand : shorthandForProperty(propertyID))
    {
      if (!propertyIsImportant(longhand))
        return false;
    }
    return true;
  }

  bool StyleProperties::customPropertyIsImportant(const String &propertyName) const
  {
    int foundPropertyIndex = findCustomPropertyIndex(propertyName);
    if (foundPropertyIndex != -1)
      return propertyAt(foundPropertyIndex).isImportant();
    return false;
  }

  String StyleProperties::getPropertyShorthand(PropertyId propertyID) const
  {
    int foundPropertyIndex = findPropertyIndex(propertyID);
    if (foundPropertyIndex == -1)
      return String();
    return nameString(propertyAt(foundPropertyIndex).shorthandID());
  }

  bool StyleProperties::isPropertyImplicit(PropertyId propertyID) const
  {
    int foundPropertyIndex = findPropertyIndex(propertyID);
    if (foundPropertyIndex == -1)
      return false;
    return propertyAt(foundPropertyIndex).isImplicit();
  }

  String StyleProperties::asText(const CSS::SerializationContext &context) const
  {
    return asTextInternal(context).toString();
  }

  AtomString StyleProperties::asTextAtom(const CSS::SerializationContext &context) const
  {
    return asTextInternal(context).toAtomString();
  }

  static constexpr bool canUseShorthandForLonghand(PropertyId shorthandID, PropertyId longhandID)
  {
    ASSERT(isShorthand(shorthandID));
    ASSERT(isLonghand(longhandID));
    switch (shorthandID)
    {
      // We are not yet using the CSSPropertyFont shorthand here because our editing code is currently
      // incompatible.
      case CSSPropertyFont:                    return false;

      // Avoid legacy shorthands according to https://www.w3.org/TR/css-cascade-5/#legacy-shorthand
      case CSSPropertyPageBreakAfter:
      case CSSPropertyPageBreakBefore:
      case CSSPropertyPageBreakInside:
      case CSSPropertyWebkitBackgroundSize:
      case CSSPropertyWebkitBorderRadius:
      case CSSPropertyWebkitColumnBreakAfter:
      case CSSPropertyWebkitColumnBreakBefore:
      case CSSPropertyWebkitColumnBreakInside:
      case CSSPropertyWebkitMaskPosition:
      case CSSPropertyWebkitPerspective:
      case CSSPropertyWebkitTextOrientation:   return false;

      // No other browser currently supports text-decoration-skip, so it's currently more web
      // compatible to avoid collapsing text-decoration-skip-ink, its only longhand.
      case CSSPropertyTextDecorationSkip:      return false;

      // FIXME: -webkit-mask is a legacy shorthand but it's used to serialize -webkit-mask-clip,
      // which should be a legacy shorthand of mask-clip, but it's implemented as a longhand.
      case CSSPropertyWebkitMask:              return longhandID == CSSPropertyWebkitMaskClip;

      // FIXME: more mask nonsense.
      case CSSPropertyMask:
        return longhandID != CSSPropertyMaskComposite && longhandID != CSSPropertyMaskMode
               && longhandID != CSSPropertyMaskSize;

      // FIXME: If font-variant-ligatures is none, this depends on the value of the longhand.
      case CSSPropertyFontVariant:
      // FIXME: These shorthands are avoided for unknown legacy reasons, probably shouldn't be avoided.
      case CSSPropertyColumnRule:
      case CSSPropertyColumns:
      case CSSPropertyContainer:
      case CSSPropertyFontSynthesis:
      case CSSPropertyGridArea:
      case CSSPropertyGridColumn:
      case CSSPropertyGridRow:
      case CSSPropertyMaskPosition:
      case CSSPropertyOffset:
      case CSSPropertyTextEmphasis:
      case CSSPropertyWebkitTextStroke: return false;

      default:                          return true;
    }
  }

  StringBuilder StyleProperties::asTextInternal(const CSS::SerializationContext &context) const
  {
    StringBuilder result;

    constexpr unsigned shorthandPropertyCount = lastShorthandProperty - firstShorthandProperty + 1;
    std::bitset<shorthandPropertyCount> shorthandPropertyUsed;
    std::bitset<shorthandPropertyCount> shorthandPropertyAppeared;

    unsigned numDecls = 0;
    for (auto property : *this)
    {
      auto propertyID = property.id();
      ASSERT(isLonghand(propertyID) || propertyID == CSSPropertyCustom);
      Vector<PropertyId, maxShorthandsForLonghand> shorthands;

      if (auto *substitutionValue = dynamicDowncast<CSSPendingSubstitutionValue>(property.value()))
        shorthands.append(substitutionValue->shorthandPropertyId());
      else
      {
        for (auto &shorthand : matchingShorthandsForLonghand(propertyID))
        {
          if (canUseShorthandForLonghand(shorthand.id(), propertyID))
            shorthands.append(shorthand.id());
        }
      }

      String value;
      bool alreadyUsedShorthand = false;
      for (auto &shorthandPropertyID : shorthands)
      {
        ASSERT(isShorthand(shorthandPropertyID));
        unsigned shorthandPropertyIndex = shorthandPropertyID - firstShorthandProperty;

        ASSERT(shorthandPropertyIndex < shorthandPropertyUsed.size());
        if (shorthandPropertyUsed[shorthandPropertyIndex])
        {
          alreadyUsedShorthand = true;
          break;
        }
        if (shorthandPropertyAppeared[shorthandPropertyIndex])
          continue;
        shorthandPropertyAppeared.set(shorthandPropertyIndex);

        value = serializeShorthandValue(context, shorthandPropertyID);
        if (!value.isNull())
        {
          propertyID = shorthandPropertyID;
          shorthandPropertyUsed.set(shorthandPropertyIndex);
          break;
        }
      }
      if (alreadyUsedShorthand)
        continue;

      if (value.isNull())
        value = WebCore::serializeLonghandValue(context, propertyID, *property.value());

      if (numDecls++)
        result.append(' ');

      if (propertyID == CSSPropertyCustom)
        result.append(downcast<CSSCustomPropertyValue>(*property.value()).name());
      else
        result.append(nameLiteral(propertyID));

      result.append(": "_s, value, property.isImportant() ? " !important"_s : ""_s, ';');
    }

    ASSERT(!numDecls ^ !result.isEmpty());
    return result;
  }

  bool StyleProperties::hasCSSOMWrapper() const
  {
    auto *mutableProperties = dynamicDowncast<MutableStyleProperties>(*this);
    return mutableProperties && mutableProperties->m_cssomWrapper;
  }

  bool StyleProperties::traverseSubresources(
    NOESCAPE const Function<bool(const CachedResource &)> &handler) const
  {
    for (auto property : *this)
    {
      if (property.value()->traverseSubresources(handler))
        return true;
    }
    return false;
  }

  bool StyleProperties::mayDependOnBaseURL() const
  {
    bool result = false;
    Function<IterationStatus(CSSValue &)> func = [&](CSSValue &value) -> IterationStatus
    {
      if (value.mayDependOnBaseURL())
      {
        result = true;
        return IterationStatus::Done;
      }
      return value.visitChildren(func);
    };

    for (auto property : *this)
    {
      if (func(*property.value()) == IterationStatus::Done)
        return result;
    }
    return false;
  }

  bool StyleProperties::propertyMatches(PropertyId propertyID, const CSSValue *propertyValue) const
  {
    int foundPropertyIndex = findPropertyIndex(propertyID);
    if (foundPropertyIndex == -1)
      return false;
    return propertyAt(foundPropertyIndex).value()->equals(*propertyValue);
  }

  Ref<MutableStyleProperties> StyleProperties::mutableCopy() const
  {
    return adoptRef(*new MutableStyleProperties(*this));
  }

  Ref<MutableStyleProperties> StyleProperties::copyProperties(std::span<const PropertyId> properties) const
  {
    auto vector = WTF::compactMap(properties,
                                  [&](auto &property) -> std::optional<CSSProperty>
                                  {
                                    if (auto value = getPropertyCSSValue(property))
                                      return CSSProperty(property, value.releaseNonNull());
                                    return std::nullopt;
                                  });
    return MutableStyleProperties::create(WTF::move(vector));
  }

  CSSStyleProperties *MutableStyleProperties::cssStyleProperties()
  {
    return m_cssomWrapper.get();
  }

  CSSStyleProperties &MutableStyleProperties::ensureCSSStyleProperties()
  {
    if (m_cssomWrapper)
    {
      ASSERT(!static_cast<CSSStyleProperties *>(m_cssomWrapper.get())->parentRule());
      ASSERT(!m_cssomWrapper->parentElement());
      return *m_cssomWrapper;
    }
    lazyInitialize(m_cssomWrapper, makeUniqueWithoutRefCountedCheck<PropertySetCSSStyleProperties>(*this));
    return *m_cssomWrapper;
  }

  unsigned StyleProperties::averageSizeInBytes()
  {
    // Please update this if the storage scheme changes so that this longer reflects the actual size.
    return ImmutableStyleProperties::objectSize(4);
  }

  // See the function above if you need to update this.
  struct SameSizeAsStyleProperties : public RefCounted<SameSizeAsStyleProperties>
  {
    unsigned bitfield;
  };
  static_assert(sizeof(StyleProperties) == sizeof(SameSizeAsStyleProperties),
                "style property set should stay small");

#ifndef NDEBUG
  void StyleProperties::showStyle()
  {
    SAFE_FPRINTF(stderr, "%s\n", asText(CSS::defaultSerializationContext()).ascii());
  }
#endif

  String StyleProperties::PropertyReference::cssName() const
  {
    if (id() == CSSPropertyCustom)
      return downcast<CSSCustomPropertyValue>(*value()).name();
    return nameString(id());
  }

  String StyleProperties::PropertyReference::cssText(const CSS::SerializationContext &context) const
  {
    return makeString(cssName(), ": "_s, WebCore::serializeLonghandValue(context, id(), *m_value),
                      isImportant() ? " !important;"_s : ";"_s);
  }

} // namespace WebCore
