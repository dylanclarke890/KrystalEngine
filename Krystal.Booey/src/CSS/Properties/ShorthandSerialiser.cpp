#include "Krystal.Booey/CSS/Properties/ShorthandSerialiser.hpp"
#include "Krystal.Booey/CSS/Parser/KeywordLookup.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Font.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Grid.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Timeline.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyInitialValues.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyShorthand.hpp"
#include "Krystal.Booey/CSS/Properties/StylePropertiesInlines.hpp"
#include "Krystal.Booey/CSS/Values/BorderImageWidthValue.hpp"
#include "Krystal.Booey/CSS/Values/GlobalKeyword.hpp"
#include "Krystal.Booey/CSS/Values/GridLineNamesValue.hpp"
#include "Krystal.Booey/CSS/Values/GridTemplateAreasValue.hpp"
#include "Krystal.Booey/CSS/Values/PendingSubstitutionValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/Quad.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"
#include "Krystal.Booey/CSS/Values/VariableReferenceValue.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include <algorithm>

namespace krys::boo::css
{
  // FIXME: Generate this from the property definitions.
  constexpr size_t MaxShorthandLength = 18uz;

  namespace
  {
    class ShorthandSerialiser
    {
    private:
      struct Longhand
      {
        PropertyId Property;
        css::Value &Value;
      };

      struct LonghandIteratorBase
      {
        const ShorthandSerialiser &Serialiser;
        size_t Index {0uz};

        void operator++() noexcept
        {
          ++Index;
        }

        KRYS_NODISCARD bool operator==(std::nullptr_t) const noexcept
        {
          return Index >= Serialiser.Length();
        }
      };

      struct LonghandIterator : LonghandIteratorBase
      {
        KRYS_NODISCARD Longhand operator*() const noexcept
        {
          return Serialiser.GetLonghand(Index);
        }
      };

      struct LonghandValueIterator : LonghandIteratorBase
      {
        KRYS_NODISCARD Value &operator*() const noexcept
        {
          return Serialiser.LonghandValue(Index);
        }
      };

      template <typename TIterator>
      struct LonghandRange
      {
        const ShorthandSerialiser &Serialiser;

        KRYS_NODISCARD TIterator begin() const noexcept
        {
          return {{Serialiser}};
        }

        KRYS_NODISCARD constexpr static std::nullptr_t end() noexcept
        {
          return nullptr;
        }

        KRYS_NODISCARD size_t size() const noexcept
        {
          return Serialiser.Length();
        }
      };

      PropertyShorthand _shorthand;
      Array<RefPtr<Value>, MaxShorthandLength> _longhandValues;
      Maybe<CSSOMString> _result;
      bool _commonSerialisationChecksSuppliedResult {false};
      const SerialisationContext &_serialisationContext;

    public:
      ShorthandSerialiser(const SerialisationContext &serialisationContext, const StyleProperties &properties,
                          PropertyId shorthandId) noexcept
          : _shorthand(ShorthandForProperty(shorthandId)), _serialisationContext(serialisationContext)
      {
        _commonSerialisationChecksSuppliedResult = CommonSerialisationChecks(properties);
      }

      KRYS_NODISCARD Maybe<CSSOMString> Serialise() noexcept;

    private:
      KRYS_NODISCARD static bool IsInitialValue(Longhand longhand) noexcept
      {
        return IsInitialValueForLonghand(longhand.Property, longhand.Value);
      }

      KRYS_NODISCARD CSSOMString SerialiseValue(Longhand longhand) const noexcept
      {
        return css::SerialiseLonghandValue(_serialisationContext, longhand.Property, longhand.Value);
      }

      KRYS_NODISCARD size_t Length() const noexcept
      {
        return _shorthand.Size();
      }

      KRYS_NODISCARD Longhand GetLonghand(size_t index) const noexcept
      {
        return {LonghandProperty(index), LonghandValue(index)};
      }

      KRYS_NODISCARD PropertyId LonghandProperty(size_t index) const noexcept
      {
        return _shorthand.Properties()[index];
      }

      KRYS_NODISCARD Value &LonghandValue(size_t index) const noexcept
      {
        return *_longhandValues[index];
      }

      KRYS_NODISCARD size_t LonghandIndex(size_t index, KRYS_MAYBE_UNUSED PropertyId longhand) const noexcept
      {
        krys_debug_assert(LonghandProperty(index) == longhand);
        return index;
      }

      KRYS_NODISCARD LonghandRange<LonghandIterator> Longhands() const noexcept
      {
        return {*this};
      }

      KRYS_NODISCARD LonghandRange<LonghandValueIterator> LonghandValues() const noexcept
      {
        return {*this};
      }

      KRYS_NODISCARD ValueId LonghandValueId(size_t index) const noexcept
      {
        return css::LonghandValueId(LonghandProperty(index), LonghandValue(index));
      }

      KRYS_NODISCARD bool IsLonghandValueId(size_t index, ValueId valueId) const noexcept
      {
        return LonghandValueId(index) == valueId;
      }

      KRYS_NODISCARD bool IsLonghandValueNone(size_t index) const noexcept
      {
        return IsLonghandValueId(index, ValueId::None);
      }

      KRYS_NODISCARD bool IsLonghandInitialValue(size_t index) const noexcept
      {
        return IsInitialValue(GetLonghand(index));
      }

      KRYS_NODISCARD CSSOMString SerialiseLonghandValue(size_t index) const noexcept
      {
        return SerialiseValue(GetLonghand(index));
      }

      KRYS_NODISCARD bool SubsequentLonghandsHaveInitialValues(size_t index) const noexcept;

      KRYS_NODISCARD bool CommonSerialisationChecks(const StyleProperties &properties) noexcept;

      KRYS_NODISCARD CSSOMString SerialiseLonghands() const noexcept;
      KRYS_NODISCARD CSSOMString SerialiseLonghands(size_t lengthLimit,
                                                    CSSOMStringView separator = u8" ") const noexcept;
      KRYS_NODISCARD CSSOMString SerialiseLonghandsOmittingInitialValues() const noexcept;
      KRYS_NODISCARD CSSOMString
        SerialiseLonghandsOmittingTrailingInitialValue(CSSOMStringView separator = u8" ") const noexcept;

      KRYS_NODISCARD Maybe<CSSOMString> SerialiseCommonValue() const noexcept;
      KRYS_NODISCARD Maybe<CSSOMString> SerialiseCommonValue(size_t startIndex, size_t count) const noexcept;
      KRYS_NODISCARD CSSOMString SerialisePair() const noexcept;
      KRYS_NODISCARD CSSOMString SerialiseQuad() const noexcept;

      KRYS_NODISCARD Maybe<CSSOMString> SerialiseLayered() const noexcept;
      KRYS_NODISCARD CSSOMString SerialiseCoordinatingListPropertyGroup() const noexcept;

      KRYS_NODISCARD Maybe<CSSOMString> SerialiseBorder(size_t sectionLength) const noexcept;
      KRYS_NODISCARD Maybe<CSSOMString> SerialiseBorderImage() const noexcept;
      KRYS_NODISCARD CSSOMString SerialiseBorderRadius() const noexcept;
      KRYS_NODISCARD Maybe<CSSOMString> SerialiseBreakInside() const noexcept;
      KRYS_NODISCARD Maybe<CSSOMString> SerialiseFont() const noexcept;
      KRYS_NODISCARD CSSOMString SerialiseFontSynthesis() const noexcept;
      KRYS_NODISCARD Maybe<CSSOMString> SerialiseFontVariant() const noexcept;
      KRYS_NODISCARD Maybe<CSSOMString> SerialiseGrid() const noexcept;
      KRYS_NODISCARD CSSOMString SerialiseGridArea() const noexcept;
      KRYS_NODISCARD CSSOMString SerialiseGridRowColumn() const noexcept;
      KRYS_NODISCARD Maybe<CSSOMString> SerialiseGridTemplate() const noexcept;
      KRYS_NODISCARD CSSOMString SerialiseOffset() const noexcept;
      KRYS_NODISCARD Maybe<CSSOMString> SerialisePageBreak() const noexcept;
      KRYS_NODISCARD CSSOMString SerialisePositionTry() const noexcept;
      KRYS_NODISCARD Maybe<CSSOMString> SerialiseLineClamp() const noexcept;
      KRYS_NODISCARD CSSOMString SerialiseTextBox() const noexcept;
      KRYS_NODISCARD CSSOMString SerialiseTextWrap() const noexcept;
      KRYS_NODISCARD CSSOMString SerialiseWhiteSpace() const noexcept;
      KRYS_NODISCARD Maybe<CSSOMString> SerialiseAnimationRange() const noexcept;
      KRYS_NODISCARD CSSOMString
        SerialiseSingleAnimationRange(const Value &value, SingleAnimationRangeType type,
                                      ValueId startValueId = ValueId::Invalid) const noexcept;
    };

