#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
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
#include "Krystal.Core/Types/IterationStatus.hpp"
#include <algorithm>
#include <bitset>

namespace krys::boo::css
{
  namespace
  {
    /// @brief Whether `AsText` may write @p longhand as part of @p shorthand.
    KRYS_NODISCARD constexpr bool CanSerialiseLonghandAsPartOf(PropertyId shorthand,
                                                               PropertyId longhand) noexcept
    {
      krys_debug_assert(IsShorthand(shorthand));
      krys_debug_assert(IsLonghand(longhand));
      switch (shorthand)
      {
        // Editing can't handle the font shorthand yet.
        // NOLINTNEXTLINE(bugprone-branch-clone): the cases are grouped by reason.
        case PropertyId::Font:               return false;

        // Legacy shorthands are for parsing only.
        // https://drafts.csswg.org/css-cascade-5/#legacy-shorthand
        case PropertyId::PageBreakAfter:
        case PropertyId::PageBreakBefore:
        case PropertyId::PageBreakInside:    return false;

        // text-decoration-skip-ink, its only longhand, is better supported on its own.
        case PropertyId::TextDecorationSkip: return false;

        default:                        return true;
      }
    }

    /// @brief The shorthands `AsText` tries for @p property's declaration, preferred first.
    KRYS_NODISCARD SmallList<PropertyId, MaxShorthandsForLonghand>
      ShorthandsToTry(const StyleProperties::PropertyReference &property) noexcept
    {
      SmallList<PropertyId, MaxShorthandsForLonghand> shorthands;
      // A shorthand with a var() keeps its text until substitution, and only it can be written.
      if (const auto *pending = DynamicDowncast<PendingSubstitutionValue>(property.Value()))
      {
        shorthands.push_back(pending->ShorthandPropertyId());
        return shorthands;
      }
      for (const auto &shorthand : MatchingShorthandsForLonghand(property.Id()))
      {
        if (CanSerialiseLonghandAsPartOf(shorthand.Id(), property.Id()))
        {
          shorthands.push_back(shorthand.Id());
        }
      }
      return shorthands;
    }
  }

  CSSOMString SerialiseLonghandValue(const SerialisationContext &context, PropertyId property,
                                     const Value &value) noexcept
  {
    // A comma-separated list from a shorthand (a background's layers, say) can hold implicit initial values.
    // Any property's comma-separated list is handled, as only those lists have them.
    if (const auto *list = DynamicDowncast<ValueList>(value);
        list && list->Separator() == ValueSeparator::Comma)
    {
      CSSOMString result;
      for (const auto &item : *list)
      {
        if (!result.empty())
        {
          result.append(u8", ");
        }
        result.append(SerialiseLonghandValue(context, property, item));
      }
      return result;
    }
    if (value.IsImplicitInitialValue())
    {
      return CSSOMString(InitialValueTextForLonghand(property));
    }
    return value.CssText(context);
  }

  CSSOMString StyleProperties::SerialiseLonghandValue(const SerialisationContext &context,
                                                      PropertyId property) const noexcept
  {
    return css::SerialiseLonghandValue(context, property, GetPropertyCSSValue(property).get());
  }

  Maybe<CSSOMString> StyleProperties::SerialiseShorthandValue(const SerialisationContext &context,
                                                              PropertyId property) const noexcept
  {
    return css::SerialiseShorthandValue(context, *this, property);
  }

  RefPtr<Value> StyleProperties::GetPropertyCSSValue(PropertyId property) const noexcept
  {
    auto index = FindPropertyIndex(property);
    if (!index)
    {
      return nullptr;
    }
    auto declaration = PropertyAt(*index);
    const auto *value = declaration.Value();
    // A system font (`font: menu`) sets its longhands to its keyword, which they mustn't serialise as.
    if (declaration.ShorthandId() == PropertyId::Font && value
        && parse_property::IsSystemFontShorthand(value->ValueId()))
    {
      return nullptr;
    }
    return ShareRefPtr(const_cast<Value *>(value));
  }

  CSSOMString StyleProperties::GetPropertyValue(PropertyId property) const noexcept
  {
    if (IsLonghand(property))
    {
      return SerialiseLonghandValue(DefaultSerialisationContext(), property);
    }
    return SerialiseShorthandValue(DefaultSerialisationContext(), property).value_or(CSSOMString());
  }

  Maybe<boo::Color> StyleProperties::PropertyAsColor(PropertyId property) const noexcept
  {
    auto value = GetPropertyCSSValue(property);
    if (!value)
    {
      return null;
    }
    if (Is<ColorValue>(*value))
    {
      return ColorValue::GetAbsoluteColor(*value);
    }
    // Other colours, such as named ones, are parsed from their text.
    return parse_property::DeprecatedParseColorRawWithoutContext(
      css::SerialiseLonghandValue(DefaultSerialisationContext(), property, *value));
  }

  Maybe<ValueId> StyleProperties::PropertyAsValueId(PropertyId property) const noexcept
  {
    return LonghandValueId(property, GetPropertyCSSValue(property).get());
  }

  bool StyleProperties::PropertyIsImportant(PropertyId property) const noexcept
  {
    if (IsLonghand(property))
    {
      auto index = FindPropertyIndex(property);
      return index && PropertyAt(*index).IsImportant();
    }
    auto shorthand = ShorthandForProperty(property);
    krys_debug_assert(shorthand.Size());
    return std::ranges::all_of(shorthand, [&](PropertyId longhand) { return PropertyIsImportant(longhand); });
  }

