#include "Krystal.Booey/Editing/EditingStyle.hpp"
#include "Krystal.Booey/CSS/CSSStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Font.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyShorthand.hpp"
#include "Krystal.Booey/CSS/Properties/StylePropertiesInlines.hpp"
#include "Krystal.Booey/CSS/Values/ColorValue.hpp"
#include "Krystal.Booey/CSS/Values/FontStyleWithAngleValue.hpp"
#include "Krystal.Booey/CSS/Values/GlobalKeyword.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/WebKitFontFamilyNames.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Position.hpp"
#include "Krystal.Booey/DOM/SimpleRange.hpp"
#include "Krystal.Booey/DOM/StyledElement.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/ApplyStyleCommand.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Editing/HTMLInterchange.hpp"
#include "Krystal.Booey/Editing/VisibleSelection.hpp"
#include "Krystal.Booey/Editing/VisibleUnits.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"
#include "Krystal.Booey/Graphics/FontCache.hpp"
#include "Krystal.Booey/Graphics/FontSelectionAlgorithm.hpp"
#include "Krystal.Booey/HTML/HTMLFontElement.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Style/Extractor.hpp"
#include "Krystal.Booey/Style/FontSizeFunctions.hpp"
#include "Krystal.Booey/Style/ResolveForFont.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include <algorithm>
#include <array>

namespace krys::boo
{
  using css::PropertyId;
  using css::ValueId;

  namespace
  {
    constexpr std::array EditingProperties {
      PropertyId::CaretColor,
      PropertyId::Color,
      PropertyId::FontFamily,
      PropertyId::FontSize,
      PropertyId::FontStyle,
      PropertyId::FontVariantCaps,
      PropertyId::FontWeight,
      PropertyId::LetterSpacing,
      PropertyId::Orphans,
      PropertyId::TextAlign,
      PropertyId::TextIndent,
      PropertyId::TextTransform,
      PropertyId::TextWrapMode,
      PropertyId::WhiteSpaceCollapse,
      PropertyId::Widows,
      PropertyId::WordSpacing,
      PropertyId::WebkitTextDecorationsInEffect,
      PropertyId::WebkitTextFillColor,

      // Non-inheritable properties
      PropertyId::BackgroundColor,
      PropertyId::TextDecorationLine,
      PropertyId::TextDecorationThickness,
      PropertyId::TextDecorationStyle,
      PropertyId::TextDecorationColor,
    };

    constexpr std::array PostLayoutEditingProperties {
      PropertyId::FontWeight, PropertyId::FontStyle, PropertyId::WebkitTextDecorationsInEffect,
      PropertyId::Direction,  PropertyId::TextAlign, PropertyId::Color,
    };

    constexpr size_t NumAllEditingProperties = EditingProperties.size();
    constexpr size_t NumInheritableEditingProperties = NumAllEditingProperties - 5;

    enum class EditingPropertiesToInclude : uint8
    {
      OnlyInheritableEditingProperties,
      AllEditingProperties,
      PostLayoutProperties
    };

    template <typename TStyleDeclaration>
    KRYS_NODISCARD Ref<css::MutableStyleProperties>
      CopyEditingProperties(TStyleDeclaration *style, EditingPropertiesToInclude type) noexcept
    {
      if (type == EditingPropertiesToInclude::AllEditingProperties)
      {
        return style->CopyProperties(Span<const PropertyId>(EditingProperties));
      }
      if (type == EditingPropertiesToInclude::PostLayoutProperties)
      {
        return style->CopyProperties(Span<const PropertyId>(PostLayoutEditingProperties));
      }
      return style->CopyProperties(
        Span<const PropertyId>(EditingProperties).first(NumInheritableEditingProperties));
    }

    KRYS_NODISCARD bool IsEditingProperty(PropertyId id) noexcept
    {
      return std::ranges::find(EditingProperties, id) != EditingProperties.end();
    }

    KRYS_NODISCARD Ref<css::MutableStyleProperties>
      CopyPropertiesFromComputedStyle(style::Extractor &computedStyle,
                                      EditingStyle::PropertiesToInclude propertiesToInclude) noexcept
    {
      switch (propertiesToInclude)
      {
        case EditingStyle::PropertiesToInclude::OnlyEditingInheritableProperties:
          return CopyEditingProperties(&computedStyle,
                                       EditingPropertiesToInclude::OnlyInheritableEditingProperties);
        case EditingStyle::PropertiesToInclude::EditingPropertiesInEffect:
          return CopyEditingProperties(&computedStyle, EditingPropertiesToInclude::AllEditingProperties);
        case EditingStyle::PropertiesToInclude::PostLayoutProperties:
          return CopyEditingProperties(&computedStyle, EditingPropertiesToInclude::PostLayoutProperties);
        case EditingStyle::PropertiesToInclude::AllProperties: break;
      }
      return computedStyle.CopyProperties();
    }

    KRYS_NODISCARD Ref<css::MutableStyleProperties>
      CopyPropertiesFromComputedStyle(dom::Node *node,
                                      EditingStyle::PropertiesToInclude propertiesToInclude) noexcept
    {
      style::Extractor computedStyle(node);
      return CopyPropertiesFromComputedStyle(computedStyle, propertiesToInclude);
    }

    KRYS_NODISCARD RefPtr<css::Value> ExtractPropertyValue(const css::StyleProperties &style,
                                                           PropertyId propertyID) noexcept
    {
      return style.GetPropertyCSSValue(propertyID);
    }

    KRYS_NODISCARD RefPtr<css::Value> ExtractPropertyValue(style::Extractor &computedStyle,
                                                           PropertyId propertyID) noexcept
    {
      return computedStyle.PropertyValue(propertyID);
    }