    KRYS_NODISCARD CSSOMString NameString(ValueId valueId) noexcept
    {
      return CSSOMString(ToString(valueId));
    }

    bool ShorthandSerialiser::SubsequentLonghandsHaveInitialValues(size_t startIndex) const noexcept
    {
      for (auto i = startIndex; i < Length(); ++i)
      {
        if (!IsLonghandInitialValue(i))
        {
          return false;
        }
      }

      return true;
    }

    bool ShorthandSerialiser::CommonSerialisationChecks(const StyleProperties &properties) noexcept
    {
      krys_debug_assert(Length());
      krys_debug_assert(Length() <= MaxShorthandLength || _shorthand.Id() == PropertyId::All);

      Maybe<ValueId> specialKeyword;
      bool allSpecialKeywords = true;
      Maybe<bool> importance;
      Maybe<const PendingSubstitutionValue *> firstValueFromShorthand;
      for (auto i = 0uz; i < Length(); ++i)
      {
        auto longhand = LonghandProperty(i);

        auto propertyIndex = properties.FindPropertyIndex(longhand);
        if (!propertyIndex)
        {
          return true;
        }

        auto property = properties.PropertyAt(*propertyIndex);

        // Don't serialize if longhands have different importance.
        bool isImportant = property.IsImportant();
        if (importance.value_or(isImportant) != isImportant)
        {
          return true;
        }

        importance = isImportant;

        auto *value = property.Value();

        // Don't serialize if longhands have different CSS-wide keywords.
        if (!IsGlobalKeyword(GetValueId(*value)) || value->IsImplicitInitialValue())
        {
          if (specialKeyword)
          {
            return true;
          }

          allSpecialKeywords = false;
        }
        else
        {
          if (!allSpecialKeywords)
          {
            return true;
          }

          auto keyword = GetValueId(value);
          if (specialKeyword.value_or(keyword) != keyword)
          {
            return true;
          }

          specialKeyword = keyword;
          continue;
        }

        // Don't serialize if any longhand was set to a variable.
        if (Is<VariableReferenceValue>(*value))
        {
          return true;
        }

        // Don't serialize if any longhand was set by a different shorthand.
        auto *valueFromShorthand = DynamicDowncast<PendingSubstitutionValue>(*value);
        if (valueFromShorthand && valueFromShorthand->ShorthandPropertyId() != _shorthand.Id())
        {
          return true;
        }

        // Don't serialize if longhands are inconsistent about whether they were set by the shorthand.
        if (!firstValueFromShorthand)
        {
          firstValueFromShorthand = valueFromShorthand;
        }
        else
        {
          bool wasSetByShorthand = valueFromShorthand;
          bool firstWasSetByShorthand = *firstValueFromShorthand;
          if (firstWasSetByShorthand != wasSetByShorthand)
          {
            return true;
          }
        }

        if (_shorthand.Id() != PropertyId::All)
        {
          _longhandValues[i] = ShareRefPtr(const_cast<Value *>(value));
        }
      }

      if (specialKeyword)
      {
        _result = NameString(*specialKeyword);
        return true;
      }

      if (*firstValueFromShorthand)
      {
        _result = (*firstValueFromShorthand)->ShorthandValue().CssText(_serialisationContext);
        return true;
      }

      return false;
    }

    Maybe<CSSOMString> ShorthandSerialiser::Serialise() noexcept
    {
      if (_commonSerialisationChecksSuppliedResult)
      {
        return krys::move(_result);
      }

      switch (_shorthand.Id())
      {
        case PropertyId::All:                  return null;
        case PropertyId::Animation:
        case PropertyId::Background:
        case PropertyId::BackgroundPosition:
        case PropertyId::Mask:
        case PropertyId::MaskPosition:
        case PropertyId::Transition:
        case PropertyId::WebkitMaskPosition:   return SerialiseLayered();
        case PropertyId::Border:               return SerialiseBorder(4);
        case PropertyId::BorderBlock:
        case PropertyId::BorderInline:         return SerialiseBorder(2);
        case PropertyId::BorderBlockColor:
        case PropertyId::BorderBlockStyle:
        case PropertyId::BorderBlockWidth:
        case PropertyId::BorderInlineColor:
        case PropertyId::BorderInlineStyle:
        case PropertyId::BorderInlineWidth:
        case PropertyId::BorderSpacing:
        case PropertyId::ContainIntrinsicSize:
        case PropertyId::Gap:
        case PropertyId::InsetBlock:
        case PropertyId::InsetInline:
        case PropertyId::MarginBlock:
        case PropertyId::MarginInline:
        case PropertyId::Overflow:
        case PropertyId::OverscrollBehavior:
        case PropertyId::PaddingBlock:
        case PropertyId::PaddingInline:
        case PropertyId::PlaceContent:
        case PropertyId::PlaceItems:
        case PropertyId::PlaceSelf:
        case PropertyId::ScrollMarginBlock:
        case PropertyId::ScrollMarginInline:
        case PropertyId::ScrollPaddingBlock:
        case PropertyId::ScrollPaddingInline:  return SerialisePair();
        case PropertyId::BlockStep:
        case PropertyId::BorderBlockEnd:
        case PropertyId::BorderBlockStart:
        case PropertyId::BorderBottom:
        case PropertyId::BorderInlineEnd:
        case PropertyId::BorderInlineStart:
        case PropertyId::BorderLeft:
        case PropertyId::BorderRight:
        case PropertyId::BorderTop:
        case PropertyId::ColumnRule:
        case PropertyId::Columns:
        case PropertyId::FlexFlow:
        case PropertyId::ListStyle:
        case PropertyId::Outline:
        case PropertyId::TextEmphasis:
        case PropertyId::TextDecoration:       return SerialiseLonghandsOmittingInitialValues();
        case PropertyId::BorderColor:
        case PropertyId::BorderStyle:
        case PropertyId::BorderWidth:
        case PropertyId::CornerShape:
        case PropertyId::Inset:
        case PropertyId::Margin:
        case PropertyId::Padding:
        case PropertyId::ScrollMargin:
        case PropertyId::ScrollPadding:        return SerialiseQuad();
        case PropertyId::BorderImage:
        case PropertyId::MaskBorder:           return SerialiseBorderImage();
        case PropertyId::BorderRadius:         return SerialiseBorderRadius();
        case PropertyId::Container:            return SerialiseLonghandsOmittingTrailingInitialValue(u8" / ");
        case PropertyId::Flex:
        case PropertyId::PerspectiveOrigin:    return SerialiseLonghands();
        case PropertyId::Font:                 return SerialiseFont();
        case PropertyId::FontVariant:          return SerialiseFontVariant();
        case PropertyId::FontSynthesis:        return SerialiseFontSynthesis();
        case PropertyId::Grid:                 return SerialiseGrid();
        case PropertyId::GridArea:             return SerialiseGridArea();
        case PropertyId::GridColumn:
        case PropertyId::GridRow:              return SerialiseGridRowColumn();
        case PropertyId::GridTemplate:         return SerialiseGridTemplate();
        case PropertyId::LineClamp:            return SerialiseLineClamp();
        case PropertyId::Marker:               return SerialiseCommonValue();
        case PropertyId::Offset:               return SerialiseOffset();
        case PropertyId::PageBreakAfter:
        case PropertyId::PageBreakBefore:      return SerialisePageBreak();
        case PropertyId::PageBreakInside:      return SerialiseBreakInside();
        case PropertyId::PositionTry:          return SerialisePositionTry();
        case PropertyId::TextDecorationSkip:
        case PropertyId::WebkitBackgroundSize: return SerialiseLonghandValue(0);
        case PropertyId::TransformOrigin:      return SerialiseLonghandsOmittingTrailingInitialValue();
        case PropertyId::TextWrap:             return SerialiseTextWrap();
        case PropertyId::TextBox:              return SerialiseTextBox();
        case PropertyId::WhiteSpace:           return SerialiseWhiteSpace();
        case PropertyId::ScrollTimeline:
        case PropertyId::ViewTimeline:         return SerialiseCoordinatingListPropertyGroup();
        case PropertyId::AnimationRange:       return SerialiseAnimationRange();
        default:
        {
          krys_debug_assert_msg(false, "unexpected shorthand");
          return null;
        }
      }
    }