  RefPtr<Value> StyleProperties::GetCustomPropertyCSSValue(CSSOMStringView propertyName) const noexcept
  {
    auto index = FindCustomPropertyIndex(propertyName);
    if (!index)
    {
      return nullptr;
    }
    return ShareRefPtr(const_cast<Value *>(PropertyAt(*index).Value()));
  }

  CSSOMString StyleProperties::GetCustomPropertyValue(CSSOMStringView propertyName) const noexcept
  {
    auto value = GetCustomPropertyCSSValue(propertyName);
    return value ? value->CssText(DefaultSerialisationContext()) : CSSOMString();
  }

  bool StyleProperties::CustomPropertyIsImportant(CSSOMStringView propertyName) const noexcept
  {
    auto index = FindCustomPropertyIndex(propertyName);
    return index && PropertyAt(*index).IsImportant();
  }

  Ref<MutableStyleProperties> StyleProperties::MutableCopy() const noexcept
  {
    return AdoptRef(*new MutableStyleProperties(*this));
  }

  Ref<ImmutableStyleProperties> StyleProperties::ImmutableCopyIfNeeded() const noexcept
  {
    if (_isMutable)
    {
      return Downcast<MutableStyleProperties>(*this).ImmutableDeduplicatedCopy();
    }
    return ShareRef(const_cast<ImmutableStyleProperties &>(Downcast<ImmutableStyleProperties>(*this)));
  }

  Ref<MutableStyleProperties>
    StyleProperties::CopyProperties(Span<const PropertyId> properties) const noexcept
  {
    List<Property> copied;
    for (auto property : properties)
    {
      if (auto value = GetPropertyCSSValue(property))
      {
        copied.emplace_back(property, krys::move(value));
      }
    }
    return MutableStyleProperties::Create(krys::move(copied));
  }

  CSSOMString StyleProperties::AsText(const SerialisationContext &context) const noexcept
  {
    constexpr auto shorthandCount =
      static_cast<size_t>(LastShorthandProperty) - static_cast<size_t>(FirstShorthandProperty) + 1uz;
    auto shorthandIndex = [](PropertyId shorthand)
    {
      krys_debug_assert(IsShorthand(shorthand));
      return static_cast<size_t>(shorthand) - static_cast<size_t>(FirstShorthandProperty);
    };
    // Shorthands written so far, whose longhands are then skipped, and those tried, which aren't tried again.
    std::bitset<shorthandCount> written;
    std::bitset<shorthandCount> tried;

    CSSOMString result;
    for (auto property : *this)
    {
      krys_debug_assert(IsLonghand(property.Id()) || property.Id() == PropertyId::Custom);

      auto name = property.Id();
      Maybe<CSSOMString> value;
      bool coveredByWrittenShorthand = false;
      for (auto shorthand : ShorthandsToTry(property))
      {
        auto index = shorthandIndex(shorthand);
        if (written[index])
        {
          coveredByWrittenShorthand = true;
          break;
        }
        if (tried[index])
        {
          continue;
        }
        tried.set(index);
        // Null when the longhands can't be written as this shorthand, such as when some aren't declared.
        value = SerialiseShorthandValue(context, shorthand);
        if (value)
        {
          name = shorthand;
          written.set(index);
          break;
        }
      }
      if (coveredByWrittenShorthand)
      {
        continue;
      }

      if (!result.empty())
      {
        result.push_back(u8' ');
      }
      if (name == PropertyId::Custom)
      {
        result.append(Downcast<CustomPropertyValue>(*property.Value()).Name().View());
      }
      else
      {
        result.append(ToString(name));
      }
      result.append(u8": ");
      result.append(value ? *value : css::SerialiseLonghandValue(context, name, *property.Value()));
      if (property.IsImportant())
      {
        result.append(u8" !important");
      }
      result.push_back(u8';');
    }
    return result;
  }

  CSSOMStringAtom StyleProperties::AsTextAtom(const SerialisationContext &context) const noexcept
  {
    return CSSOMStringAtom(AsText(context));
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

  bool StyleProperties::MayDependOnBaseUrl() const noexcept
  {
    Func<IterationStatus(Value &)> visit = [&](Value &value) -> IterationStatus
    {
      if (value.MayDependOnBaseUrl())
      {
        return IterationStatus::Done;
      }
      return value.VisitChildren(visit);
    };
    for (auto property : *this)
    {
      const auto &value = *property.Value();
      if (value.MayDependOnBaseUrl() || value.VisitChildren(visit) == IterationStatus::Done)
      {
        return true;
      }
    }
    return false;
  }

  bool StyleProperties::PropertyMatches(PropertyId property, const Value *value) const noexcept
  {
    auto index = FindPropertyIndex(property);
    return index && PropertyAt(*index).Value()->Equals(*value);
  }

  size_t StyleProperties::AverageSizeInBytes() noexcept
  {
    // Keep this in step with ImmutableStyleProperties' storage.
    return ImmutableStyleProperties::ObjectSize(4u);
  }

  // A block is a reference count and one word of bit-fields; AverageSizeInBytes assumes as much.
  struct SameSizeAsStyleProperties : public RefCounted<SameSizeAsStyleProperties>
  {
    uint32 BitFields;
  };

  static_assert(sizeof(StyleProperties) == sizeof(SameSizeAsStyleProperties),
                "StyleProperties should stay small");

  CSSOMString StyleProperties::PropertyReference::CssName() const noexcept
  {
    if (Id() == PropertyId::Custom)
    {
      return Downcast<CustomPropertyValue>(*Value()).Name().ToString();
    }
    return CSSOMString(ToString(Id()));
  }
}