    // This synthesizes ValueId::Bold and ValueId::Italic when appropriate, and never returns
    // ValueId::Oblique.
    template <typename T>
    KRYS_NODISCARD ValueId IdentifierForStyleProperty(T &style, PropertyId propertyID) noexcept
    {
      RefPtr<css::Value> value = ExtractPropertyValue(style, propertyID);
      if (auto *fontStyleValue = DynamicDowncast<css::FontStyleWithAngleValue>(value.get()))
      {
        krys_debug_assert(propertyID == PropertyId::FontStyle);
        auto resolvedAngle = style::FontStyleAngleFromCSSFontStyleWithAngleValueDeprecated(*fontStyleValue);
        if (!resolvedAngle)
        {
          return ValueId::Normal;
        }
        return *resolvedAngle >= ItalicThreshold() ? ValueId::Italic : ValueId::Normal;
      }
      if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value.get()))
      {
        if (propertyID == PropertyId::FontWeight && primitiveValue->IsNumber()
            && primitiveValue->template ResolveAsNumberDeprecated<float>()
                 >= static_cast<float>(BoldThreshold()))
        {
          return ValueId::Bold;
        }
        auto identifier = primitiveValue->ValueId();
        if (identifier == ValueId::Oblique)
        {
          krys_debug_assert(propertyID == PropertyId::FontStyle);
          return ValueId::Italic;
        }
        return identifier;
      }
      return ValueId::Invalid;
    }

    enum class LegacyFontSizeMode : bool
    {
      AlwaysUseLegacyFontSize,
      UseLegacyFontSizeOnlyIfPixelValuesMatch
    };

    KRYS_NODISCARD int32 LegacyFontSizeFromCSSValue(dom::Document &document, css::PrimitiveValue *value,
                                                    bool shouldUseFixedFontDefaultSize,
                                                    LegacyFontSizeMode mode) noexcept;
    KRYS_NODISCARD bool HasTransparentBackgroundColor(css::StyleProperties *style) noexcept;
    KRYS_NODISCARD RefPtr<css::Value> BackgroundColorInEffect(dom::Node *node) noexcept;

    KRYS_NODISCARD bool IsValueID(const css::Value &value, ValueId valueID) noexcept
    {
      auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value);
      return primitiveValue && primitiveValue->ValueId() == valueID;
    }

    bool RemoveAll(css::ValueListBuilder &list, ValueId valueID) noexcept
    {
      auto removed =
        std::ranges::remove_if(list, [&](const Ref<css::Value> &item) { return IsValueID(*item, valueID); });
      bool didRemove = !removed.empty();
      list.erase(removed.begin(), removed.end());
      return didRemove;
    }

    bool RemoveAll(css::ValueListBuilder &list, const css::Value &value) noexcept
    {
      auto removed =
        std::ranges::remove_if(list, [&](const Ref<css::Value> &item) { return value.Equals(*item); });
      bool didRemove = !removed.empty();
      list.erase(removed.begin(), removed.end());
      return didRemove;
    }

    KRYS_NODISCARD bool Contains(const css::ValueListBuilder &list, ValueId valueID) noexcept
    {
      return std::ranges::any_of(list,
                                 [&](const Ref<css::Value> &item) { return IsValueID(*item, valueID); });
    }

    KRYS_NODISCARD bool CompareCSSValuePtr(const RefPtr<css::Value> &first,
                                           const RefPtr<css::Value> &second) noexcept
    {
      if (first && second)
      {
        return first->Equals(*second);
      }
      return !first && !second;
    }
  }

  class HTMLElementEquivalent
  {
  protected:
    const PropertyId _propertyID;
    const RefPtr<css::PrimitiveValue> _primitiveValue;
    const Maybe<html::TagName> _tagName;

  public:
    HTMLElementEquivalent(PropertyId id, ValueId primitiveValue, html::TagName tagName) noexcept
        : _propertyID(id), _primitiveValue(css::PrimitiveValue::Create(primitiveValue)), _tagName(tagName)
    {
      krys_debug_assert(primitiveValue != ValueId::Invalid);
    }
    virtual ~HTMLElementEquivalent() noexcept = default;

    KRYS_NODISCARD virtual bool Matches(const dom::Element &element) const noexcept
    {
      return !_tagName || IsHTMLElementOfType(&element, *_tagName);
    }
    KRYS_NODISCARD virtual bool HasAttribute() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool PropertyExistsInStyle(const EditingStyle &style) const noexcept
    {
      return style._mutableStyle && style._mutableStyle->GetPropertyCSSValue(_propertyID);
    }
    KRYS_NODISCARD virtual bool ValueIsPresentInStyle(dom::Element &element,
                                                      const EditingStyle &style) const noexcept
    {
      return Matches(element)
             && style._mutableStyle->PropertyAsValueId(_propertyID) == _primitiveValue->ValueId();
    }
    virtual void AddToStyle(dom::Element *, EditingStyle *style) const noexcept
    {
      style->SetProperty(_propertyID, _primitiveValue->CssText(css::DefaultSerialisationContext()));
    }

  protected:
    explicit HTMLElementEquivalent(PropertyId id) noexcept : _propertyID(id)
    {
    }
    HTMLElementEquivalent(PropertyId id, html::TagName tagName) noexcept : _propertyID(id), _tagName(tagName)
    {
    }
  };

  class HTMLTextDecorationEquivalent : public HTMLElementEquivalent
  {
  private:
    bool _isUnderline;

  public:
    HTMLTextDecorationEquivalent(ValueId primitiveValue, html::TagName tagName) noexcept
        : HTMLElementEquivalent(PropertyId::TextDecorationLine, primitiveValue, tagName),
          _isUnderline(primitiveValue == ValueId::Underline)
    {
    }

    KRYS_NODISCARD bool PropertyExistsInStyle(const EditingStyle &style) const noexcept override
    {
      if (ChangeInStyle(style) != TextDecorationChange::None)
      {
        return true;
      }

      if (!style._mutableStyle)
      {
        return false;
      }

      auto &mutableStyle = *style._mutableStyle;
      return mutableStyle.GetPropertyCSSValue(PropertyId::WebkitTextDecorationsInEffect)
             || mutableStyle.GetPropertyCSSValue(PropertyId::TextDecorationLine);
    }

    KRYS_NODISCARD bool ValueIsPresentInStyle(dom::Element &element,
                                              const EditingStyle &style) const noexcept override
    {
      if (!Matches(element))
      {
        return false;
      }
      auto change = ChangeInStyle(style);
      if (change != TextDecorationChange::None)
      {
        return change == TextDecorationChange::Add;
      }
      auto *mutableStyle = style.Style();
      RefPtr<css::Value> styleValue =
        mutableStyle->GetPropertyCSSValue(PropertyId::WebkitTextDecorationsInEffect);
      if (!styleValue)
      {
        styleValue = mutableStyle->GetPropertyCSSValue(PropertyId::TextDecorationLine);
      }
      if (!_primitiveValue)
      {
        return false;
      }
      auto *valueList = DynamicDowncast<css::ValueList>(styleValue.get());
      return valueList && valueList->HasValue(*_primitiveValue);
    }

  private:
    KRYS_NODISCARD TextDecorationChange ChangeInStyle(const EditingStyle &style) const noexcept
    {
      return _isUnderline ? style.UnderlineChange() : style.StrikeThroughChange();
    }
  };

  namespace
  {
    KRYS_NODISCARD bool FontWeightValueIsBold(const css::Value &fontWeight) noexcept
    {
      auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(fontWeight);
      if (!primitiveValue)
      {
        return false;
      }

      auto valueID = primitiveValue->ValueId();

      if (css::IsGlobalKeyword(valueID))
      {
        return false;
      }

      switch (valueID)
      {
        case ValueId::Normal: return false;
        case ValueId::Bold:   return true;
        case ValueId::Bolder: return false;
        default:              break;
      }

      if (css::PropertyParserHelpers::IsSystemFontShorthand(valueID))
      {
        return false;
      }

      krys_debug_assert(primitiveValue->IsNumber());
      return primitiveValue->ResolveAsNumberDeprecated<float>() >= static_cast<float>(BoldThreshold());
    }
  }

  class HTMLFontWeightEquivalent : public HTMLElementEquivalent
  {
  public:
    explicit HTMLFontWeightEquivalent(html::TagName tagName) noexcept
        : HTMLElementEquivalent(PropertyId::FontWeight, ValueId::Bold, tagName)
    {
    }

  private:
    KRYS_NODISCARD bool ValueIsPresentInStyle(dom::Element &element,
                                              const EditingStyle &style) const noexcept override
    {
      RefPtr<css::Value> value = style._mutableStyle->GetPropertyCSSValue(_propertyID);
      return Matches(element) && value && FontWeightValueIsBold(*value);
    }
  };

  class HTMLAttributeEquivalent : public HTMLElementEquivalent
  {
  protected:
    dom::DOMStringAtom _attrName;

  public:
    HTMLAttributeEquivalent(PropertyId id, html::TagName tagName, dom::DOMStringAtom attrName) noexcept
        : HTMLElementEquivalent(id, tagName), _attrName(attrName)
    {
    }
    HTMLAttributeEquivalent(PropertyId id, dom::DOMStringAtom attrName) noexcept
        : HTMLElementEquivalent(id), _attrName(attrName)
    {
    }

    KRYS_NODISCARD bool Matches(const dom::Element &element) const noexcept override
    {
      return HTMLElementEquivalent::Matches(element) && element.HasAttribute(_attrName);
    }
    KRYS_NODISCARD bool HasAttribute() const noexcept override
    {
      return true;
    }
    KRYS_NODISCARD bool ValueIsPresentInStyle(dom::Element &element,
                                              const EditingStyle &style) const noexcept override
    {
      RefPtr<css::Value> value = AttributeValueAsCSSValue(&element);
      RefPtr<css::Value> styleValue = style._mutableStyle->GetPropertyCSSValue(_propertyID);

      return CompareCSSValuePtr(value, styleValue);
    }
    void AddToStyle(dom::Element *element, EditingStyle *style) const noexcept override
    {
      if (RefPtr<css::Value> value = AttributeValueAsCSSValue(element))
      {
        style->SetProperty(_propertyID, value->CssText(css::DefaultSerialisationContext()));
      }
    }
    KRYS_NODISCARD virtual RefPtr<css::Value> AttributeValueAsCSSValue(dom::Element *element) const noexcept
    {
      krys_debug_assert(element);
      auto value = element->GetAttribute(_attrName);
      if (!value)
      {
        return nullptr;
      }

      auto dummyStyle = css::MutableStyleProperties::Create();
      (void)dummyStyle->SetProperty(_propertyID, *value);
      return dummyStyle->GetPropertyCSSValue(_propertyID);
    }
    KRYS_NODISCARD dom::DOMStringAtom AttributeName() const noexcept
    {
      return _attrName;
    }
  };

  class HTMLFontSizeEquivalent : public HTMLAttributeEquivalent
  {
  public:
    HTMLFontSizeEquivalent() noexcept
        : HTMLAttributeEquivalent(PropertyId::FontSize, html::TagName::font, dom::DOMStringAtom(u8"size"))
    {
    }

    KRYS_NODISCARD RefPtr<css::Value> AttributeValueAsCSSValue(dom::Element *element) const noexcept override
    {
      krys_debug_assert(element);
      auto value = element->GetAttribute(_attrName);
      if (!value)
      {
        return nullptr;
      }
      auto size = html::HTMLFontElement::CSSValueFromFontSizeNumber(*value);
      if (!size)
      {
        return nullptr;
      }
      return css::PrimitiveValue::Create(*size);
    }
  };

  Ref<EditingStyle> EditingStyle::Create() noexcept
  {
    return CreateRef<EditingStyle>();
  }

  Ref<EditingStyle> EditingStyle::Create(dom::Node *node, PropertiesToInclude propertiesToInclude) noexcept
  {
    return CreateRef<EditingStyle>(node, propertiesToInclude);
  }

  Ref<EditingStyle> EditingStyle::Create(const dom::Position &position,
                                         PropertiesToInclude propertiesToInclude) noexcept
  {
    return CreateRef<EditingStyle>(position, propertiesToInclude);
  }

  Ref<EditingStyle> EditingStyle::Create(const css::StyleProperties *style) noexcept
  {
    return CreateRef<EditingStyle>(style);
  }

  Ref<EditingStyle> EditingStyle::Create(const css::CSSStyleProperties *style) noexcept
  {
    return CreateRef<EditingStyle>(style);
  }

  Ref<EditingStyle> EditingStyle::Create(PropertyId propertyID, const dom::DOMString &value) noexcept
  {
    return CreateRef<EditingStyle>(propertyID, value);
  }

  Ref<EditingStyle> EditingStyle::Create(PropertyId propertyID, ValueId value) noexcept
  {
    return CreateRef<EditingStyle>(propertyID, value);
  }

  EditingStyle::EditingStyle() noexcept = default;

  EditingStyle::EditingStyle(dom::Node *node, PropertiesToInclude propertiesToInclude) noexcept
  {
    Init(node, propertiesToInclude);
  }

  EditingStyle::EditingStyle(const dom::Position &position, PropertiesToInclude propertiesToInclude) noexcept
  {
    Init(position.DeprecatedNode(), propertiesToInclude);
  }

  EditingStyle::EditingStyle(const css::CSSStyleProperties *style) noexcept
  {
    if (style)
    {
      _mutableStyle = style->CopyProperties();
    }
    ExtractFontSizeDelta();
  }

  EditingStyle::EditingStyle(const css::StyleProperties *style) noexcept
  {
    if (style)
    {
      _mutableStyle = style->MutableCopy();
    }
    ExtractFontSizeDelta();
  }

  EditingStyle::EditingStyle(PropertyId propertyID, const dom::DOMString &value) noexcept
  {
    SetProperty(propertyID, value);
    ExtractFontSizeDelta();
  }

  EditingStyle::EditingStyle(PropertyId propertyID, ValueId value) noexcept
  {
    _mutableStyle = css::MutableStyleProperties::Create();
    (void)_mutableStyle->SetProperty(propertyID, value);
    ExtractFontSizeDelta();
  }

  EditingStyle::~EditingStyle() noexcept = default;

  namespace
  {
    KRYS_NODISCARD Color CSSValueToColor(const css::Value *value) noexcept
    {
      if (!value)
      {
        return Color::transparentBlack;
      }

      auto color = css::ColorValue::GetAbsoluteColor(*value);
      if (!color.IsValid())
      {
        return Color::transparentBlack;
      }
      return color;
    }

    template <typename T>
    KRYS_NODISCARD Color TextColorFromStyle(T &style) noexcept
    {
      return CSSValueToColor(ExtractPropertyValue(style, PropertyId::Color).get());
    }

    template <typename T>
    KRYS_NODISCARD Color CaretColorFromStyle(T &style) noexcept
    {
      return CSSValueToColor(ExtractPropertyValue(style, PropertyId::CaretColor).get());
    }

    template <typename T>
    KRYS_NODISCARD Color BackgroundColorFromStyle(T &style) noexcept
    {
      return CSSValueToColor(ExtractPropertyValue(style, PropertyId::BackgroundColor).get());
    }

    KRYS_NODISCARD Color RGBABackgroundColorInEffect(dom::Node *node) noexcept
    {
      return CSSValueToColor(BackgroundColorInEffect(node).get());
    }

    KRYS_NODISCARD ValueId TextAlignResolvingStartAndEnd(ValueId textAlign, ValueId direction) noexcept
    {
      switch (textAlign)
      {
        case ValueId::Center:
        case ValueId::WebkitCenter: return ValueId::Center;
        case ValueId::Justify:      return ValueId::Justify;
        case ValueId::Left:
        case ValueId::WebkitLeft:   return ValueId::Left;
        case ValueId::Right:
        case ValueId::WebkitRight:  return ValueId::Right;
        case ValueId::Start:        return direction != ValueId::Rtl ? ValueId::Left : ValueId::Right;
        case ValueId::End:          return direction == ValueId::Rtl ? ValueId::Right : ValueId::Left;
        default:                    break;
      }
      return ValueId::Invalid;
    }

    template <typename T>
    KRYS_NODISCARD ValueId TextAlignResolvingStartAndEnd(T &style) noexcept
    {
      return TextAlignResolvingStartAndEnd(IdentifierForStyleProperty(style, PropertyId::TextAlign),
                                           IdentifierForStyleProperty(style, PropertyId::Direction));
    }
  }

  void EditingStyle::Init(dom::Node *initialNode, PropertiesToInclude propertiesToInclude) noexcept
  {
    RefPtr<dom::Node> node = ShareRefPtr(initialNode);
    if (auto *tabSpan = ParentTabSpanNode(node.get()))
    {
      node = ShareRefPtr<dom::Node>(tabSpan->ParentNode());
    }
    else if (TabSpanNode(node.get()))
    {
      node = ShareRefPtr<dom::Node>(node->ParentNode());
    }

    style::Extractor computedStyleAtPosition(node.get());
    // FIXME: It's strange to not set background-color and text-decoration when propertiesToInclude is
    // EditingPropertiesInEffect. However the selection boundaries test fails without this ternary.
    _mutableStyle = CopyPropertiesFromComputedStyle(
      computedStyleAtPosition, propertiesToInclude == PropertiesToInclude::EditingPropertiesInEffect
                                 ? PropertiesToInclude::OnlyEditingInheritableProperties
                                 : propertiesToInclude);

    auto &mutableStyle = *_mutableStyle;
    if (propertiesToInclude == PropertiesToInclude::EditingPropertiesInEffect)
    {
      if (RefPtr<css::Value> value = BackgroundColorInEffect(node.get()))
      {
        (void)mutableStyle.SetProperty(PropertyId::BackgroundColor,
                                       value->CssText(css::DefaultSerialisationContext()));
      }
      if (RefPtr<css::Value> value =
            computedStyleAtPosition.PropertyValue(PropertyId::WebkitTextDecorationsInEffect))
      {
        (void)mutableStyle.SetProperty(PropertyId::TextDecorationLine,
                                       value->CssText(css::DefaultSerialisationContext()));
        (void)mutableStyle.SetProperty(PropertyId::TextDecorationThickness, ValueId::Auto);
        (void)mutableStyle.SetProperty(PropertyId::TextDecorationStyle, ValueId::Solid);
        (void)mutableStyle.SetProperty(PropertyId::TextDecorationColor, ValueId::Currentcolor);
        (void)mutableStyle.RemoveProperty(PropertyId::WebkitTextDecorationsInEffect);
      }
    }

    if (node && node->ComputedStyle())
    {
      bool shouldSetFontSize = false;
      {
        auto *renderStyle = node->ComputedStyle();
        RemoveTextFillAndStrokeColorsIfNeeded(renderStyle);
        shouldSetFontSize = renderStyle->GetFontDescription().KeywordSize();
      }

      if (shouldSetFontSize)
      {
        if (auto cssValue = computedStyleAtPosition.GetFontSizeCSSValuePreferringKeyword())
        {
          (void)mutableStyle.SetProperty(PropertyId::FontSize,
                                         cssValue->CssText(css::DefaultSerialisationContext()));
        }
      }
    }

    _shouldUseFixedDefaultFontSize = computedStyleAtPosition.UseFixedFontDefaultSize();
    ExtractFontSizeDelta();
  }

  void EditingStyle::RemoveTextFillAndStrokeColorsIfNeeded(const RenderStyle *renderStyle) noexcept
  {
    if (renderStyle->TextFillColor().IsCurrentColor())
    {
      (void)_mutableStyle->RemoveProperty(PropertyId::WebkitTextFillColor);
    }
  }

  void EditingStyle::SetProperty(PropertyId propertyID, const dom::DOMString &value,
                                 css::IsImportant important) noexcept
  {
    if (!_mutableStyle)
    {
      _mutableStyle = css::MutableStyleProperties::Create();
    }

    (void)_mutableStyle->SetProperty(propertyID, value, important);
  }

  // FIXME: -webkit-font-size-delta, which execCommand's font size changes set, with rich editing commands;
  // until then there's no font size delta to extract.
  void EditingStyle::ExtractFontSizeDelta() noexcept
  {
    if (!_mutableStyle)
    {
      return;
    }
  }

  bool EditingStyle::IsEmpty() const noexcept
  {
    return (!_mutableStyle || _mutableStyle->empty()) && _fontSizeDelta == NoFontDelta
           && UnderlineChange() == TextDecorationChange::None
           && StrikeThroughChange() == TextDecorationChange::None;
  }

  Ref<css::MutableStyleProperties> EditingStyle::StyleWithResolvedTextDecorations() const noexcept
  {
    bool hasTextDecorationChanges =
      UnderlineChange() != TextDecorationChange::None || StrikeThroughChange() != TextDecorationChange::None;
    if (_mutableStyle && !hasTextDecorationChanges)
    {
      return ShareRef(*_mutableStyle);
    }

    Ref<css::MutableStyleProperties> style =
      _mutableStyle ? _mutableStyle->MutableCopy() : css::MutableStyleProperties::Create();

    css::ValueListBuilder valueList;
    if (UnderlineChange() == TextDecorationChange::Add)
    {
      valueList.push_back(css::PrimitiveValue::Create(ValueId::Underline));
    }
    if (StrikeThroughChange() == TextDecorationChange::Add)
    {
      valueList.push_back(css::PrimitiveValue::Create(ValueId::LineThrough));
    }
    if (valueList.empty())
    {
      (void)style->RemoveProperty(PropertyId::TextDecorationLine);
    }
    else
    {
      style->SetProperty(PropertyId::TextDecorationLine,
                         css::ValueList::CreateSpaceSeparated(krys::move(valueList)));
    }

    (void)style->SetProperty(PropertyId::TextDecorationThickness, ValueId::Auto);
    (void)style->SetProperty(PropertyId::TextDecorationStyle, ValueId::Solid);
    (void)style->SetProperty(PropertyId::TextDecorationColor, ValueId::Currentcolor);

    return style;
  }

  Maybe<WritingDirection> EditingStyle::TextDirection() const noexcept
  {
    if (!_mutableStyle)
    {
      return null;
    }

    auto unicodeBidi = _mutableStyle->PropertyAsValueId(PropertyId::UnicodeBidi);

    if (unicodeBidi == ValueId::Embed)
    {
      auto direction = _mutableStyle->PropertyAsValueId(PropertyId::Direction);
      if (!direction)
      {
        return null;
      }

      return direction == ValueId::Ltr ? WritingDirection::LeftToRight : WritingDirection::RightToLeft;
    }

    if (unicodeBidi == ValueId::Normal)
    {
      return WritingDirection::Natural;
    }

    return null;
  }

  void EditingStyle::SetStyle(RefPtr<css::MutableStyleProperties> &&style) noexcept
  {
    _mutableStyle = krys::move(style);
    // FIXME: We should be able to figure out whether or not font is fixed width for mutable style.
    // We need to check font-family is monospace as in FontDescription but we don't want to duplicate code
    // here.
    _shouldUseFixedDefaultFontSize = false;
    ExtractFontSizeDelta();
  }

  void EditingStyle::OverrideWithStyle(const css::StyleProperties &style) noexcept
  {
    MergeStyle(&style, CSSPropertyOverrideMode::OverrideValues);
  }

  namespace
  {
    void ApplyTextDecorationChangeToValueList(css::ValueListBuilder &valueList, TextDecorationChange change,
                                              Ref<css::PrimitiveValue> &&value) noexcept
    {
      switch (change)
      {
        case TextDecorationChange::None:   break;
        case TextDecorationChange::Add:    valueList.push_back(krys::move(value)); break;
        case TextDecorationChange::Remove: (void)RemoveAll(valueList, *value); break;
      }
    }
  }

  void EditingStyle::OverrideTypingStyleAt(const EditingStyle &style, const dom::Position &position) noexcept
  {
    MergeStyle(style._mutableStyle.get(), CSSPropertyOverrideMode::OverrideValues);

    _fontSizeDelta += style._fontSizeDelta;

    PrepareToApplyAt(position, ShouldPreserveWritingDirection::Yes);

    auto underlineChange = style.UnderlineChange();
    auto strikeThroughChange = style.StrikeThroughChange();
    if (underlineChange == TextDecorationChange::None && strikeThroughChange == TextDecorationChange::None)
    {
      return;
    }

    if (!_mutableStyle)
    {
      _mutableStyle = css::MutableStyleProperties::Create();
    }

    auto underline = css::PrimitiveValue::Create(ValueId::Underline);
    auto lineThrough = css::PrimitiveValue::Create(ValueId::LineThrough);
    RefPtr<css::Value> value = _mutableStyle->GetPropertyCSSValue(PropertyId::WebkitTextDecorationsInEffect);
    css::ValueListBuilder valueList;
    if (auto *list = DynamicDowncast<css::ValueList>(value.get()))
    {
      valueList = list->CopyValues();
      ApplyTextDecorationChangeToValueList(valueList, underlineChange, krys::move(underline));
      ApplyTextDecorationChangeToValueList(valueList, strikeThroughChange, krys::move(lineThrough));
    }
    else
    {
      if (underlineChange == TextDecorationChange::Add)
      {
        valueList.push_back(krys::move(underline));
      }
      if (strikeThroughChange == TextDecorationChange::Add)
      {
        valueList.push_back(krys::move(lineThrough));
      }
    }
    _mutableStyle->SetProperty(PropertyId::WebkitTextDecorationsInEffect,
                               css::ValueList::CreateSpaceSeparated(krys::move(valueList)));
  }

  void EditingStyle::Clear() noexcept
  {
    _mutableStyle = nullptr;
    _shouldUseFixedDefaultFontSize = false;
    _fontSizeDelta = NoFontDelta;
    SetUnderlineChange(TextDecorationChange::None);
    SetStrikeThroughChange(TextDecorationChange::None);
  }

  Ref<EditingStyle> EditingStyle::Copy() const noexcept
  {
    auto copy = EditingStyle::Create();
    if (_mutableStyle)
    {
      copy->_mutableStyle = _mutableStyle->MutableCopy();
    }
    copy->_shouldUseFixedDefaultFontSize = _shouldUseFixedDefaultFontSize;
    copy->_underlineChange = _underlineChange;
    copy->_strikeThroughChange = _strikeThroughChange;
    copy->_fontSizeDelta = _fontSizeDelta;
    return copy;
  }

  namespace
  {
    // This is the list of properties we want to copy in the CopyBlockProperties() function.
    // It is the list of CSS properties that apply specially to block-level elements.
    // FIXME: The legacy -webkit-column-break-* and page-break-* aliases are left out, as Booey has none.
    constexpr std::array BlockProperties {
      PropertyId::Orphans,
      PropertyId::Overflow, // This can be also be applied to replaced elements
      PropertyId::ColumnCount,     PropertyId::ColumnGap,       PropertyId::RowGap,
      PropertyId::ColumnRuleColor, PropertyId::ColumnRuleStyle, PropertyId::ColumnRuleWidth,
      PropertyId::ColumnWidth,     PropertyId::TextAlign,       PropertyId::TextAlignLast,
      PropertyId::TextJustify,     PropertyId::TextIndent,      PropertyId::Widows,
    };
  }

  Ref<EditingStyle> EditingStyle::ExtractAndRemoveBlockProperties() noexcept
  {
    auto result = EditingStyle::Create();
    if (_mutableStyle)
    {
      result->_mutableStyle = _mutableStyle->CopyProperties(Span<const PropertyId>(BlockProperties));
      (void)_mutableStyle->RemoveProperties(Span<const PropertyId>(BlockProperties));
    }
    return result;
  }

  Ref<EditingStyle> EditingStyle::ExtractAndRemoveTextDirection() noexcept
  {
    auto textDirection = EditingStyle::Create();
    textDirection->_mutableStyle = css::MutableStyleProperties::Create();
    auto &mutableStyleForTextDirection = *textDirection->_mutableStyle;
    auto &mutableStyle = *_mutableStyle;
    (void)mutableStyleForTextDirection.SetProperty(
      PropertyId::UnicodeBidi, ValueId::Embed,
      css::IsImportant(mutableStyle.PropertyIsImportant(PropertyId::UnicodeBidi)));
    (void)mutableStyleForTextDirection.SetProperty(
      PropertyId::Direction, mutableStyle.GetPropertyValue(PropertyId::Direction),
      css::IsImportant(mutableStyle.PropertyIsImportant(PropertyId::Direction)));

    (void)mutableStyle.RemoveProperty(PropertyId::UnicodeBidi);
    (void)mutableStyle.RemoveProperty(PropertyId::Direction);

    return textDirection;
  }

  void EditingStyle::RemoveBlockProperties() noexcept
  {
    if (!_mutableStyle)
    {
      return;
    }

    (void)_mutableStyle->RemoveProperties(Span<const PropertyId>(BlockProperties));
  }

  void EditingStyle::RemoveStyleAddedByNode(dom::Node *node) noexcept
  {
    if (!node || !node->ParentNode())
    {
      return;
    }
    auto parentStyle =
      CopyPropertiesFromComputedStyle(node->ParentNode(), PropertiesToInclude::EditingPropertiesInEffect);
    auto nodeStyle = CopyPropertiesFromComputedStyle(node, PropertiesToInclude::EditingPropertiesInEffect);
    RemoveEquivalentProperties(*parentStyle);
    RemoveEquivalentProperties(*nodeStyle);
  }

  void EditingStyle::RemoveStyleConflictingWithStyleOfNode(dom::Node &node) noexcept
  {
    if (!node.ParentNode() || !_mutableStyle)
    {
      return;
    }

    auto parentStyle =
      CopyPropertiesFromComputedStyle(node.ParentNode(), PropertiesToInclude::EditingPropertiesInEffect);
    auto nodeStyle = EditingStyle::Create(&node, PropertiesToInclude::EditingPropertiesInEffect);
    nodeStyle->RemoveEquivalentProperties(*parentStyle);

    for (auto property : *nodeStyle->Style())
    {
      (void)_mutableStyle->RemoveProperty(property.Id());
    }
  }

  void EditingStyle::CollapseTextDecorationProperties() noexcept
  {
    if (!_mutableStyle)
    {
      return;
    }

    auto &mutableStyle = *_mutableStyle;
    RefPtr<css::Value> textDecorationsInEffect =
      mutableStyle.GetPropertyCSSValue(PropertyId::WebkitTextDecorationsInEffect);
    if (!textDecorationsInEffect)
    {
      return;
    }

    if (Is<css::ValueList>(*textDecorationsInEffect))
    {
      auto isImportant = css::IsImportant(mutableStyle.PropertyIsImportant(PropertyId::TextDecorationLine));
      (void)mutableStyle.SetProperty(PropertyId::TextDecoration,
                                     textDecorationsInEffect->CssText(css::DefaultSerialisationContext()),
                                     isImportant);
    }
    else
    {
      (void)mutableStyle.RemoveProperty(PropertyId::TextDecoration);
    }
    (void)mutableStyle.RemoveProperty(PropertyId::WebkitTextDecorationsInEffect);
  }

  namespace
  {
    // CSS properties that create a visual difference only when applied to text.
    constexpr std::array TextOnlyProperties {
      PropertyId::TextDecorationLine,
      PropertyId::WebkitTextDecorationsInEffect,
      PropertyId::FontStyle,
      PropertyId::FontWeight,
      PropertyId::Color,
    };

    template <typename T>
    KRYS_NODISCARD Ref<css::MutableStyleProperties>
      GetPropertiesNotIn(css::StyleProperties &styleWithRedundantProperties, T &baseStyle) noexcept;
  }

  TriState EditingStyle::TriStateOfStyle(EditingStyle *style) const noexcept
  {
    if (!style || !style->_mutableStyle)
    {
      return TriState::False;
    }
    return TriStateOfStyle(*style->_mutableStyle, ShouldIgnoreTextOnlyProperties::No);
  }

  template <typename T>
  TriState EditingStyle::TriStateOfStyle(
    T &styleToCompare, ShouldIgnoreTextOnlyProperties shouldIgnoreTextOnlyProperties) const noexcept
  {
    if (!_mutableStyle)
    {
      return TriState::True;
    }

    auto difference = GetPropertiesNotIn(*_mutableStyle, styleToCompare);

    if (shouldIgnoreTextOnlyProperties == ShouldIgnoreTextOnlyProperties::Yes)
    {
      (void)difference->RemoveProperties(Span<const PropertyId>(TextOnlyProperties));
    }

    if (difference->empty())
    {
      return TriState::True;
    }
    if (difference->PropertyCount() == _mutableStyle->PropertyCount())
    {
      return TriState::False;
    }

    return TriState::Indeterminate;
  }

  TriState EditingStyle::TriStateOfStyle(const VisibleSelection &selection) const noexcept
  {
    if (!selection.IsCaretOrRange())
    {
      return TriState::False;
    }

    if (selection.IsCaret())
    {
      return TriStateOfStyle(EditingStyle::StyleAtSelectionStart(selection).get());
    }

    TriState state = TriState::False;
    bool nodeIsStart = true;
    for (auto *node = selection.Start().DeprecatedNode(); node; node = dom::TreeQueries::Next(*node))
    {
      if (node->Renderer() && node->HasEditableStyle())
      {
        style::Extractor computedStyle(node);
        TriState nodeState =
          TriStateOfStyle(computedStyle, Is<dom::Text>(*node) ? ShouldIgnoreTextOnlyProperties::No
                                                              : ShouldIgnoreTextOnlyProperties::Yes);
        if (nodeIsStart)
        {
          state = nodeState;
          nodeIsStart = false;
        }
        else if (state != nodeState && Is<dom::Text>(*node))
        {
          state = TriState::Indeterminate;
          break;
        }
      }

      if (node == selection.End().DeprecatedNode())
      {
        break;
      }
    }

    return state;
  }

  namespace
  {
    KRYS_NODISCARD RefPtr<css::ValueList>
      TextDecorationValueList(const css::StyleProperties &properties) noexcept
    {
      return ShareRefPtr(DynamicDowncast<css::ValueList>(
        properties.GetPropertyCSSValue(PropertyId::TextDecorationLine).get()));
    }
  }

  bool EditingStyle::ConflictsWithInlineStyleOfElement(dom::StyledElement &element,
                                                       RefPtr<css::MutableStyleProperties> *newInlineStylePtr,
                                                       EditingStyle *extractedStyle) const noexcept
  {
    RefPtr<const css::StyleProperties> inlineStyle = ShareRefPtr(element.InlineStyle());
    if (!inlineStyle)
    {
      return false;
    }
    bool conflicts = false;
    RefPtr<css::MutableStyleProperties> newInlineStyle;
    if (newInlineStylePtr)
    {
      newInlineStyle = inlineStyle->MutableCopy();
      *newInlineStylePtr = newInlineStyle;
    }

    bool shouldRemoveUnderline = UnderlineChange() == TextDecorationChange::Remove;
    bool shouldRemoveStrikeThrough = StrikeThroughChange() == TextDecorationChange::Remove;
    if (shouldRemoveUnderline || shouldRemoveStrikeThrough)
    {
      if (auto valueList = TextDecorationValueList(*inlineStyle))
      {
        auto newValueList = valueList->CopyValues();
        css::ValueListBuilder extractedValueList;

        if (shouldRemoveUnderline && valueList->HasValue(ValueId::Underline))
        {
          if (!newInlineStyle)
          {
            return true;
          }
          (void)RemoveAll(newValueList, ValueId::Underline);
          extractedValueList.push_back(css::PrimitiveValue::Create(ValueId::Underline));
        }

        if (shouldRemoveStrikeThrough && valueList->HasValue(ValueId::LineThrough))
        {
          if (!newInlineStyle)
          {
            return true;
          }
          (void)RemoveAll(newValueList, ValueId::LineThrough);
          extractedValueList.push_back(css::PrimitiveValue::Create(ValueId::LineThrough));
        }

        if (!extractedValueList.empty())
        {
          conflicts = true;
          if (newValueList.empty())
          {
            (void)newInlineStyle->RemoveProperty(PropertyId::TextDecoration);
          }
          else
          {
            (void)newInlineStyle->SetProperty(PropertyId::TextDecorationThickness, ValueId::Auto);
            (void)newInlineStyle->SetProperty(PropertyId::TextDecorationStyle, ValueId::Solid);
            (void)newInlineStyle->SetProperty(PropertyId::TextDecorationColor, ValueId::Currentcolor);
            (void)newInlineStyle->SetProperty(PropertyId::TextDecorationLine,
                                              css::ValueList::CreateSpaceSeparated(krys::move(newValueList))
                                                ->CssText(css::DefaultSerialisationContext()));
          }
          if (extractedStyle)
          {
            auto isImportant =
              css::IsImportant(inlineStyle->PropertyIsImportant(PropertyId::TextDecorationLine));
            extractedStyle->SetProperty(PropertyId::TextDecoration,
                                        css::ValueList::CreateSpaceSeparated(extractedValueList)
                                          ->CssText(css::DefaultSerialisationContext()),
                                        isImportant);
          }
        }
      }
    }

    if (!_mutableStyle)
    {
      return conflicts;
    }

    for (auto property : *_mutableStyle)
    {
      auto propertyID = property.Id();

      // We don't override whitespace property of a tab span because that would collapse the tab into a space.
      if ((propertyID == PropertyId::WhiteSpaceCollapse || propertyID == PropertyId::TextWrapMode)
          && TabSpanNode(&element))
      {
        continue;
      }

      if (propertyID == PropertyId::WebkitTextDecorationsInEffect
          && inlineStyle->GetPropertyCSSValue(PropertyId::TextDecorationLine))
      {
        if (!newInlineStyle)
        {
          return true;
        }
        conflicts = true;
        (void)newInlineStyle->RemoveProperty(PropertyId::TextDecoration);
        if (extractedStyle)
        {
          auto isImportant =
            css::IsImportant(inlineStyle->PropertyIsImportant(PropertyId::TextDecorationLine));
          extractedStyle->SetProperty(PropertyId::TextDecoration,
                                      inlineStyle->GetPropertyValue(PropertyId::TextDecorationLine),
                                      isImportant);
        }
      }

      if (!inlineStyle->GetPropertyCSSValue(propertyID))
      {
        continue;
      }

      if (propertyID == PropertyId::UnicodeBidi && inlineStyle->GetPropertyCSSValue(PropertyId::Direction))
      {
        if (!newInlineStyle)
        {
          return true;
        }
        conflicts = true;
        (void)newInlineStyle->RemoveProperty(PropertyId::Direction);
        if (extractedStyle)
        {
          auto isImportant = css::IsImportant(inlineStyle->PropertyIsImportant(propertyID));
          extractedStyle->SetProperty(propertyID, inlineStyle->GetPropertyValue(propertyID), isImportant);
        }
      }

      if (!newInlineStyle)
      {
        return true;
      }

      conflicts = true;
      (void)newInlineStyle->RemoveProperty(propertyID);
      if (extractedStyle)
      {
        auto isImportant = css::IsImportant(inlineStyle->PropertyIsImportant(propertyID));
        extractedStyle->SetProperty(propertyID, inlineStyle->GetPropertyValue(propertyID), isImportant);
      }
    }

    return conflicts;
  }

  namespace
  {
    KRYS_NODISCARD Span<const HTMLElementEquivalent *const> HTMLElementEquivalents() noexcept
    {
      static const HTMLElementEquivalent *const equivalents[] = {
        new HTMLFontWeightEquivalent(html::TagName::b),
        new HTMLFontWeightEquivalent(html::TagName::strong),

        new HTMLElementEquivalent(PropertyId::VerticalAlign, ValueId::Sub, html::TagName::sub),
        new HTMLElementEquivalent(PropertyId::VerticalAlign, ValueId::Super, html::TagName::sup),
        new HTMLElementEquivalent(PropertyId::FontStyle, ValueId::Italic, html::TagName::i),
        new HTMLElementEquivalent(PropertyId::FontStyle, ValueId::Italic, html::TagName::em),

        new HTMLTextDecorationEquivalent(ValueId::Underline, html::TagName::u),
        new HTMLTextDecorationEquivalent(ValueId::LineThrough, html::TagName::s),
        new HTMLTextDecorationEquivalent(ValueId::LineThrough, html::TagName::strike),
      };
      return equivalents;
    }

    KRYS_NODISCARD Span<const HTMLAttributeEquivalent *const> HTMLAttributeEquivalents() noexcept
    {
      static const HTMLAttributeEquivalent *const equivalents[] = {
        // ElementIsStyledSpanOrHTMLEquivalent depends on the fact each HTMLAttributeEquivalent matches
        // exactly
        // one attribute of exactly one element except dir.
        new HTMLAttributeEquivalent(PropertyId::Color, html::TagName::font, dom::DOMStringAtom(u8"color")),
        new HTMLAttributeEquivalent(PropertyId::FontFamily, html::TagName::font,
                                    dom::DOMStringAtom(u8"face")),
        new HTMLFontSizeEquivalent,

        new HTMLAttributeEquivalent(PropertyId::Direction, dom::DOMStringAtom(u8"dir")),
        new HTMLAttributeEquivalent(PropertyId::UnicodeBidi, dom::DOMStringAtom(u8"dir")),
      };
      return equivalents;
    }
  }

  bool EditingStyle::ConflictsWithImplicitStyleOfElement(
    dom::Element &element, EditingStyle *extractedStyle,
    ShouldExtractMatchingStyle shouldExtractMatchingStyle) const noexcept
  {
    if (IsEmpty())
    {
      return false;
    }

    for (auto *equivalent : HTMLElementEquivalents())
    {
      if (equivalent->Matches(element) && equivalent->PropertyExistsInStyle(*this)
          && (shouldExtractMatchingStyle == ShouldExtractMatchingStyle::Yes
              || !equivalent->ValueIsPresentInStyle(element, *this)))
      {
        if (extractedStyle)
        {
          equivalent->AddToStyle(&element, extractedStyle);
        }
        return true;
      }
    }
    return false;
  }

  bool EditingStyle::ConflictsWithImplicitStyleOfAttributes(dom::Element &element) const noexcept
  {
    if (IsEmpty())
    {
      return false;
    }

    for (auto *equivalent : HTMLAttributeEquivalents())
    {
      if (equivalent->Matches(element) && equivalent->PropertyExistsInStyle(*this)
          && !equivalent->ValueIsPresentInStyle(element, *this))
      {
        return true;
      }
    }

    return false;
  }

  bool EditingStyle::ExtractConflictingImplicitStyleOfAttributes(
    dom::Element &element, ShouldPreserveWritingDirection shouldPreserveWritingDirection,
    EditingStyle *extractedStyle, List<dom::DOMStringAtom> &conflictingAttributes,
    ShouldExtractMatchingStyle shouldExtractMatchingStyle) const noexcept
  {
    // HTMLAttributeEquivalent::AddToStyle doesn't support unicode-bidi and direction properties
    krys_debug_assert(!extractedStyle
                      || shouldPreserveWritingDirection == ShouldPreserveWritingDirection::Yes);
    if (!_mutableStyle)
    {
      return false;
    }

    bool removed = false;
    for (auto *equivalent : HTMLAttributeEquivalents())
    {
      // unicode-bidi and direction are pushed down separately so don't push down with other styles.
      if (shouldPreserveWritingDirection == ShouldPreserveWritingDirection::Yes
          && equivalent->AttributeName() == u8"dir")
      {
        continue;
      }

      if (!equivalent->Matches(element) || !equivalent->PropertyExistsInStyle(*this)
          || (shouldExtractMatchingStyle == ShouldExtractMatchingStyle::No
              && equivalent->ValueIsPresentInStyle(element, *this)))
      {
        continue;
      }

      if (extractedStyle)
      {
        equivalent->AddToStyle(&element, extractedStyle);
      }
      conflictingAttributes.push_back(equivalent->AttributeName());
      removed = true;
    }

    return removed;
  }

  bool EditingStyle::StyleIsPresentInComputedStyleOfNode(dom::Node &node) const noexcept
  {
    if (IsEmpty())
    {
      return true;
    }
    style::Extractor computedStyle(&node);

    bool shouldAddUnderline = UnderlineChange() == TextDecorationChange::Add;
    bool shouldAddLineThrough = StrikeThroughChange() == TextDecorationChange::Add;
    if (shouldAddUnderline || shouldAddLineThrough)
    {
      bool hasUnderline = false;
      bool hasLineThrough = false;
      if (RefPtr<css::Value> value = computedStyle.PropertyValue(PropertyId::TextDecorationLine))
      {
        if (auto *valueList = DynamicDowncast<css::ValueList>(*value))
        {
          hasUnderline = valueList->HasValue(ValueId::Underline);
          hasLineThrough = valueList->HasValue(ValueId::LineThrough);
        }
      }
      if ((shouldAddUnderline && !hasUnderline) || (shouldAddLineThrough && !hasLineThrough))
      {
        return false;
      }
    }

    return !_mutableStyle || GetPropertiesNotIn(*_mutableStyle, computedStyle)->empty();
  }

  bool EditingStyle::ElementIsStyledSpanOrHTMLEquivalent(const dom::Element &element) noexcept
  {
    bool elementIsSpanOrElementEquivalent = false;
    if (IsHTMLElementOfType(&element, html::TagName::span))
    {
      elementIsSpanOrElementEquivalent = true;
    }
    else
    {
      for (auto *equivalent : HTMLElementEquivalents())
      {
        if (equivalent->Matches(element))
        {
          elementIsSpanOrElementEquivalent = true;
          break;
        }
      }
    }

    if (!element.HasAttributes())
    {
      return elementIsSpanOrElementEquivalent; // span, b, etc... without any attributes
    }

    uint32 matchedAttributes = 0;
    for (auto *equivalent : HTMLAttributeEquivalents())
    {
      if (equivalent->Matches(element) && equivalent->AttributeName() != u8"dir")
      {
        matchedAttributes++;
      }
    }

    if (!elementIsSpanOrElementEquivalent && !matchedAttributes)
    {
      return false; // element is not a span, a html element equivalent, or font element.
    }

    if (element.GetAttribute(dom::DOMStringAtom(u8"class")) == dom::DOMString(AppleStyleSpanClass))
    {
      matchedAttributes++;
    }

    if (element.HasAttribute(dom::DOMStringAtom(u8"style")))
    {
      if (auto *styledElement = DynamicDowncast<dom::StyledElement>(element))
      {
        if (auto *style = styledElement->InlineStyle())
        {
          for (auto property : *style)
          {
            if (!IsEditingProperty(property.Id()))
            {
              return false;
            }
          }
        }
      }
      matchedAttributes++;
    }

    // font with color attribute, span with style attribute, etc...
    auto attributeCount = static_cast<uint32>(element.AttributeList().size());
    krys_debug_assert(matchedAttributes <= attributeCount);
    return matchedAttributes >= attributeCount;
  }

  void EditingStyle::PrepareToApplyAt(const dom::Position &position,
                                      ShouldPreserveWritingDirection shouldPreserveWritingDirection) noexcept
  {
    if (!_mutableStyle)
    {
      return;
    }
    auto &mutableStyle = *_mutableStyle;

    // ReplaceSelectionCommand::HandleStyleSpans() requires that this function only removes the editing style.
    // If this function was modified in the future to delete all redundant properties, then add a boolean
    // value to indicate which one of editingStyleAtPosition or computedStyle is called.
    auto editingStyleAtPosition =
      EditingStyle::Create(position, PropertiesToInclude::EditingPropertiesInEffect);
    auto &styleAtPosition = *editingStyleAtPosition->_mutableStyle;

    Maybe<ValueId> unicodeBidi;
    Maybe<ValueId> direction;
    if (shouldPreserveWritingDirection == ShouldPreserveWritingDirection::Yes)
    {
      unicodeBidi = mutableStyle.PropertyAsValueId(PropertyId::UnicodeBidi);
      direction = mutableStyle.PropertyAsValueId(PropertyId::Direction);
    }

    RemoveEquivalentProperties(styleAtPosition);

    if (TextAlignResolvingStartAndEnd(mutableStyle) == TextAlignResolvingStartAndEnd(styleAtPosition))
    {
      (void)mutableStyle.RemoveProperty(PropertyId::TextAlign);
    }

    if (EqualIgnoringSemanticColor(TextColorFromStyle(mutableStyle), TextColorFromStyle(styleAtPosition)))
    {
      (void)mutableStyle.RemoveProperty(PropertyId::Color);
    }

    if (EqualIgnoringSemanticColor(CaretColorFromStyle(mutableStyle), CaretColorFromStyle(styleAtPosition)))
    {
      (void)mutableStyle.RemoveProperty(PropertyId::CaretColor);
    }

    if (HasTransparentBackgroundColor(&mutableStyle)
        || CSSValueToColor(mutableStyle.GetPropertyCSSValue(PropertyId::BackgroundColor).get())
             == RGBABackgroundColorInEffect(position.GetContainerNode()))
    {
      (void)mutableStyle.RemoveProperty(PropertyId::BackgroundColor);
    }

    if (unicodeBidi)
    {
      (void)mutableStyle.SetProperty(PropertyId::UnicodeBidi, *unicodeBidi);
      if (direction)
      {
        (void)mutableStyle.SetProperty(PropertyId::Direction, *direction);
      }
    }
  }

  void EditingStyle::MergeTypingStyle(dom::Document &document) noexcept
  {
    RefPtr<EditingStyle> typingStyle = ShareRefPtr(document.Selection().TypingStyle());
    if (!typingStyle || typingStyle.get() == this)
    {
      return;
    }

    MergeStyle(typingStyle->_mutableStyle.get(), CSSPropertyOverrideMode::OverrideValues);
  }

  void EditingStyle::MergeInlineStyleOfElement(dom::StyledElement &element, CSSPropertyOverrideMode mode,
                                               PropertiesToInclude propertiesToInclude) noexcept
  {
    auto *inlineStyle = element.InlineStyle();
    if (!inlineStyle)
    {
      return;
    }

    switch (propertiesToInclude)
    {
      case PropertiesToInclude::AllProperties: MergeStyle(inlineStyle, mode); return;
      case PropertiesToInclude::OnlyEditingInheritableProperties:
        MergeStyle(
          CopyEditingProperties(inlineStyle, EditingPropertiesToInclude::OnlyInheritableEditingProperties)
            .get(),
          mode);
        return;
      case PropertiesToInclude::EditingPropertiesInEffect:
        MergeStyle(CopyEditingProperties(inlineStyle, EditingPropertiesToInclude::AllEditingProperties).get(),
                   mode);
        return;
      case PropertiesToInclude::PostLayoutProperties:
        MergeStyle(CopyEditingProperties(inlineStyle, EditingPropertiesToInclude::PostLayoutProperties).get(),
                   mode);
        return;
    }
  }

  namespace
  {
    KRYS_NODISCARD bool ElementMatchesAndPropertyIsNotInInlineStyleDecl(
      const HTMLElementEquivalent &equivalent, const dom::StyledElement &element,
      EditingStyle::CSSPropertyOverrideMode mode, EditingStyle &style) noexcept
    {
      if (!equivalent.Matches(element))
      {
        return false;
      }
      if (mode != EditingStyle::CSSPropertyOverrideMode::OverrideValues
          && equivalent.PropertyExistsInStyle(style))
      {
        return false;
      }

      return !element.InlineStyle()
             || !equivalent.PropertyExistsInStyle(*EditingStyle::Create(element.InlineStyle()));
    }

    KRYS_NODISCARD RefPtr<css::MutableStyleProperties>
      ExtractEditingProperties(const css::StyleProperties *style,
                               EditingStyle::PropertiesToInclude propertiesToInclude) noexcept
    {
      if (!style)
      {
        return nullptr;
      }

      switch (propertiesToInclude)
      {
        case EditingStyle::PropertiesToInclude::OnlyEditingInheritableProperties:
          return CopyEditingProperties(style, EditingPropertiesToInclude::OnlyInheritableEditingProperties);
        case EditingStyle::PropertiesToInclude::PostLayoutProperties:
          return CopyEditingProperties(style, EditingPropertiesToInclude::PostLayoutProperties);
        case EditingStyle::PropertiesToInclude::AllProperties:
        case EditingStyle::PropertiesToInclude::EditingPropertiesInEffect: break;
      }
      return CopyEditingProperties(style, EditingPropertiesToInclude::AllEditingProperties);
    }
  }

  void EditingStyle::MergeInlineAndImplicitStyleOfElement(
    dom::StyledElement &element, CSSPropertyOverrideMode mode, PropertiesToInclude propertiesToInclude,
    StandardFontFamilySerializationMode standardFontFamilySerializationMode) noexcept
  {
    auto styleFromRules = EditingStyle::Create();
    styleFromRules->MergeStyleFromRulesForSerialization(element, standardFontFamilySerializationMode);

    if (auto *inlineStyle = element.InlineStyle())
    {
      (void)styleFromRules->_mutableStyle->MergeAndOverrideOnConflict(*inlineStyle);
    }

    styleFromRules->_mutableStyle =
      ExtractEditingProperties(styleFromRules->_mutableStyle.get(), propertiesToInclude);
    MergeStyle(styleFromRules->_mutableStyle.get(), mode);

    for (auto *equivalent : HTMLElementEquivalents())
    {
      if (ElementMatchesAndPropertyIsNotInInlineStyleDecl(*equivalent, element, mode, *this))
      {
        equivalent->AddToStyle(&element, this);
      }
    }

    for (auto *equivalent : HTMLAttributeEquivalents())
    {
      if (equivalent->AttributeName() == u8"dir")
      {
        continue; // We don't want to include directionality
      }
      if (ElementMatchesAndPropertyIsNotInInlineStyleDecl(*equivalent, element, mode, *this))
      {
        equivalent->AddToStyle(&element, this);
      }
    }
  }

  Ref<EditingStyle> EditingStyle::WrappingStyleForSerialization(
    dom::Node &context, bool shouldAnnotate,
    StandardFontFamilySerializationMode standardFontFamilySerializationMode) noexcept
  {
    if (shouldAnnotate)
    {
      auto wrappingStyle = EditingStyle::Create(&context, PropertiesToInclude::EditingPropertiesInEffect);

      // Styles that Mail blockquotes contribute should only be placed on the Mail blockquote,
      // to help us differentiate those styles from ones that the user has applied.
      // This helps us get the color of content pasted into blockquotes right.
      wrappingStyle->RemoveStyleAddedByNode(
        EnclosingNodeOfType(FirstPositionInOrBeforeNode(&context), IsMailBlockquote,
                            EditingBoundaryCrossingRule::CanCrossEditingBoundary)
          .get());

      // Call CollapseTextDecorationProperties first or otherwise it'll copy the value over from in-effect to
      // text-decorations.
      wrappingStyle->CollapseTextDecorationProperties();

      return wrappingStyle;
    }

    auto wrappingStyle = EditingStyle::Create();

    // When not annotating for interchange, we only preserve inline style declarations.
    for (auto *node = &context; node && !Is<dom::Document>(*node); node = node->ParentNode())
    {
      if (auto *element = DynamicDowncast<dom::StyledElement>(*node); element && !IsMailBlockquote(*element))
      {
        wrappingStyle->MergeInlineAndImplicitStyleOfElement(
          *element, CSSPropertyOverrideMode::DoNotOverrideValues,
          PropertiesToInclude::EditingPropertiesInEffect, standardFontFamilySerializationMode);
      }
    }

    return wrappingStyle;
  }

  namespace
  {
    void MergeTextDecorationValues(css::ValueListBuilder &mergedValue,
                                   const css::ValueList &valueToMerge) noexcept
    {
      if (valueToMerge.HasValue(ValueId::Underline) && !Contains(mergedValue, ValueId::Underline))
      {
        mergedValue.push_back(css::PrimitiveValue::Create(ValueId::Underline));
      }

      if (valueToMerge.HasValue(ValueId::LineThrough) && !Contains(mergedValue, ValueId::LineThrough))
      {
        mergedValue.push_back(css::PrimitiveValue::Create(ValueId::LineThrough));
      }
    }
  }

  void EditingStyle::MergeStyle(const css::StyleProperties *style, CSSPropertyOverrideMode mode) noexcept
  {
    if (!style)
    {
      return;
    }

    if (!_mutableStyle)
    {
      _mutableStyle = style->MutableCopy();
      return;
    }

    auto &mutableStyle = *_mutableStyle;
    for (auto property : *style)
    {
      auto value = mutableStyle.GetPropertyCSSValue(property.Id());

      // text decorations never override values.
      if ((property.Id() == PropertyId::TextDecorationLine
           || property.Id() == PropertyId::WebkitTextDecorationsInEffect)
          && value)
      {
        if (auto *propertyValueList = DynamicDowncast<css::ValueList>(*property.Value()))
        {
          if (auto *valueList = DynamicDowncast<css::ValueList>(*value))
          {
            auto newValue = valueList->CopyValues();
            MergeTextDecorationValues(newValue, *propertyValueList);
            auto isImportant = css::IsImportant(property.IsImportant());
            (void)mutableStyle.SetProperty(property.Id(),
                                           css::ValueList::CreateSpaceSeparated(krys::move(newValue))
                                             ->CssText(css::DefaultSerialisationContext()),
                                           isImportant);
            continue;
          }
          value = nullptr; // text-decoration: none is equivalent to not having the property.
        }
      }

      if (mode == CSSPropertyOverrideMode::OverrideValues
          || (mode == CSSPropertyOverrideMode::DoNotOverrideValues && !value))
      {
        mutableStyle.SetProperty(property.Id(), property.ProtectedValue(),
                                 css::IsImportant(property.IsImportant()));
      }
    }

    float oldFontSizeDelta = _fontSizeDelta;
    ExtractFontSizeDelta();
    _fontSizeDelta += oldFontSizeDelta;
  }

  namespace
  {
    KRYS_NODISCARD Ref<css::MutableStyleProperties>
      StyleFromMatchedRulesForElement(dom::Element &element, uint32 rulesToInclude) noexcept
    {
      auto style = css::MutableStyleProperties::Create();

      if (!element.IsConnected())
      {
        return style;
      }

      for (auto &matchedRule : element.StyleResolver().StyleRulesForElement(&element, rulesToInclude))
      {
        (void)style->MergeAndOverrideOnConflict(matchedRule->Properties());
      }

      return style;
    }
  }

  void EditingStyle::MergeStyleFromRules(dom::StyledElement &element) noexcept
  {
    auto styleFromMatchedRules = StyleFromMatchedRulesForElement(element, style::Resolver::AuthorCSSRules);
    // Styles from the inline style declaration, held in the variable "style", take precedence
    // over those from matched rules.
    if (_mutableStyle)
    {
      (void)styleFromMatchedRules->MergeAndOverrideOnConflict(*_mutableStyle);
    }

    Clear();
    _mutableStyle = krys::move(styleFromMatchedRules);
  }

  namespace
  {
    KRYS_NODISCARD dom::DOMString LoneFontFamilyName(const css::Value &value) noexcept
    {
      if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
      {
        return primitiveValue->StringValue();
      }
      auto *list = DynamicDowncast<css::ValueList>(value);
      if (!list || list->Length() != 1)
      {
        return {};
      }
      auto *item = DynamicDowncast<css::PrimitiveValue>(list->Item(0));
      return item ? item->StringValue() : dom::DOMString();
    }
  }

  void EditingStyle::MergeStyleFromRulesForSerialization(
    dom::StyledElement &element,
    StandardFontFamilySerializationMode standardFontFamilySerializationMode) noexcept
  {
    MergeStyleFromRules(element);

    // The property value, if it's a percentage, may not reflect the actual computed value.
    // For example: style="height: 1%; overflow: visible;" in quirksmode
    // FIXME: There are others like this.
    auto fromComputedStyle = css::MutableStyleProperties::Create();
    style::Extractor computedStyle(&element);
    auto &mutableStyle = *_mutableStyle;
    bool shouldRemoveFontFamily = false;
    {
      for (auto property : mutableStyle)
      {
        const auto &value = *property.Value();
        if (property.Id() == PropertyId::FontFamily)
        {
          auto familyName = LoneFontFamilyName(value);
          if (FontCache::IsSystemFontForbiddenForEditing(familyName)
              || (standardFontFamilySerializationMode == StandardFontFamilySerializationMode::Strip
                  && familyName == css::WebKitFontFamilyNames::StandardFamily().View()))
          {
            shouldRemoveFontFamily = true;
          }
          continue;
        }
        auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value);
        if (!primitiveValue)
        {
          continue;
        }
        if (primitiveValue->IsPercentage())
        {
          if (auto computedPropertyValue = computedStyle.PropertyValue(property.Id()))
          {
            (void)fromComputedStyle->AddParsedProperty(
              css::Property(property.Id(), ShareRef(*computedPropertyValue)));
          }
        }
      }
    }
    if (shouldRemoveFontFamily)
    {
      (void)mutableStyle.RemoveProperty(PropertyId::FontFamily);
      (void)fromComputedStyle->RemoveProperty(PropertyId::FontFamily);
    }
    (void)mutableStyle.MergeAndOverrideOnConflict(*fromComputedStyle);
  }

  namespace
  {
    void RemovePropertiesInStyle(css::MutableStyleProperties &styleToRemovePropertiesFrom,
                                 css::MutableStyleProperties &style) noexcept
    {
      List<PropertyId> properties;
      for (auto property : style)
      {
        properties.push_back(property.Id());
      }
      (void)styleToRemovePropertiesFrom.RemoveProperties(Span<const PropertyId>(properties));
    }

  }

  Ref<css::MutableStyleProperties>
    EditingStyle::RemoveInlineStyleRedundantDueToMatchedRules(dom::StyledElement &element) noexcept
  {
    auto styleFromMatchedRules =
      StyleFromMatchedRulesForElement(element, style::Resolver::AllButEmptyCSSRules);
    if (!styleFromMatchedRules->empty())
    {
      _mutableStyle = GetPropertiesNotIn(*_mutableStyle, *styleFromMatchedRules);
    }
    return styleFromMatchedRules;
  }

  void EditingStyle::RemoveStyleInContextNotOverridenByMatchedRules(
    dom::StyledElement &element, dom::Node *context,
    css::MutableStyleProperties &styleFromMatchedRules) noexcept
  {
    auto computedStyle = EditingStyle::Create(context, PropertiesToInclude::EditingPropertiesInEffect);
    if (RefPtr<css::MutableStyleProperties> computedStyleMutableStyle = computedStyle->_mutableStyle)
    {
      if (!computedStyleMutableStyle->GetPropertyCSSValue(PropertyId::BackgroundColor))
      {
        (void)computedStyleMutableStyle->SetProperty(PropertyId::BackgroundColor, ValueId::Transparent);
      }

      // If white-space differs from context, do not remove white-space longhand values.
      // They are necessary for reconstructing the corresponding white-space shorthand value.
      auto &mutableStyle = *_mutableStyle;
      auto whiteSpaceCollapse = mutableStyle.GetPropertyCSSValue(PropertyId::WhiteSpaceCollapse);
      auto contextWhiteSpaceCollapse =
        computedStyleMutableStyle->GetPropertyCSSValue(PropertyId::WhiteSpaceCollapse);

      auto textWrapMode = mutableStyle.GetPropertyCSSValue(PropertyId::TextWrapMode);
      auto contextTextWrapMode = computedStyleMutableStyle->GetPropertyCSSValue(PropertyId::TextWrapMode);

      if (whiteSpaceCollapse != contextWhiteSpaceCollapse || textWrapMode != contextTextWrapMode)
      {
        (void)computedStyleMutableStyle->RemoveProperty(PropertyId::WhiteSpaceCollapse);
        (void)computedStyleMutableStyle->RemoveProperty(PropertyId::TextWrapMode);
      }

      RefPtr<EditingStyle> computedStyleOfElement;
      auto replaceSpecifiedValueWithComputedValue = [&](const PropertyId id)
      {
        if (!mutableStyle.FindPropertyIndex(id))
        {
          return;
        }
        if (!computedStyleOfElement)
        {
          computedStyleOfElement =
            EditingStyle::Create(&element, PropertiesToInclude::EditingPropertiesInEffect);
        }

        if (!computedStyleOfElement->_mutableStyle)
        {
          return;
        }

        auto computedValue = computedStyleOfElement->_mutableStyle->GetPropertyValue(id);
        if (computedValue.empty())
        {
          return;
        }

        (void)mutableStyle.SetProperty(id, computedValue);
      };
      auto replaceSemanticColorWithComputedValue = [&](const PropertyId id)
      {
        auto color = mutableStyle.PropertyAsColor(id);
        if (!color || (color->IsVisible() && !color->IsSemantic()))
        {
          return;
        }
        replaceSpecifiedValueWithComputedValue(id);
      };

      // Replace semantic color identifiers with RGB values so that comparsions in GetPropertiesNotIn below
      // would work.
      replaceSemanticColorWithComputedValue(PropertyId::Color);
      replaceSemanticColorWithComputedValue(PropertyId::CaretColor);
      replaceSemanticColorWithComputedValue(PropertyId::BackgroundColor);

      // This avoid us from comparing non-resolved css-wide keywords (like initial) to resolved values (like
      // text-decoration-style: solid) This will make sure GetPropertiesNotIn below would work, just like the
      // semantic example above.
      replaceSpecifiedValueWithComputedValue(PropertyId::TextDecorationLine);
      replaceSpecifiedValueWithComputedValue(PropertyId::TextDecorationThickness);
      replaceSpecifiedValueWithComputedValue(PropertyId::TextDecorationStyle);
      replaceSpecifiedValueWithComputedValue(PropertyId::TextDecorationColor);

      RemovePropertiesInStyle(*computedStyleMutableStyle, styleFromMatchedRules);
      _mutableStyle = GetPropertiesNotIn(mutableStyle, *computedStyleMutableStyle);
    }
  }

  void EditingStyle::RemoveDisplayPropertyFromSpanStyleIfRedundant(
    dom::StyledElement &element, css::MutableStyleProperties &styleFromMatchedRules) noexcept
  {
    // If this element is a span and has display: inline or float: none, remove them unless they are
    // overridden by rules. These rules are added by serialization code to wrap text nodes.
    if (!IsStyleSpanOrSpanWithOnlyStyleAttribute(element))
    {
      return;
    }

    auto &mutableStyle = *_mutableStyle;
    if (!styleFromMatchedRules.GetPropertyCSSValue(PropertyId::Display)
        && IdentifierForStyleProperty(mutableStyle, PropertyId::Display) == ValueId::Inline)
    {
      (void)mutableStyle.RemoveProperty(PropertyId::Display);
    }
    if (!styleFromMatchedRules.GetPropertyCSSValue(PropertyId::Float)
        && IdentifierForStyleProperty(mutableStyle, PropertyId::Float) == ValueId::None)
    {
      (void)mutableStyle.RemoveProperty(PropertyId::Float);
    }
  }

  void EditingStyle::RemoveStyleFromRulesAndContext(dom::StyledElement &element, dom::Node *context) noexcept
  {
    if (!_mutableStyle)
    {
      return;
    }

    auto styleFromMatchedRules = RemoveInlineStyleRedundantDueToMatchedRules(element);
    RemoveStyleInContextNotOverridenByMatchedRules(element, context, *styleFromMatchedRules);
    RemoveDisplayPropertyFromSpanStyleIfRedundant(element, *styleFromMatchedRules);
  }

  void EditingStyle::RemovePropertiesInElementDefaultStyle(dom::Element &element) noexcept
  {
    if (!_mutableStyle || _mutableStyle->empty())
    {
      return;
    }

    auto defaultStyle = StyleFromMatchedRulesForElement(element, style::Resolver::UAAndUserCSSRules);

    RemovePropertiesInStyle(*_mutableStyle, *defaultStyle);
  }

  template <typename T>
  void EditingStyle::RemoveEquivalentProperties(T &style) noexcept
  {
    List<PropertyId> propertiesToRemove;
    List<PropertyId> alreadyHandled;
    auto &mutableStyle = *_mutableStyle;
    for (auto property : mutableStyle)
    {
      auto id = property.Id();
      if (std::ranges::find(alreadyHandled, id) != alreadyHandled.end())
      {
        continue;
      }
      if (!style.PropertyMatches(property.Id(), property.Value()))
      {
        continue;
      }

      // Only the text-decoration longhands support serializing with the shorthand in all editing properties.
      auto shorthandID = property.ShorthandId();
      if (shorthandID != PropertyId::TextDecoration)
      {
        propertiesToRemove.push_back(property.Id());
        continue;
      }

      // Do not remove equivalent properties when they share a shorthand with non-equivalent ones, and the
      // removal would prevent them from being serialized with the shorthand.
      if (mutableStyle.GetPropertyValue(shorthandID).empty())
      {
        propertiesToRemove.push_back(property.Id());
        continue;
      }
      auto shorthand = css::ShorthandForProperty(shorthandID);
      bool canRemoveAllLonghands = true;
      for (auto longhandID : shorthand)
      {
        if (id != longhandID
            && !style.PropertyMatches(longhandID, mutableStyle.GetPropertyCSSValue(longhandID).get()))
        {
          canRemoveAllLonghands = false;
          break;
        }
      }
      for (auto longhandID : shorthand)
      {
        if (canRemoveAllLonghands)
        {
          propertiesToRemove.push_back(longhandID);
        }
        alreadyHandled.push_back(longhandID);
      }
    }
    (void)mutableStyle.RemoveProperties(Span<const PropertyId>(propertiesToRemove));
  }

  void EditingStyle::ForceDisplayInline() noexcept
  {
    if (!_mutableStyle)
    {
      _mutableStyle = css::MutableStyleProperties::Create();
    }
    (void)_mutableStyle->SetProperty(PropertyId::Display, ValueId::Inline, css::IsImportant(true));
  }

  void EditingStyle::AddDisplayContents() noexcept
  {
    if (!_mutableStyle)
    {
      _mutableStyle = css::MutableStyleProperties::Create();
    }
    (void)_mutableStyle->SetProperty(PropertyId::Display, ValueId::Contents);
  }

  bool EditingStyle::ConvertPositionStyle() noexcept
  {
    if (!_mutableStyle)
    {
      return false;
    }

    auto &mutableStyle = *_mutableStyle;
    auto sticky = css::PrimitiveValue::Create(ValueId::Sticky);
    if (mutableStyle.PropertyMatches(PropertyId::Position, sticky.get()))
    {
      mutableStyle.SetProperty(PropertyId::Position, css::PrimitiveValue::Create(ValueId::Static),
                               css::IsImportant(mutableStyle.PropertyIsImportant(PropertyId::Position)));
      return false;
    }
    auto fixed = css::PrimitiveValue::Create(ValueId::Fixed);
    if (mutableStyle.PropertyMatches(PropertyId::Position, fixed.get()))
    {
      mutableStyle.SetProperty(PropertyId::Position, css::PrimitiveValue::Create(ValueId::Absolute),
                               css::IsImportant(mutableStyle.PropertyIsImportant(PropertyId::Position)));
      return true;
    }
    auto absolute = css::PrimitiveValue::Create(ValueId::Absolute);
    if (mutableStyle.PropertyMatches(PropertyId::Position, absolute.get()))
    {
      return true;
    }
    return false;
  }

  bool EditingStyle::IsFloating() noexcept
  {
    RefPtr<css::Value> v = _mutableStyle->GetPropertyCSSValue(PropertyId::Float);
    auto noneValue = css::PrimitiveValue::Create(ValueId::None);
    return v && !v->Equals(*noneValue);
  }

  int32 EditingStyle::LegacyFontSize(dom::Document &document) const noexcept
  {
    auto cssValue = _mutableStyle->GetPropertyCSSValue(PropertyId::FontSize);
    auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(cssValue.get());
    if (!primitiveValue)
    {
      return 0;
    }
    return LegacyFontSizeFromCSSValue(document, primitiveValue, _shouldUseFixedDefaultFontSize,
                                      LegacyFontSizeMode::AlwaysUseLegacyFontSize);
  }

  bool EditingStyle::HasStyle(PropertyId propertyID, const dom::DOMString &value) noexcept
  {
    return EditingStyle::Create(propertyID, value)->TriStateOfStyle(this) != TriState::False;
  }

  namespace
  {
    template <typename T>
    KRYS_NODISCARD bool FontWeightIsBold(T &style) noexcept
    {
      RefPtr<css::Value> fontWeight = ExtractPropertyValue(style, PropertyId::FontWeight);
      return fontWeight && FontWeightValueIsBold(*fontWeight);
    }
  }

  bool EditingStyle::FontWeightIsBold() noexcept
  {
    if (!_mutableStyle)
    {
      return false;
    }

    return boo::FontWeightIsBold(*_mutableStyle);
  }

  bool EditingStyle::FontStyleIsItalic() noexcept
  {
    if (!_mutableStyle)
    {
      return false;
    }
    RefPtr<css::Value> fontStyle = ExtractPropertyValue(*_mutableStyle, PropertyId::FontStyle);
    if (!fontStyle)
    {
      return false;
    }

    auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(*fontStyle);
    auto keyword = primitiveValue ? primitiveValue->ValueId() : ValueId::Oblique;
    return keyword == ValueId::Oblique || keyword == ValueId::Italic;
  }

  bool EditingStyle::WebkitTextDecorationsInEffectIsUnderline() noexcept
  {
    if (!_mutableStyle)
    {
      return false;
    }
    auto value = ExtractPropertyValue(*_mutableStyle, PropertyId::WebkitTextDecorationsInEffect);
    auto *textDecorations = DynamicDowncast<css::ValueList>(value.get());
    if (!textDecorations)
    {
      return false;
    }
    return textDecorations->HasValue(ValueId::Underline);
  }

  RefPtr<EditingStyle> EditingStyle::StyleAtSelectionStart(const VisibleSelection &selection,
                                                           bool shouldUseBackgroundColorInEffect,
                                                           PropertiesToInclude propertiesToInclude) noexcept
  {
    if (selection.IsNone())
    {
      return nullptr;
    }

    dom::Position position = AdjustedSelectionStartForStyleComputation(selection);

    // If the pos is at the end of a text node, then this node is not fully selected.
    // Move it to the next deep equivalent position to avoid removing the style from this node.
    // e.g. if pos was at Position("hello", 5) in <b>hello<div>world</div></b>, we want Position("world", 0)
    // instead. We only do this for range because caret at Position("hello", 5) in <b>hello</b>world should
    // give you font-weight: bold.
    if (auto *text = DynamicDowncast<dom::Text>(position.GetContainerNode());
        text && selection.IsRange()
        && static_cast<size_t>(position.ComputeOffsetInContainerNode()) == text->Length())
    {
      position = NextVisuallyDistinctCandidate(position);
    }

    RefPtr<dom::Element> element = position.AnchorElementAncestor();
    if (!element)
    {
      return nullptr;
    }

    auto style = EditingStyle::Create(element.get(), propertiesToInclude);
    style->MergeTypingStyle(element->NodeDocument());

    // If background color is transparent, traverse parent nodes until we hit a different value or document
    // root Also, if the selection is a range, ignore the background color at the start of selection, and find
    // the background color of the common ancestor.
    if (shouldUseBackgroundColorInEffect
        && (selection.IsRange() || HasTransparentBackgroundColor(style->_mutableStyle.get())))
    {
      if (auto range = selection.ToNormalizedRange())
      {
        if (RefPtr<css::Value> value = BackgroundColorInEffect(dom::CommonInclusiveAncestor(*range)))
        {
          style->SetProperty(PropertyId::BackgroundColor, value->CssText(css::DefaultSerialisationContext()));
        }
      }
    }

    return style;
  }

  WritingDirection EditingStyle::TextDirectionForSelection(const VisibleSelection &selection,
                                                           EditingStyle *typingStyle,
                                                           bool &hasNestedOrMultipleEmbeddings) noexcept
  {
    hasNestedOrMultipleEmbeddings = true;

    if (selection.IsNone())
    {
      return WritingDirection::Natural;
    }

    dom::Position position = selection.Start().Downstream();

    RefPtr<dom::Node> node = ShareRefPtr(position.DeprecatedNode());
    if (!node)
    {
      return WritingDirection::Natural;
    }

    dom::Position end;
    if (selection.IsRange())
    {
      end = selection.End().Upstream();
      auto range = dom::MakeSimpleRange(position, end);
      for (auto &intersectingNode : dom::IntersectingNodes(*range))
      {
        if (!Is<dom::StyledElement>(intersectingNode))
        {
          continue;
        }
        auto value = style::Extractor(&intersectingNode).PropertyValue(PropertyId::UnicodeBidi);
        auto valueID = value ? css::LonghandValueId(PropertyId::UnicodeBidi, *value) : ValueId::Invalid;
        if (valueID == ValueId::Embed || valueID == ValueId::BidiOverride)
        {
          return WritingDirection::Natural;
        }
      }
    }

    if (selection.IsCaret())
    {
      if (typingStyle)
      {
        if (auto direction = typingStyle->TextDirection())
        {
          hasNestedOrMultipleEmbeddings = false;
          return *direction;
        }
      }
      node = ShareRefPtr(selection.VisibleStart().DeepEquivalent().DeprecatedNode());
    }

    // The selection is either a caret with no typing attributes or a range in which no embedding is added, so
    // just use the start position to decide.
    auto block = EnclosingBlock(node);
    auto foundDirection = WritingDirection::Natural;

    for (; node != block; node = ShareRefPtr<dom::Node>(node->ParentNode()))
    {
      if (!Is<dom::StyledElement>(*node))
      {
        continue;
      }

      style::Extractor computedStyle(node.get());
      RefPtr<css::Value> unicodeBidi = computedStyle.PropertyValue(PropertyId::UnicodeBidi);
      auto *unicodeBidiPrimitive = DynamicDowncast<css::PrimitiveValue>(unicodeBidi.get());
      if (!unicodeBidiPrimitive)
      {
        continue;
      }

      auto unicodeBidiValue = unicodeBidiPrimitive->ValueId();
      if (unicodeBidiValue == ValueId::Normal)
      {
        continue;
      }

      if (unicodeBidiValue == ValueId::BidiOverride)
      {
        return WritingDirection::Natural;
      }

      krys_debug_assert(IsEmbedOrIsolate(unicodeBidiValue));
      RefPtr<css::Value> direction = computedStyle.PropertyValue(PropertyId::Direction);
      auto *directionPrimitive = DynamicDowncast<css::PrimitiveValue>(direction.get());
      if (!directionPrimitive)
      {
        continue;
      }

      auto directionValue = directionPrimitive->ValueId();
      if (directionValue != ValueId::Ltr && directionValue != ValueId::Rtl)
      {
        continue;
      }

      if (foundDirection != WritingDirection::Natural)
      {
        return WritingDirection::Natural;
      }

      // In the range case, make sure that the embedding element persists until the end of the range.
      if (selection.IsRange() && !dom::TreeQueries::IsDescendant(*end.DeprecatedNode(), *node))
      {
        return WritingDirection::Natural;
      }

      foundDirection =
        directionValue == ValueId::Ltr ? WritingDirection::LeftToRight : WritingDirection::RightToLeft;
    }
    hasNestedOrMultipleEmbeddings = false;
    return foundDirection;
  }

  // Colours are only inverted for -apple-color-filter, which isn't supported, so the style never changes.
  Ref<EditingStyle> EditingStyle::InverseTransformColorIfNeeded(dom::Element &element) noexcept
  {
    (void)element;
    return ShareRef(*this);
  }

  namespace
  {
    void ReconcileTextDecorationProperties(css::MutableStyleProperties &style) noexcept
    {
      auto textDecorationsInEffect = style.GetPropertyCSSValue(PropertyId::WebkitTextDecorationsInEffect);
      auto textDecoration = style.GetPropertyCSSValue(PropertyId::TextDecorationLine);
      // We shouldn't have both text-decoration and -webkit-text-decorations-in-effect because that wouldn't
      // make sense.
      krys_debug_assert(!textDecorationsInEffect || !textDecoration);
      if (textDecorationsInEffect)
      {
        (void)style.SetProperty(PropertyId::TextDecoration,
                                textDecorationsInEffect->CssText(css::DefaultSerialisationContext()));
        (void)style.RemoveProperty(PropertyId::WebkitTextDecorationsInEffect);
        textDecoration = textDecorationsInEffect;
      }

      // If text-decoration is set to "none", remove the property because we don't want to add redundant
      // "text-decoration: none".
      if (textDecoration && !Is<css::ValueList>(*textDecoration))
      {
        (void)style.RemoveProperty(PropertyId::TextDecoration);
      }
    }
  }

  // FIXME: Styling with CSS (the editor's styleWithCSS), with rich editing commands; until then elements
  // express the styles they can, as is upstream's default.
  StyleChange::StyleChange(EditingStyle *style, const dom::Position &position) noexcept
  {
    auto *document = position.DeprecatedNode() ? &position.DeprecatedNode()->NodeDocument() : nullptr;
    if (!style || style->IsEmpty() || !document || !document->Frame())
    {
      return;
    }

    auto *node = position.GetContainerNode();
    if (!node)
    {
      return;
    }

    style::Extractor computedStyle(node);

    // FIXME: take care of background-color in effect
    auto mutableStyle = style->Style() ? GetPropertiesNotIn(*style->Style(), computedStyle)
                                       : css::MutableStyleProperties::Create();

    ReconcileTextDecorationProperties(*mutableStyle);
    constexpr bool shouldStyleWithCSS = false;
    if (!shouldStyleWithCSS)
    {
      ExtractTextStyles(*document, *mutableStyle, computedStyle.UseFixedFontDefaultSize());
    }

    bool shouldAddUnderline = style->UnderlineChange() == TextDecorationChange::Add;
    bool shouldAddStrikeThrough = style->StrikeThroughChange() == TextDecorationChange::Add;
    if (shouldAddUnderline || shouldAddStrikeThrough)
    {
      RefPtr<css::Value> value = computedStyle.PropertyValue(PropertyId::WebkitTextDecorationsInEffect);
      if (!value || !Is<css::ValueList>(*value))
      {
        value = computedStyle.PropertyValue(PropertyId::TextDecorationLine);
      }

      css::ValueListBuilder valueList;
      if (auto *list = DynamicDowncast<css::ValueList>(value.get()))
      {
        valueList = list->CopyValues();
      }

      bool hasUnderline = Contains(valueList, ValueId::Underline);
      bool hasLineThrough = Contains(valueList, ValueId::LineThrough);

      if (shouldStyleWithCSS)
      {
        if (shouldAddUnderline && !hasUnderline)
        {
          valueList.push_back(css::PrimitiveValue::Create(ValueId::Underline));
        }
        if (shouldAddStrikeThrough && !hasLineThrough)
        {
          valueList.push_back(css::PrimitiveValue::Create(ValueId::LineThrough));
        }
        (void)mutableStyle->SetProperty(PropertyId::TextDecoration,
                                        css::ValueList::CreateSpaceSeparated(krys::move(valueList))
                                          ->CssText(css::DefaultSerialisationContext()));
      }
      else
      {
        _applyUnderline = shouldAddUnderline && !hasUnderline;
        _applyLineThrough = shouldAddStrikeThrough && !hasLineThrough;
      }
    }

    // Changing the whitespace style in a tab span would collapse the tab into a space.
    if (auto *positionDeprecatedNode = position.DeprecatedNode())
    {
      if (ParentTabSpanNode(positionDeprecatedNode) || TabSpanNode(positionDeprecatedNode))
      {
        (void)mutableStyle->RemoveProperty(PropertyId::WhiteSpaceCollapse);
        (void)mutableStyle->RemoveProperty(PropertyId::TextWrapMode);
      }
    }

    // If unicode-bidi is present in mutableStyle and direction is not, then add direction to mutableStyle.
    // FIXME: Shouldn't this be done in GetPropertiesNotIn?
    if (mutableStyle->GetPropertyCSSValue(PropertyId::UnicodeBidi)
        && !style->Style()->GetPropertyCSSValue(PropertyId::Direction))
    {
      (void)mutableStyle->SetProperty(PropertyId::Direction,
                                      style->Style()->GetPropertyValue(PropertyId::Direction));
    }

    if (!mutableStyle->empty())
    {
      _cssStyle = krys::move(mutableStyle);
    }
  }

  StyleChange::~StyleChange() noexcept = default;

  bool StyleChange::operator==(const StyleChange &other) const noexcept
  {
    if (_applyBold != other._applyBold || _applyItalic != other._applyItalic
        || _applyUnderline != other._applyUnderline || _applyLineThrough != other._applyLineThrough
        || _applySubscript != other._applySubscript || _applySuperscript != other._applySuperscript
        || _applyFontColor != other._applyFontColor || _applyFontFace != other._applyFontFace
        || _applyFontSize != other._applyFontSize)
    {
      return false;
    }

    return (!_cssStyle && !other._cssStyle)
           || (_cssStyle && other._cssStyle
               && _cssStyle->AsText(css::DefaultSerialisationContext())
                    == other._cssStyle->AsText(css::DefaultSerialisationContext()));
  }

  namespace
  {
    void SetTextDecorationProperty(css::MutableStyleProperties &style,
                                   const css::ValueList &newTextDecoration, PropertyId propertyID) noexcept
    {
      if (newTextDecoration.Length())
      {
        auto isImportant = css::IsImportant(style.PropertyIsImportant(
          propertyID == PropertyId::TextDecoration ? PropertyId::TextDecorationLine : propertyID));
        (void)style.SetProperty(propertyID, newTextDecoration.CssText(css::DefaultSerialisationContext()),
                                isImportant);
      }
      else
      {
        // text-decoration: none is redundant since it does not remove any text decorations.
        (void)style.RemoveProperty(propertyID);
      }
    }
  }

  void StyleChange::ExtractTextStyles(dom::Document &document, css::MutableStyleProperties &style,
                                      bool shouldUseFixedFontDefaultSize) noexcept
  {
    if (IdentifierForStyleProperty(style, PropertyId::FontWeight) == ValueId::Bold)
    {
      (void)style.RemoveProperty(PropertyId::FontWeight);
      _applyBold = true;
    }

    auto fontStyle = IdentifierForStyleProperty(style, PropertyId::FontStyle);
    if (fontStyle == ValueId::Italic)
    {
      (void)style.RemoveProperty(PropertyId::FontStyle);
      _applyItalic = true;
    }

    // Assuming ReconcileTextDecorationProperties has been called, there should not be
    // -webkit-text-decorations-in-effect Furthermore, text-decoration: none has been trimmed so that
    // text-decoration property is always a ValueList.
    auto textDecorationValue = style.GetPropertyCSSValue(PropertyId::TextDecorationLine);
    if (auto *textDecoration = DynamicDowncast<css::ValueList>(textDecorationValue.get()))
    {
      auto newTextDecoration = textDecoration->CopyValues();
      if (RemoveAll(newTextDecoration, ValueId::Underline))
      {
        _applyUnderline = true;
      }
      if (RemoveAll(newTextDecoration, ValueId::LineThrough))
      {
        _applyLineThrough = true;
      }

      // If trimTextDecorations, delete underline and line-through
      SetTextDecorationProperty(style, *css::ValueList::CreateSpaceSeparated(krys::move(newTextDecoration)),
                                PropertyId::TextDecoration);
    }

    auto verticalAlign = IdentifierForStyleProperty(style, PropertyId::VerticalAlign);
    switch (verticalAlign)
    {
      case ValueId::Sub:
        (void)style.RemoveProperty(PropertyId::VerticalAlign);
        _applySubscript = true;
        break;
      case ValueId::Super:
        (void)style.RemoveProperty(PropertyId::VerticalAlign);
        _applySuperscript = true;
        break;
      default: break;
    }

    if (style.GetPropertyCSSValue(PropertyId::Color))
    {
      auto color = TextColorFromStyle(style);
      if (color.IsOpaque())
      {
        _applyFontColor = SerialisationForHTML(color);
        (void)style.RemoveProperty(PropertyId::Color);
      }
    }

    // Remove quotes for Outlook 2007 compatibility.
    _applyFontFace = style.GetPropertyValue(PropertyId::FontFamily);
    std::erase(_applyFontFace, u8'"');
    (void)style.RemoveProperty(PropertyId::FontFamily);

    if (RefPtr<css::Value> fontSize = style.GetPropertyCSSValue(PropertyId::FontSize))
    {
      auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(*fontSize);
      if (!primitiveValue)
      {
        (void)style.RemoveProperty(PropertyId::FontSize); // Can't make sense of the number. Put no font size.
      }
      else if (int32 legacyFontSize =
                 LegacyFontSizeFromCSSValue(document, primitiveValue, shouldUseFixedFontDefaultSize,
                                            LegacyFontSizeMode::UseLegacyFontSizeOnlyIfPixelValuesMatch))
      {
        _applyFontSize =
          dom::DOMString(reinterpret_cast<const char8_t *>(std::to_string(legacyFontSize).c_str()));
        (void)style.RemoveProperty(PropertyId::FontSize);
      }
    }
  }

  namespace
  {
    void DiffTextDecorations(css::MutableStyleProperties &style, PropertyId propertyID,
                             const css::Value *refTextDecoration) noexcept
    {
      auto textDecorationValue = style.GetPropertyCSSValue(propertyID);
      auto *textDecoration = DynamicDowncast<css::ValueList>(textDecorationValue.get());
      auto *refTextDecorationList = DynamicDowncast<css::ValueList>(refTextDecoration);
      if (!textDecoration || !refTextDecorationList)
      {
        return;
      }
      auto newTextDecoration = textDecoration->CopyValues();
      for (uint32 i = 0; i < refTextDecorationList->Length(); ++i)
      {
        (void)RemoveAll(newTextDecoration, *refTextDecorationList->Item(i));
      }
      SetTextDecorationProperty(style, *css::ValueList::CreateSpaceSeparated(krys::move(newTextDecoration)),
                                propertyID);
    }

    template <typename T>
    KRYS_NODISCARD Ref<css::MutableStyleProperties>
      ExtractPropertiesNotIn(css::StyleProperties &styleWithRedundantProperties, T &baseStyle) noexcept
    {
      auto result = EditingStyle::Create(&styleWithRedundantProperties);
      result->RemoveEquivalentProperties(baseStyle);
      krys_debug_assert(result->Style());
      Ref<css::MutableStyleProperties> mutableStyle = ShareRef(*result->Style());

      auto baseTextDecorationsInEffect =
        ExtractPropertyValue(baseStyle, PropertyId::WebkitTextDecorationsInEffect);
      DiffTextDecorations(*mutableStyle, PropertyId::TextDecorationLine, baseTextDecorationsInEffect.get());
      DiffTextDecorations(*mutableStyle, PropertyId::WebkitTextDecorationsInEffect,
                          baseTextDecorationsInEffect.get());

      if (ExtractPropertyValue(baseStyle, PropertyId::FontWeight)
          && FontWeightIsBold(*mutableStyle) == FontWeightIsBold(baseStyle))
      {
        (void)mutableStyle->RemoveProperty(PropertyId::FontWeight);
      }

      if (ExtractPropertyValue(baseStyle, PropertyId::Color)
          && EqualIgnoringSemanticColor(TextColorFromStyle(*mutableStyle), TextColorFromStyle(baseStyle)))
      {
        (void)mutableStyle->RemoveProperty(PropertyId::Color);
      }

      if (ExtractPropertyValue(baseStyle, PropertyId::CaretColor)
          && EqualIgnoringSemanticColor(CaretColorFromStyle(*mutableStyle), CaretColorFromStyle(baseStyle)))
      {
        (void)mutableStyle->RemoveProperty(PropertyId::CaretColor);
      }

      if (ExtractPropertyValue(baseStyle, PropertyId::TextAlign)
          && TextAlignResolvingStartAndEnd(*mutableStyle) == TextAlignResolvingStartAndEnd(baseStyle))
      {
        (void)mutableStyle->RemoveProperty(PropertyId::TextAlign);
      }

      if (ExtractPropertyValue(baseStyle, PropertyId::BackgroundColor)
          && EqualIgnoringSemanticColor(BackgroundColorFromStyle(*mutableStyle),
                                        BackgroundColorFromStyle(baseStyle)))
      {
        (void)mutableStyle->RemoveProperty(PropertyId::BackgroundColor);
      }

      return mutableStyle;
    }

    template <typename T>
    Ref<css::MutableStyleProperties> GetPropertiesNotIn(css::StyleProperties &styleWithRedundantProperties,
                                                        T &baseStyle) noexcept
    {
      return ExtractPropertiesNotIn(styleWithRedundantProperties, baseStyle);
    }

    KRYS_NODISCARD bool IsCSSValueLength(css::PrimitiveValue *value) noexcept
    {
      return value->IsFontIndependentLength();
    }

    int32 LegacyFontSizeFromCSSValue(dom::Document &document, css::PrimitiveValue *value,
                                     bool shouldUseFixedFontDefaultSize, LegacyFontSizeMode mode) noexcept
    {
      if (IsCSSValueLength(value))
      {
        int32 pixelFontSize = value->ResolveAsLengthDeprecated<int32>();
        int32 legacyFontSize =
          style::LegacyFontSizeForPixelSize(pixelFontSize, shouldUseFixedFontDefaultSize, document);
        // Use legacy font size only if pixel value matches exactly to that of legacy font size.
        auto cssPrimitiveEquivalent =
          static_cast<ValueId>(legacyFontSize - 1 + static_cast<int32>(ValueId::XSmall));
        if (mode == LegacyFontSizeMode::AlwaysUseLegacyFontSize
            || style::FontSizeForKeyword(static_cast<uint32>(cssPrimitiveEquivalent),
                                         shouldUseFixedFontDefaultSize, document)
                 == static_cast<float>(pixelFontSize))
        {
          return legacyFontSize;
        }

        return 0;
      }

      if (ValueId::XSmall <= value->ValueId() && value->ValueId() <= ValueId::XxxLarge)
      {
        return static_cast<int32>(value->ValueId()) - static_cast<int32>(ValueId::XSmall) + 1;
      }

      return 0;
    }

    KRYS_NODISCARD bool IsTransparentColorValue(const css::Value *value) noexcept
    {
      if (!value)
      {
        return true;
      }
      if (IsValueID(*value, ValueId::Transparent))
      {
        return true;
      }
      return !CSSValueToColor(value).IsVisible();
    }

    bool HasTransparentBackgroundColor(css::StyleProperties *style) noexcept
    {
      return IsTransparentColorValue(style->GetPropertyCSSValue(PropertyId::BackgroundColor).get());
    }

    RefPtr<css::Value> BackgroundColorInEffect(dom::Node *node) noexcept
    {
      for (auto *ancestor = node; ancestor; ancestor = ancestor->ParentNode())
      {
        auto value = style::Extractor(ancestor).PropertyValue(PropertyId::BackgroundColor);
        if (!IsTransparentColorValue(value.get()))
        {
          return value;
        }
      }
      return nullptr;
    }
  }
}