    CSSOMString ShorthandSerialiser::SerialiseLonghands() const noexcept
    {
      return SerialiseLonghands(Length());
    }

    CSSOMString ShorthandSerialiser::SerialiseLonghands(size_t lengthLimit,
                                                        CSSOMStringView separator) const noexcept
    {
      krys_debug_assert(lengthLimit <= Length());

      CSSOMString result;
      CSSOMStringView prefix = u8"";
      for (auto i = 0uz; i < lengthLimit; ++i)
      {
        result.append(std::exchange(prefix, separator));
        result.append(SerialiseLonghandValue(i));
      }

      return result;
    }

    CSSOMString ShorthandSerialiser::SerialiseLonghandsOmittingInitialValues() const noexcept
    {
      CSSOMString result;
      CSSOMStringView prefix = u8"";
      for (auto longhand : Longhands())
      {
        if (!IsInitialValue(longhand))
        {
          result.append(std::exchange(prefix, u8" "));
          result.append(SerialiseValue(longhand));
        }
      }

      return result.empty() ? SerialiseLonghandValue(0) : result;
    }

    CSSOMString ShorthandSerialiser::SerialiseLonghandsOmittingTrailingInitialValue(
      CSSOMStringView separator) const noexcept
    {
      krys_debug_assert(Length() >= 2);
      return SerialiseLonghands(Length() - IsLonghandInitialValue(Length() - 1), separator);
    }

    Maybe<CSSOMString> ShorthandSerialiser::SerialiseCommonValue() const noexcept
    {
      return SerialiseCommonValue(0, Length());
    }

    Maybe<CSSOMString> ShorthandSerialiser::SerialiseCommonValue(size_t startIndex,
                                                                 size_t count) const noexcept
    {
      Maybe<CSSOMString> result;
      for (auto i = 0uz; i < count; ++i)
      {
        auto text = SerialiseLonghandValue(startIndex + i);
        if (!result)
        {
          result = krys::move(text);
        }
        else if (*result != text)
        {
          return null;
        }
      }

      return result;
    }

    CSSOMString ShorthandSerialiser::SerialisePair() const noexcept
    {
      krys_debug_assert(Length() == 2);
      auto first = SerialiseLonghandValue(0);
      auto second = SerialiseLonghandValue(1);
      if (first != second)
      {
        return first + u8' ' + second;
      }

      return first;
    }

    CSSOMString ShorthandSerialiser::SerialiseQuad() const noexcept
    {
      krys_debug_assert(Length() == 4);
      return Quad::Serialise(SerialiseLonghandValue(0), SerialiseLonghandValue(1), SerialiseLonghandValue(2),
                             SerialiseLonghandValue(3));
    }

    class LayerValues
    {
    private:
      const PropertyShorthand &_shorthand;
      Array<bool, MaxShorthandLength> _skipSerialising {};
      Array<RefPtr<const Value>, MaxShorthandLength> _values;

    public:
      explicit LayerValues(const PropertyShorthand &shorthand) noexcept : _shorthand(shorthand)
      {
        krys_debug_assert(_shorthand.Size() <= MaxShorthandLength);
      }

      void Set(size_t index, const Value *value, bool skipSerialising = false) noexcept
      {
        krys_debug_assert(index < _shorthand.Size());
        _skipSerialising[index] =
          skipSerialising || !value || IsInitialValueForLonghand(_shorthand.Properties()[index], *value);
        _values[index] = ShareRefPtr(value);
      }

      KRYS_NODISCARD bool &Skip(size_t index) noexcept
      {
        krys_debug_assert(index < _shorthand.Size());
        return _skipSerialising[index];
      }

      KRYS_NODISCARD Maybe<ValueId> GetValueId(size_t index) const noexcept
      {
        return LonghandValueId(_shorthand.Properties()[index], _values[index].get());
      }

      KRYS_NODISCARD ValueId ValueIdIncludingCustomIdent(size_t index) const noexcept
      {
        auto *value = DynamicDowncast<PrimitiveValue>(_values[index].get());
        if (value && value->IsCustomIdent())
        {
          return ValueIdFromName(value->StringValue());
        }

        return GetValueId(index).value_or(ValueId::Invalid);
      }

      KRYS_NODISCARD bool EqualValueIds(size_t indexA, size_t indexB) const noexcept
      {
        auto valueA = GetValueId(indexA);
        auto valueB = GetValueId(indexB);
        return valueA && valueB && *valueA == *valueB;
      }

      KRYS_NODISCARD bool IsValueId(size_t index) const noexcept
      {
        auto result = GetValueId(index);
        return result && *result != ValueId::Invalid;
      }

      KRYS_NODISCARD bool IsPair(size_t index) const noexcept
      {
        // This returns false for implicit initial values that are pairs, which is OK for now.
        krys_debug_assert(index < _shorthand.Size());
        auto *value = _values[index].get();
        return value && value->IsValuePairValue();
      }

      void Serialise(CSSOMString &builder, const SerialisationContext &context) const noexcept
      {
        // If all are skipped, then serialize the first.
        auto range = Span<const bool>(_skipSerialising.data(), _shorthand.Size());
        bool allSkipped = std::ranges::find(range, false) == range.end();

        CSSOMStringView separator = builder.empty() ? u8"" : u8", ";
        auto longhands = _shorthand.Properties();
        for (auto j = 0uz; j < longhands.size(); ++j)
        {
          auto longhand = longhands[j];
          if (allSkipped ? j : _skipSerialising[j])
          {
            continue;
          }

          if (longhand == PropertyId::BackgroundSize || longhand == PropertyId::MaskSize)
          {
            separator = u8" / ";
          }

          builder.append(separator);
          if (auto &value = _values[j])
          {
            builder.append(SerialiseLonghandValue(context, longhand, *value));
          }
          else
          {
            builder.append(InitialValueTextForLonghand(longhand));
          }

          separator = u8" ";
        }
      }
    };

    CSSOMString ShorthandSerialiser::SerialiseCoordinatingListPropertyGroup() const noexcept
    {
      krys_debug_assert(Length() > 1);

      // https://drafts.csswg.org/css-values-4/#linked-properties

      // First, figure out the number of items in the coordinating list base property,
      // which we'll need to match for all coordinated longhands, thus possibly trimming
      // or expanding.
      uint32 numberOfItemsForCoordinatingListBaseProperty = 1u;
      if (auto *valueList = DynamicDowncast<ValueList>(LonghandValue(0)))
      {
        numberOfItemsForCoordinatingListBaseProperty =
          std::max(valueList->Length(), numberOfItemsForCoordinatingListBaseProperty);
      }

      // Now go through all longhands and ensure we repeat items earlier in the list
      // should there not be a specified value.
      CSSOMString result;
      for (auto listItemIndex = 0u; listItemIndex < numberOfItemsForCoordinatingListBaseProperty;
           ++listItemIndex)
      {
        LayerValues layerValues {_shorthand};
        for (auto longhandIndex = 0uz; longhandIndex < Length(); ++longhandIndex)
        {
          auto &value = LonghandValue(longhandIndex);
          if (auto *valueList = DynamicDowncast<ValueList>(value))
          {
            auto *valueInList = [&]() -> const Value *
            {
              if (auto *specifiedValue = valueList->Item(listItemIndex))
              {
                return specifiedValue;
              }

              if (auto numberOfItemsInList = valueList->size())
              {
                return valueList->Item(listItemIndex % numberOfItemsInList);
              }

              return nullptr;
            }();
            layerValues.Set(longhandIndex, valueInList);
          }
          else
          {
            layerValues.Set(longhandIndex, &value);
          }
        }

        // The coordinating list base property must never be skipped.
        layerValues.Skip(0) = false;
        layerValues.Serialise(result, _serialisationContext);
      }

      return result;
    }

    Maybe<CSSOMString> ShorthandSerialiser::SerialiseLayered() const noexcept
    {
      uint32 numLayers = 1u;
      for (auto &value : LonghandValues())
      {
        if (auto *valueList = DynamicDowncast<ValueList>(value))
        {
          numLayers = std::max(valueList->Length(), numLayers);
        }
      }

      CSSOMString result;
      for (auto i = 0u; i < numLayers; i++)
      {
        LayerValues layerValues {_shorthand};

        for (auto j = 0uz; j < Length(); j++)
        {
          auto &value = LonghandValue(j);
          if (auto *valueList = DynamicDowncast<ValueList>(value))
          {
            layerValues.Set(j, valueList->Item(i));
          }
          else
          {
            // Color is only in the last layer. Other singletons are only in the first.
            auto singletonLayer = LonghandProperty(j) == PropertyId::BackgroundColor ? numLayers - 1 : 0;
            layerValues.Set(j, &value, i != singletonLayer);
          }
        }

        bool hasUnskippedValue = false;
        for (auto j = 0uz; j < Length(); j++)
        {
          auto longhand = LonghandProperty(j);

          // A single box value sets both background-origin and background-clip.
          // A single geometry-box value sets both mask-origin and mask-clip.
          if (longhand == PropertyId::BackgroundClip || longhand == PropertyId::MaskClip)
          {
            // The previous property is origin.
            krys_debug_assert(j >= 1);
            krys_debug_assert(LonghandProperty(j - 1) == PropertyId::BackgroundOrigin
                              || LonghandProperty(j - 1) == PropertyId::MaskOrigin);
            if (layerValues.EqualValueIds(j - 1, j))
            {
              // If the two are the same, one value sets both.
              if (!layerValues.Skip(j - 1) && !layerValues.Skip(j))
              {
                layerValues.Skip(j) = true;
              }
            }
            else if (!layerValues.Skip(j - 1) || !layerValues.Skip(j))
            {
              // If the two are different, both need to be serialized, unless clip is invalid as origin
              if (layerValues.GetValueId(j) == ValueId::NoClip)
              {
                continue;
              }

              if (layerValues.GetValueId(j) == ValueId::BorderArea)
              {
                layerValues.Skip(j - 1) = layerValues.GetValueId(j - 1) == ValueId::BorderBox;
                continue;
              }

              layerValues.Skip(j - 1) = false;
              layerValues.Skip(j) = false;
            }
          }

          // A single background-position value (identifier or numeric) sets the other value to center.
          // A single mask-position value (identifier or numeric) sets the other value to center.
          // Order matters when one is numeric, but not when both are identifiers.
          if (longhand == PropertyId::BackgroundPositionY || longhand == PropertyId::WebkitMaskPositionY)
          {
            // The previous property is X.
            krys_debug_assert(j >= 1);
            krys_debug_assert(LonghandProperty(j - 1) == PropertyId::BackgroundPositionX
                              || LonghandProperty(j - 1) == PropertyId::WebkitMaskPositionX);
            if (Length() == 2)
            {
              krys_debug_assert(j == 1);
              layerValues.Skip(0) = false;
              layerValues.Skip(1) = false;
            }
            else
            {
              // Always serialize positions to at least 2 values.
              // https://drafts.csswg.org/css-values-4/#position-serialization
              if (!layerValues.Skip(j - 1))
              {
                layerValues.Skip(j) = false;
              }
            }
          }

          // If we get to "animation-timeline" and yet haven't encountered an unskipped value,
          // this means that this "animation" shorthand only has initial values for the non
          // reset-only longhands and so we cannot serialize it. We only do this when we deal
          // with multiple layers since the single "none" case would be caught otherwise.
          if (numLayers > 1 && longhand == PropertyId::AnimationTimeline && !hasUnskippedValue)
          {
            return null;
          }

          if (layerValues.Skip(j))
          {
            continue;
          }

          hasUnskippedValue = true;

          // If we encounter one of the reset-only "animation" longhands and the value was not skipped,
          // then it was set to a non-initial value and we cannot serialize it.
          if (longhand == PropertyId::AnimationTimeline || longhand == PropertyId::AnimationRangeStart
              || longhand == PropertyId::AnimationRangeEnd)
          {
            return null;
          }

          // The syntax for background-size means that if it is present, background-position must be too.
          // The syntax for mask-size means that if it is present, mask-position must be too.
          if (longhand == PropertyId::BackgroundSize || longhand == PropertyId::MaskSize)
          {
            // The previous properties are X and Y.
            krys_debug_assert(j >= 2);
            krys_debug_assert(LonghandProperty(j - 2) == PropertyId::BackgroundPositionX
                              || LonghandProperty(j - 2) == PropertyId::WebkitMaskPositionX);
            krys_debug_assert(LonghandProperty(j - 1) == PropertyId::BackgroundPositionY
                              || LonghandProperty(j - 1) == PropertyId::WebkitMaskPositionY);
            layerValues.Skip(j - 2) = false;
            layerValues.Skip(j - 1) = false;
          }

          // The first value in each animation shorthand that can be parsed as a time is assigned to
          // animation-duration, and the second is assigned to animation-delay, so we must serialize
          // both if we are serializing animation-delay.
          if (longhand == PropertyId::AnimationDelay)
          {
            layerValues.Skip(LonghandIndex(0, PropertyId::AnimationDuration)) = false;
          }
        }

        // In the animation shorthand, if the value of animation-name could be parsed as one of
        // the other longhands that longhand must be serialized to avoid ambiguity.
        if (_shorthand.Id() == PropertyId::Animation)
        {
          auto animationTimingFunctionIndex = LonghandIndex(1, PropertyId::AnimationTimingFunction);
          auto animationIterationCountIndex = LonghandIndex(3, PropertyId::AnimationIterationCount);
          auto animationDirectionIndex = LonghandIndex(4, PropertyId::AnimationDirection);
          auto animationFillModeIndex = LonghandIndex(5, PropertyId::AnimationFillMode);
          auto animationPlayStateIndex = LonghandIndex(6, PropertyId::AnimationPlayState);
          auto animationNameIndex = LonghandIndex(7, PropertyId::AnimationName);
          if (!layerValues.Skip(animationNameIndex))
          {
            switch (layerValues.ValueIdIncludingCustomIdent(animationNameIndex))
            {
              case ValueId::Alternate:
              case ValueId::AlternateReverse:
              case ValueId::Normal:
              case ValueId::Reverse:
              {
                layerValues.Skip(animationDirectionIndex) = false;
                break;
              }
              case ValueId::Backwards:
              case ValueId::Both:
              case ValueId::Forwards:
              {
                layerValues.Skip(animationFillModeIndex) = false;
                break;
              }
              case ValueId::Ease:
              case ValueId::EaseIn:
              case ValueId::EaseInOut:
              case ValueId::EaseOut:
              case ValueId::Linear:
              case ValueId::StepEnd:
              case ValueId::StepStart:
              {
                layerValues.Skip(animationTimingFunctionIndex) = false;
                break;
              }
              case ValueId::Infinite:
              {
                layerValues.Skip(animationIterationCountIndex) = false;
                break;
              }
              case ValueId::Paused:
              case ValueId::Running:
              {
                layerValues.Skip(animationPlayStateIndex) = false;
                break;
              }
              default: break;
            }
          }
        }

        layerValues.Serialise(result, _serialisationContext);
      }

      return result;
    }

    Maybe<CSSOMString> ShorthandSerialiser::SerialiseBorder(size_t sectionLength) const noexcept
    {
      krys_debug_assert(3 * sectionLength <= Length());

      bool mustSerialiseAsEmptyString = false;
      auto serialiseSection = [&](size_t index, ValueId defaultValue) -> Maybe<CSSOMString>
      {
        auto value = SerialiseCommonValue(index * sectionLength, sectionLength);
        if (!value)
        {
          mustSerialiseAsEmptyString = true;
        }
        else if (*value == ToString(defaultValue))
        {
          value = null;
        }

        return value;
      };
      auto width = serialiseSection(0, ValueId::Medium);       // widths
      auto style = serialiseSection(1, ValueId::None);         // styles
      auto color = serialiseSection(2, ValueId::Currentcolor); // colors
      if (mustSerialiseAsEmptyString || !SubsequentLonghandsHaveInitialValues(3 * sectionLength))
      {
        return null;
      }

      auto bits = (static_cast<int>(width.has_value()) << 2) | (static_cast<int>(style.has_value()) << 1)
                  | static_cast<int>(color.has_value());
      switch (bits)
      {
        case 0b000: return NameString(ValueId::Medium);
        case 0b001: return *color;
        case 0b010: return *style;
        case 0b011: return *style + u8' ' + *color;
        case 0b100: return *width;
        case 0b101: return *width + u8' ' + *color;
        case 0b110: return *width + u8' ' + *style;
        case 0b111: return *width + u8' ' + *style + u8' ' + *color;
        default:    break;
      }

      krys_unreachable();
    }

    Maybe<CSSOMString> ShorthandSerialiser::SerialiseBorderImage() const noexcept
    {
      krys_debug_assert(Length() == 5);
      CSSOMString result;
      bool omittedSlice = false;
      bool omittedWidth = false;
      CSSOMStringView separator = u8"";
      for (auto longhand : Longhands())
      {
        if (IsInitialValue(longhand))
        {
          if (longhand.Property == PropertyId::BorderImageSlice
              || longhand.Property == PropertyId::MaskBorderSlice)
          {
            omittedSlice = true;
          }
          else if (longhand.Property == PropertyId::BorderImageWidth
                   || longhand.Property == PropertyId::MaskBorderWidth)
          {
            omittedWidth = true;
          }

          continue;
        }

        if (omittedSlice
            && (longhand.Property == PropertyId::BorderImageWidth
                || longhand.Property == PropertyId::BorderImageOutset
                || longhand.Property == PropertyId::MaskBorderWidth
                || longhand.Property == PropertyId::MaskBorderOutset))
        {
          return null;
        }

        CSSOMString valueText;

        // Only the legacy border-image shorthand lets fixed border slices also set the border widths.
        if (auto *width = DynamicDowncast<BorderImageWidthValue>(longhand.Value))
        {
          if (width->OverridesBorderWidths())
          {
            return null;
          }

          valueText = width->Widths().CssText(_serialisationContext);
        }
        else
        {
          valueText = SerialiseValue(longhand);
        }

        // Append separator and text.
        if (longhand.Property == PropertyId::BorderImageWidth
            || longhand.Property == PropertyId::MaskBorderWidth)
        {
          separator = u8" / ";
        }
        else if (longhand.Property == PropertyId::BorderImageOutset
                 || longhand.Property == PropertyId::MaskBorderOutset)
        {
          separator = omittedWidth ? u8" / / " : u8" / ";
        }

        result.append(separator);
        result.append(valueText);
        separator = u8" ";
      }

      if (result.empty())
      {
        return NameString(ValueId::None);
      }

      return result;
    }

    CSSOMString ShorthandSerialiser::SerialiseBorderRadius() const noexcept
    {
      krys_debug_assert(Length() == 4);
      Array<const Value *, 4> horizontalRadii;
      Array<const Value *, 4> verticalRadii;
      for (auto i = 0uz; i < 4uz; ++i)
      {
        auto &value = LonghandValue(i);
        horizontalRadii[i] = &value.First();
        verticalRadii[i] = &value.Second();
      }

      bool serialiseBoth = false;
      for (auto i = 0uz; i < 4uz; ++i)
      {
        if (!horizontalRadii[i]->Equals(*verticalRadii[i]))
        {
          serialiseBoth = true;
          break;
        }
      }

      CSSOMString result;
      auto serialiseRadii = [&](const Array<const Value *, 4> &r)
      {
        auto count = 1uz;
        if (!r[3]->Equals(*r[1]))
        {
          count = 4uz;
        }
        else if (!r[2]->Equals(*r[0]))
        {
          count = 3uz;
        }
        else if (!r[1]->Equals(*r[0]))
        {
          count = 2uz;
        }

        for (auto i = 0uz; i < count; ++i)
        {
          if (i)
          {
            result.push_back(u8' ');
          }

          result.append(r[i]->CssText(_serialisationContext));
        }
      };

      serialiseRadii(horizontalRadii);
      if (serialiseBoth)
      {
        result.append(u8" / ");
        serialiseRadii(verticalRadii);
      }

      return result;
    }

    Maybe<CSSOMString> ShorthandSerialiser::SerialiseBreakInside() const noexcept
    {
      auto keyword = LonghandValueId(0);
      switch (keyword)
      {
        case ValueId::Auto:
        case ValueId::Avoid: return NameString(keyword);
        default:             return null;
      }
    }

    /// @brief The font-width keyword for `value`, a percentage.
    /// @note Matches only exact keyword percentages: a value that a fixed-point font selection value can't
    /// hold exactly isn't converted to a keyword even if it rounds to one.
    KRYS_NODISCARD Maybe<ValueId> FontWidthKeyword(double value) noexcept
    {
      if (value == 50.0)
      {
        return ValueId::UltraCondensed;
      }
      if (value == 62.5)
      {
        return ValueId::ExtraCondensed;
      }
      if (value == 75.0)
      {
        return ValueId::Condensed;
      }
      if (value == 87.5)
      {
        return ValueId::SemiCondensed;
      }
      if (value == 100.0)
      {
        return ValueId::Normal;
      }
      if (value == 112.5)
      {
        return ValueId::SemiExpanded;
      }
      if (value == 125.0)
      {
        return ValueId::Expanded;
      }
      if (value == 150.0)
      {
        return ValueId::ExtraExpanded;
      }
      if (value == 200.0)
      {
        return ValueId::UltraExpanded;
      }

      return null;
    }

    Maybe<CSSOMString> ShorthandSerialiser::SerialiseFont() const noexcept
    {
      // If all properties are set to the same system font shorthand, serialize as that.
      // If some but not all properties are, the font shorthand can't represent that, serialize as empty
      // string.
      Maybe<ValueId> specialKeyword;
      bool allSpecialKeywords = true;
      for (auto &longhandValue : LonghandValues())
      {
        auto keyword = GetValueId(longhandValue);
        if (!PropertyParserHelpers::IsSystemFontShorthand(keyword))
        {
          allSpecialKeywords = false;
        }
        else
        {
          if (specialKeyword.value_or(keyword) != keyword)
          {
            return null;
          }

          specialKeyword = keyword;
        }
      }

      if (specialKeyword)
      {
        return allSpecialKeywords ? Maybe<CSSOMString>(NameString(*specialKeyword)) : null;
      }

      auto styleIndex = LonghandIndex(0, PropertyId::FontStyle);
      auto capsIndex = LonghandIndex(1, PropertyId::FontVariantCaps);
      auto weightIndex = LonghandIndex(2, PropertyId::FontWeight);
      auto widthIndex = LonghandIndex(3, PropertyId::FontWidth);
      auto sizeIndex = LonghandIndex(4, PropertyId::FontSize);
      auto lineHeightIndex = LonghandIndex(5, PropertyId::LineHeight);
      auto familyIndex = LonghandIndex(6, PropertyId::FontFamily);

      // Properties after font-family are reset but not represented by the shorthand.
      // If any is not the initial value, serialize as empty string.
      if (!SubsequentLonghandsHaveInitialValues(familyIndex + 1))
      {
        return null;
      }

      // Only two values of variant-caps can be serialized in the font shorthand.
      // If the value is anything else, serialize as empty string.
      auto capsKeyword = LonghandValueId(capsIndex);
      if (capsKeyword != ValueId::Normal && capsKeyword != ValueId::SmallCaps)
      {
        return null;
      }

      // Font width values can only be serialized in the font shorthand as keywords, since percentages are
      // also valid font sizes. If a font width percentage can be expressed as a keyword, then do that.
      auto widthKeyword = LonghandValueId(widthIndex);
      if (widthKeyword == ValueId::Invalid)
      {
        auto &widthValue = Downcast<PrimitiveValue>(LonghandValue(widthIndex));
        if (widthValue.IsCalculated() || !widthValue.IsPercentage())
        {
          return null;
        }

        auto keyword = FontWidthKeyword(widthValue.ResolveAsPercentageNoConversionDataRequired<double>());
        if (!keyword)
        {
          return null;
        }

        widthKeyword = *keyword;
      }

      bool includeStyle = !IsLonghandInitialValue(styleIndex);
      bool includeCaps = capsKeyword != ValueId::Normal;
      bool includeWeight = !IsLonghandInitialValue(weightIndex);
      bool includeWidth = widthKeyword != ValueId::Normal;
      bool includeLineHeight = !IsLonghandInitialValue(lineHeightIndex);

      CSSOMString result;
      if (includeStyle)
      {
        result.append(SerialiseLonghandValue(styleIndex));
      }

      if (includeCaps)
      {
        result.append(includeStyle ? u8" " : u8"");
        result.append(ToString(capsKeyword));
      }

      if (includeWeight)
      {
        result.append(includeStyle || includeCaps ? u8" " : u8"");
        result.append(SerialiseLonghandValue(weightIndex));
      }

      if (includeWidth)
      {
        result.append(includeStyle || includeCaps || includeWeight ? u8" " : u8"");
        result.append(ToString(widthKeyword));
      }

      result.append(includeStyle || includeCaps || includeWeight || includeWidth ? u8" " : u8"");
      result.append(SerialiseLonghandValue(sizeIndex));
      if (includeLineHeight)
      {
        result.append(u8" / ");
        result.append(SerialiseLonghandValue(lineHeightIndex));
      }

      result.push_back(u8' ');
      result.append(SerialiseLonghandValue(familyIndex));
      return result;
    }

    CSSOMString ShorthandSerialiser::SerialiseFontSynthesis() const noexcept
    {
      // font-synthesis: none | [ weight || style || small-caps ]
      krys_debug_assert(Length() == 3);

      auto bits =
        static_cast<int>(!IsLonghandValueNone(LonghandIndex(0, PropertyId::FontSynthesisWeight))) << 2
        | static_cast<int>(!IsLonghandValueNone(LonghandIndex(1, PropertyId::FontSynthesisStyle))) << 1
        | static_cast<int>(!IsLonghandValueNone(LonghandIndex(2, PropertyId::FontSynthesisSmallCaps)));

      switch (bits)
      {
        case 0b000: return NameString(ValueId::None);
        case 0b001: return NameString(ValueId::SmallCaps);
        case 0b010: return NameString(ValueId::Style);
        case 0b011: return u8"style small-caps";
        case 0b100: return NameString(ValueId::Weight);
        case 0b101: return u8"weight small-caps";
        case 0b110: return u8"weight style";
        case 0b111: return u8"weight style small-caps";
        default:    break;
      }

      krys_unreachable();
    }

    Maybe<CSSOMString> ShorthandSerialiser::SerialiseFontVariant() const noexcept
    {
      for (auto &value : LonghandValues())
      {
        if (PropertyParserHelpers::IsSystemFontShorthand(GetValueId(value)))
        {
          return null;
        }
      }

      if (IsLonghandValueNone(LonghandIndex(0, PropertyId::FontVariantLigatures)))
      {
        for (auto longhand : Longhands())
        {
          // font-variant cannot represent "font-variant-ligatures: none" along with any other non-normal
          // longhands.
          if (longhand.Property != PropertyId::FontVariantLigatures && !IsInitialValue(longhand))
          {
            return null;
          }
        }
      }

      return SerialiseLonghandsOmittingInitialValues();
    }

    KRYS_NODISCARD bool IsValueIdIncludingList(const Value &value, ValueId id) noexcept
    {
      if (auto *valueList = DynamicDowncast<ValueList>(value))
      {
        if (valueList->size() != 1)
        {
          return false;
        }

        auto *item = valueList->Item(0);
        return item && IsValueId(*item, id);
      }

      return IsValueId(value, id);
    }

    KRYS_NODISCARD bool GridAutoFlowContains(const Value &autoFlow, ValueId id) noexcept
    {
      if (auto *valueList = DynamicDowncast<ValueList>(autoFlow))
      {
        for (auto &currentValue : *valueList)
        {
          if (IsValueId(currentValue, id))
          {
            return true;
          }
        }

        return false;
      }

      return IsValueId(autoFlow, id);
    }

    Maybe<CSSOMString> ShorthandSerialiser::SerialiseGrid() const noexcept
    {
      krys_debug_assert(Length() == 6);

      auto rowsIndex = LonghandIndex(0, PropertyId::GridTemplateRows);
      auto columnsIndex = LonghandIndex(1, PropertyId::GridTemplateColumns);
      auto areasIndex = LonghandIndex(2, PropertyId::GridTemplateAreas);
      auto autoFlowIndex = LonghandIndex(3, PropertyId::GridAutoFlow);
      auto autoRowsIndex = LonghandIndex(4, PropertyId::GridAutoRows);
      auto autoColumnsIndex = LonghandIndex(5, PropertyId::GridAutoColumns);

      auto &autoColumns = LonghandValue(autoColumnsIndex);
      auto &autoRows = LonghandValue(autoRowsIndex);
      auto &autoFlow = LonghandValue(autoFlowIndex);

      if (IsValueIdIncludingList(autoColumns, ValueId::Auto)
          && IsValueIdIncludingList(autoRows, ValueId::Auto)
          && IsValueIdIncludingList(autoFlow, ValueId::Row))
      {
        return SerialiseGridTemplate();
      }

      if (!IsLonghandValueNone(areasIndex))
      {
        return null;
      }

      auto &rows = LonghandValue(rowsIndex);
      auto &columns = LonghandValue(columnsIndex);

      bool autoFlowContainsDense = GridAutoFlowContains(autoFlow, ValueId::Dense);
      CSSOMStringView dense = autoFlowContainsDense ? u8" dense" : u8"";

      if (GridAutoFlowContains(autoFlow, ValueId::Column))
      {
        if (!IsValueIdIncludingList(autoRows, ValueId::Auto)
            || !IsValueIdIncludingList(columns, ValueId::None))
        {
          return null;
        }

        auto result = SerialiseLonghandValue(rowsIndex) + u8" / auto-flow";
        result.append(dense);
        if (!IsValueIdIncludingList(autoColumns, ValueId::Auto))
        {
          result.push_back(u8' ');
          result.append(SerialiseLonghandValue(autoColumnsIndex));
        }

        return result;
      }

      if (!GridAutoFlowContains(autoFlow, ValueId::Row) && !autoFlowContainsDense)
      {
        return null;
      }

      if (!IsValueIdIncludingList(autoColumns, ValueId::Auto) || !IsValueIdIncludingList(rows, ValueId::None))
      {
        return null;
      }

      CSSOMString result = u8"auto-flow";
      result.append(dense);
      if (!IsValueIdIncludingList(autoRows, ValueId::Auto))
      {
        result.push_back(u8' ');
        result.append(SerialiseLonghandValue(autoRowsIndex));
      }

      result.append(u8" / ");
      result.append(SerialiseLonghandValue(columnsIndex));
      return result;
    }

    KRYS_NODISCARD bool CanOmitTrailingGridAreaValue(const Value &value, const Value &trailing,
                                                     const SerialisationContext &context) noexcept
    {
      if (IsCustomIdentValue(value))
      {
        return IsCustomIdentValue(trailing) && value.CssText(context) == trailing.CssText(context);
      }

      return IsValueId(trailing, ValueId::Auto);
    }

    CSSOMString ShorthandSerialiser::SerialiseGridArea() const noexcept
    {
      krys_debug_assert(Length() == 4);
      auto longhandsToSerialise = 4uz;
      if (CanOmitTrailingGridAreaValue(LonghandValue(1), LonghandValue(3), _serialisationContext))
      {
        --longhandsToSerialise;
        if (CanOmitTrailingGridAreaValue(LonghandValue(0), LonghandValue(2), _serialisationContext))
        {
          --longhandsToSerialise;
          if (CanOmitTrailingGridAreaValue(LonghandValue(0), LonghandValue(1), _serialisationContext))
          {
            --longhandsToSerialise;
          }
        }
      }

      return SerialiseLonghands(longhandsToSerialise, u8" / ");
    }

    CSSOMString ShorthandSerialiser::SerialiseGridRowColumn() const noexcept
    {
      krys_debug_assert(Length() == 2);
      return SerialiseLonghands(
        CanOmitTrailingGridAreaValue(LonghandValue(0), LonghandValue(1), _serialisationContext) ? 1 : 2,
        u8" / ");
    }

    Maybe<CSSOMString> ShorthandSerialiser::SerialiseGridTemplate() const noexcept
    {
      krys_debug_assert(Length() >= 3);

      auto rowsIndex = LonghandIndex(0, PropertyId::GridTemplateRows);
      auto columnsIndex = LonghandIndex(1, PropertyId::GridTemplateColumns);
      auto areasIndex = LonghandIndex(2, PropertyId::GridTemplateAreas);

      auto *areasValue = DynamicDowncast<GridTemplateAreasValue>(LonghandValue(areasIndex));
      if (!areasValue)
      {
        if (IsLonghandValueNone(rowsIndex) && IsLonghandValueNone(columnsIndex))
        {
          return NameString(ValueId::None);
        }

        return SerialiseLonghands(2, u8" / ");
      }

      // Depending on the values of grid-template-rows and grid-template-columns, we may not
      // be able to completely represent them in this version of the grid-template shorthand.
      // We need to make sure that those values map to a value the syntax supports
      auto isValidTrackSize = [&](const Value &value)
      {
        auto valueId = value.ValueId();
        if (PropertyParserHelpers::IdentMatches<ValueId::FitContent, ValueId::Minmax>(valueId)
            || PropertyParserHelpers::IsGridBreadthIdent(valueId))
        {
          return true;
        }

        if (const auto *primitiveValue = DynamicDowncast<PrimitiveValue>(value))
        {
          return primitiveValue->IsLength() || primitiveValue->IsPercentage()
                 || primitiveValue->IsCalculated() || primitiveValue->IsFlex();
        }

        return false;
      };

      auto isValidExplicitTrackList = [&](const Value &value)
      {
        const auto *values = DynamicDowncast<ValueList>(value);
        if (!values)
        {
          return isValidTrackSize(value);
        }

        auto hasAtLeastOneTrackSize = false;
        for (const auto &item : *values)
        {
          if (isValidTrackSize(item))
          {
            hasAtLeastOneTrackSize = true;
          }
          else if (!item.IsGridLineNamesValue())
          {
            return false;
          }
        }

        return hasAtLeastOneTrackSize;
      };

      Ref<Value> rowTrackSizes = ShareRef(LonghandValue(rowsIndex));

      // Make sure the longhands can be expressed in this version of the shorthand.
      if (!rowTrackSizes->IsValueListValue()
          || (!IsLonghandValueNone(columnsIndex) && !isValidExplicitTrackList(LonghandValue(columnsIndex))))
      {
        return null;
      }

      CSSOMString result;
      auto row = 0uz;
      for (auto &currentValue : Downcast<ValueList>(*rowTrackSizes))
      {
        if (!result.empty())
        {
          result.push_back(u8' ');
        }

        if (auto *lineNames = DynamicDowncast<GridLineNamesValue>(currentValue))
        {
          result.append(lineNames->CustomCSSText(_serialisationContext));
        }
        else
        {
          result.push_back(u8'"');
          result.append(areasValue->StringForRow(row));
          result.push_back(u8'"');
          if (!isValidTrackSize(currentValue))
          {
            return null;
          }

          if (!IsValueId(currentValue, ValueId::Auto))
          {
            result.push_back(u8' ');
            result.append(currentValue.CssText(_serialisationContext));
          }

          row++;
        }
      }

      if (!IsLonghandValueNone(columnsIndex))
      {
        result.append(u8" / ");
        result.append(SerialiseLonghandValue(columnsIndex));
      }

      return result;
    }

    CSSOMString ShorthandSerialiser::SerialiseOffset() const noexcept
    {
      auto positionIndex = LonghandIndex(0, PropertyId::OffsetPosition);
      auto pathIndex = LonghandIndex(1, PropertyId::OffsetPath);
      auto distanceIndex = LonghandIndex(2, PropertyId::OffsetDistance);
      auto rotateIndex = LonghandIndex(3, PropertyId::OffsetRotate);
      auto anchorIndex = LonghandIndex(4, PropertyId::OffsetAnchor);

      bool includeDistance = !IsLonghandInitialValue(distanceIndex);
      bool includeRotate = !IsLonghandInitialValue(rotateIndex);
      bool includePath = includeDistance || includeRotate || !IsLonghandInitialValue(pathIndex);
      bool includePosition = !includePath || !IsLonghandInitialValue(positionIndex);
      bool includeAnchor = !IsLonghandInitialValue(anchorIndex);

      if (!includeDistance && !includeRotate && !includeAnchor)
      {
        if (includePosition && includePath)
        {
          return SerialiseLonghands(2);
        }

        if (includePosition)
        {
          return SerialiseLonghandValue(positionIndex);
        }

        krys_debug_assert(includePath);
        return SerialiseLonghandValue(pathIndex);
      }

      CSSOMString result;
      if (includePosition)
      {
        result.append(SerialiseLonghandValue(positionIndex));
      }

      if (includePath)
      {
        result.append(includePosition ? u8" " : u8"");
        result.append(SerialiseLonghandValue(pathIndex));
      }

      if (includeDistance)
      {
        result.push_back(u8' ');
        result.append(SerialiseLonghandValue(distanceIndex));
      }

      if (includeRotate)
      {
        result.push_back(u8' ');
        result.append(SerialiseLonghandValue(rotateIndex));
      }

      if (includeAnchor)
      {
        result.append(u8" / ");
        result.append(SerialiseLonghandValue(anchorIndex));
      }

      return result;
    }

    Maybe<CSSOMString> ShorthandSerialiser::SerialisePageBreak() const noexcept
    {
      auto keyword = LonghandValueId(0);
      switch (keyword)
      {
        case ValueId::Page:  return NameString(ValueId::Always);
        case ValueId::Auto:
        case ValueId::Avoid:
        case ValueId::Left:
        case ValueId::Right: return NameString(keyword);
        default:             return null;
      }
    }

    CSSOMString ShorthandSerialiser::SerialisePositionTry() const noexcept
    {
      auto positionTryOrderIndex = LonghandIndex(0, PropertyId::PositionTryOrder);
      auto positionTryFallbacksIndex = LonghandIndex(1, PropertyId::PositionTryFallbacks);

      auto positionTryFallbacksSerialisation = SerialiseLonghandValue(positionTryFallbacksIndex);
      if (IsLonghandInitialValue(positionTryOrderIndex))
      {
        return positionTryFallbacksSerialisation;
      }

      return SerialiseLonghandValue(positionTryOrderIndex) + u8' ' + positionTryFallbacksSerialisation;
    }

    Maybe<CSSOMString> ShorthandSerialiser::SerialiseLineClamp() const noexcept
    {
      auto isMaxLinesInitial = IsLonghandInitialValue(0);
      auto isBlockEllipsisInitial = IsLonghandInitialValue(1);
      if (isMaxLinesInitial && isBlockEllipsisInitial)
      {
        return NameString(ValueId::None);
      }

      if (isMaxLinesInitial != isBlockEllipsisInitial)
      {
        return null;
      }

      auto blockEllipsis = LonghandValueId(1);
      if (isBlockEllipsisInitial || (!isMaxLinesInitial && blockEllipsis == ValueId::Auto))
      {
        return SerialiseLonghands(1);
      }

      // FIXME: Add check for correct order.
      return SerialiseLonghands(2);
    }

    CSSOMString ShorthandSerialiser::SerialiseTextBox() const noexcept
    {
      auto textBoxTrim = LonghandValueId(0);
      auto &textBoxEdge = LonghandValue(LonghandIndex(1, PropertyId::TextBoxEdge));
      auto textBoxEdgeIsAuto = [&]()
      {
        if (auto *primitiveValue = DynamicDowncast<PrimitiveValue>(textBoxEdge))
        {
          return primitiveValue->ValueId() == ValueId::Auto;
        }

        return false;
      }();

      if (textBoxTrim == ValueId::None && textBoxEdgeIsAuto)
      {
        return NameString(ValueId::Normal);
      }

      if (textBoxEdgeIsAuto)
      {
        return NameString(textBoxTrim);
      }

      if (textBoxTrim == ValueId::TrimBoth)
      {
        return textBoxEdge.CssText(_serialisationContext);
      }

      return NameString(textBoxTrim) + u8' ' + textBoxEdge.CssText(_serialisationContext);
    }

    CSSOMString ShorthandSerialiser::SerialiseTextWrap() const noexcept
    {
      auto mode = LonghandValueId(0);
      auto style = LonghandValueId(1);

      if (style == ValueId::Auto)
      {
        return NameString(mode);
      }

      if (mode == ValueId::Wrap)
      {
        return NameString(style);
      }

      return NameString(mode) + u8' ' + NameString(style);
    }

    CSSOMString ShorthandSerialiser::SerialiseSingleAnimationRange(const Value &value,
                                                                   SingleAnimationRangeType type,
                                                                   ValueId startValueId) const noexcept
    {
      auto isDefault = [](const PrimitiveValue &value, SingleAnimationRangeType type)
      {
        if (!value.IsPercentage() || value.IsCalculated())
        {
          return false;
        }

        auto percentageValue = value.ResolveAsPercentageNoConversionDataRequired<double>();
        if (type == SingleAnimationRangeType::Start)
        {
          return percentageValue == 0;
        }

        return percentageValue == 100;
      };

      auto isRangeOffset = [](const PrimitiveValue &value)
      {
        return value.IsLength() || value.IsPercentage() || value.IsCalculatedPercentageWithLength();
      };

      if (auto *pair = DynamicDowncast<ValuePair>(value))
      {
        bool isSameNameAsStart = pair->First().ValueId() == startValueId;
        bool isStartValue = type == SingleAnimationRangeType::Start;
        bool isDefaultValue =
          isDefault(Downcast<PrimitiveValue>(pair->Second()), SingleAnimationRangeType::Start);
        if (isDefaultValue && (isStartValue || !isSameNameAsStart))
        {
          return NameString(pair->First().ValueId());
        }

        return pair->CssText(_serialisationContext);
      }

      if (auto *primitiveValue = DynamicDowncast<PrimitiveValue>(value))
      {
        if (isRangeOffset(*primitiveValue))
        {
          return primitiveValue->CssText(_serialisationContext);
        }

        bool isNormal = primitiveValue->ValueId() == ValueId::Normal;
        bool isSameNameAsStart = primitiveValue->ValueId() == startValueId;
        bool isStartValue = type == SingleAnimationRangeType::Start;
        if (isStartValue || (!isNormal && !isSameNameAsStart))
        {
          return NameString(primitiveValue->ValueId());
        }
      }

      return {};
    }

    Maybe<CSSOMString> ShorthandSerialiser::SerialiseAnimationRange() const noexcept
    {
      auto &startValue = LonghandValue(0);
      auto &endValue = LonghandValue(1);
      auto *startList = DynamicDowncast<ValueList>(startValue);
      auto *endList = DynamicDowncast<ValueList>(endValue);
      if (startList || endList)
      {
        // Lists of different lengths can't be represented by the shorthand.
        if (!startList || !endList || startList->size() != endList->size())
        {
          return null;
        }

        CSSOMString result;
        for (auto i = 0u; i < startList->size(); i++)
        {
          auto *start = startList->Item(i);
          auto *startPair = DynamicDowncast<ValuePair>(*start);
          auto startId = startPair ? startPair->First().ValueId() : start->ValueId();

          auto serialisedStart = SerialiseSingleAnimationRange(*start, SingleAnimationRangeType::Start);
          auto serialisedEnd =
            SerialiseSingleAnimationRange(*endList->Item(i), SingleAnimationRangeType::End, startId);
          result.append(serialisedStart);
          if (!serialisedEnd.empty())
          {
            result.push_back(u8' ');
            result.append(serialisedEnd);
          }

          if (i < startList->size() - 1)
          {
            result.append(u8", ");
          }
        }

        return result;
      }

      auto *startPair = DynamicDowncast<ValuePair>(startValue);
      auto startId = startPair ? startPair->First().ValueId() : startValue.ValueId();

      auto serialisedStart = SerialiseSingleAnimationRange(startValue, SingleAnimationRangeType::Start);
      auto serialisedEnd = SerialiseSingleAnimationRange(endValue, SingleAnimationRangeType::End, startId);
      if (serialisedEnd.empty())
      {
        return serialisedStart;
      }

      return serialisedStart + u8' ' + serialisedEnd;
    }

    CSSOMString ShorthandSerialiser::SerialiseWhiteSpace() const noexcept
    {
      auto whiteSpaceCollapse = LonghandValueId(0);
      auto textWrapMode = LonghandValueId(1);

      // Convert to backwards-compatible keywords if possible.
      if (whiteSpaceCollapse == ValueId::Collapse && textWrapMode == ValueId::Wrap)
      {
        return NameString(ValueId::Normal);
      }

      if (whiteSpaceCollapse == ValueId::Preserve && textWrapMode == ValueId::Nowrap)
      {
        return NameString(ValueId::Pre);
      }

      if (whiteSpaceCollapse == ValueId::Preserve && textWrapMode == ValueId::Wrap)
      {
        return NameString(ValueId::PreWrap);
      }

      if (whiteSpaceCollapse == ValueId::PreserveBreaks && textWrapMode == ValueId::Wrap)
      {
        return NameString(ValueId::PreLine);
      }

      // Omit default longhand values.
      if (whiteSpaceCollapse == ValueId::Collapse)
      {
        return NameString(textWrapMode);
      }

      if (textWrapMode == ValueId::Wrap)
      {
        return NameString(whiteSpaceCollapse);
      }

      return NameString(whiteSpaceCollapse) + u8' ' + NameString(textWrapMode);
    }
  }

  Maybe<CSSOMString> SerialiseShorthandValue(const SerialisationContext &context,
                                             const StyleProperties &properties, PropertyId shorthand) noexcept
  {
    return ShorthandSerialiser(context, properties, shorthand).Serialise();
  }
}
