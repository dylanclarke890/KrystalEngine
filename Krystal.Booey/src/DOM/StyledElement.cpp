#include "Krystal.Booey/DOM/StyledElement.hpp"
#include "Krystal.Booey/Constants/Attributes.hpp"
#include "Krystal.Booey/CSS/CSSStyleProperties.hpp"
#include "Krystal.Booey/CSS/Parser/Parser.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Properties/ImmutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/StylePropertiesInlines.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/Scope.hpp"

namespace krys::boo::dom
{
  StyledElement::StyledElement(Document &document, const QualifiedName &name, NodeFlags nodeFlags) noexcept
      : Element(document, name, nodeFlags)
  {
  }

  StyledElement::~StyledElement() noexcept = default;

#pragma region ElementCSSInlineStyle

  css::CSSStyleProperties &StyledElement::Style() noexcept
  {
    return EnsureMutableInlineStyle()->EnsureInlineCSSStyleProperties(*this);
  }

#pragma endregion

  const css::StyleProperties *StyledElement::InlineStyle() const noexcept
  {
    return _inlineStyle.get();
  }

  void StyledElement::SynchronizeStyleAttributeInternal() noexcept
  {
    krys_debug_assert(HasNodeFlag(NodeFlags::StyleAttributeIsDirty));
    ClearNodeFlag(NodeFlags::StyleAttributeIsDirty);
    if (const auto *inlineStyle = InlineStyle())
    {
      SetSynchronizedLazyAttribute(Attributes::Style,
                                   inlineStyle->AsText(css::DefaultSerialisationContext()));
    }
  }

  Ref<css::MutableStyleProperties> StyledElement::EnsureMutableInlineStyle() noexcept
  {
    if (!_inlineStyle)
    {
      auto mutableProperties = css::MutableStyleProperties::Create(
        css::StrictToParserMode(IsHTMLElement() && !NodeDocument().InQuirksMode()));
      _inlineStyle = mutableProperties;
      return mutableProperties;
    }
    if (auto *mutableProperties = DynamicDowncast<css::MutableStyleProperties>(*_inlineStyle))
    {
      return ShareRef(*mutableProperties);
    }
    auto mutableProperties = _inlineStyle->MutableCopy();
    _inlineStyle = mutableProperties;
    return mutableProperties;
  }

  void StyledElement::OnAttributeChanged(DOMStringAtom localName, Maybe<DOMStringView> oldValue,
                                         Maybe<DOMStringView> value, DOMStringAtom namespaceURI) noexcept
  {
    Element::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (oldValue != value)
    {
      if (localName == Attributes::Style && namespaceURI == DOMStringAtom::Null())
      {
        StyleAttributeChanged(value);
      }
      else if (HasPresentationalHintsForAttribute(localName, namespaceURI))
      {
        _presentationalHintStyleIsDirty = true;
        InvalidateStyleInternal();
      }
    }
  }

  const css::ImmutableStyleProperties *StyledElement::PresentationalHintStyle() const noexcept
  {
    if (_presentationalHintStyleIsDirty)
    {
      const_cast<StyledElement &>(*this).RebuildPresentationalHintStyle();
    }
    return _presentationalHintStyle.get();
  }

  void StyledElement::InvalidatePresentationalHintStyle() noexcept
  {
    _presentationalHintStyleIsDirty = true;
  }

  void StyledElement::RebuildPresentationalHintStyle() noexcept
  {
    bool isSVG = IsSVGElement();
    auto style = css::MutableStyleProperties::Create(isSVG ? css::ParserMode::SVGAttribute
                                                           : css::ParserMode::HTMLQuirks);
    for (auto &attribute : AttributeList())
    {
      CollectPresentationalHintsForAttribute(attribute->LocalName(), attribute->NamespaceURI(),
                                             attribute->Value(), *style);
    }
    CollectExtraStyleForPresentationalHints(*style);

    _presentationalHintStyleIsDirty = false;
    if (style->empty())
    {
      _presentationalHintStyle = nullptr;
      return;
    }

    auto shouldDeduplicate = [&]
    {
      auto hasNonZeroProperty = [&](css::PropertyId property)
      {
        auto value = style->GetPropertyCSSValue(property);
        auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value.get());
        return primitiveValue && !primitiveValue->IsZero().value_or(false);
      };
      if (isSVG)
      {
        // SVG style tends to have unique x/y properties. Don't spend effort trying to deduplicate them.
        if (hasNonZeroProperty(css::PropertyId::X))
        {
          return false;
        }
        if (hasNonZeroProperty(css::PropertyId::Y))
        {
          return false;
        }
        if (hasNonZeroProperty(css::PropertyId::Cx))
        {
          return false;
        }
        if (hasNonZeroProperty(css::PropertyId::Cy))
        {
          return false;
        }
      }
      return true;
    }();

    _presentationalHintStyle =
      shouldDeduplicate ? style->ImmutableDeduplicatedCopy() : style->ImmutableCopy();
  }

  void StyledElement::AddPropertyToPresentationalHintStyle(css::MutableStyleProperties &style,
                                                           css::PropertyId property,
                                                           css::ValueId identifier) noexcept
  {
    style.SetProperty(property, css::PrimitiveValue::Create(identifier));
  }

  void StyledElement::AddPropertyToPresentationalHintStyle(css::MutableStyleProperties &style,
                                                           css::PropertyId property, double value,
                                                           css::UnitType unit) noexcept
  {
    style.SetProperty(property, css::PrimitiveValue::Create(value, unit));
  }

  void StyledElement::AddPropertyToPresentationalHintStyle(css::MutableStyleProperties &style,
                                                           css::PropertyId property,
                                                           const css::CSSOMString &value) noexcept
  {
    style.SetProperty(property, value, NodeDocument().CssParserContext());
  }

  void StyledElement::AddPropertyToPresentationalHintStyle(css::MutableStyleProperties &style,
                                                           css::PropertyId property,
                                                           Ref<css::Value> &&value) noexcept
  {
    style.SetProperty(property, krys::move(value));
  }

  void StyledElement::SetInlineStyleFromString(DOMStringView newStyleString) noexcept
  {
    // We reconstruct the property set instead of mutating if there is no CSSOM wrapper.
    // This makes wrapperless property sets immutable and so cacheable.
    if (auto *mutableStyleProperties = DynamicDowncast<css::MutableStyleProperties>(_inlineStyle.get()))
    {
      mutableStyleProperties->ParseDeclaration(css::CSSOMString(newStyleString),
                                               css::ParserContext(NodeDocument()));
    }
    else
    {
      _inlineStyle = css::Parser::ParseInlineStyleDeclaration(newStyleString, *this);
    }
  }

  void StyledElement::StyleAttributeChanged(Maybe<DOMStringView> newStyleString) noexcept
  {
    // SPEC-VIOLATION(CSP): the style attribute's value is parsed without asking the Content Security Policy
    // whether inline styles are allowed.
    if (!newStyleString)
    {
      EnsureMutableInlineStyle()->Clear();
    }
    else
    {
      SetInlineStyleFromString(*newStyleString);
    }

    ClearNodeFlag(NodeFlags::StyleAttributeIsDirty);

    Node::InvalidateStyle(style::Validity::InlineStyleInvalid);
  }

  void StyledElement::DirtyStyleAttribute() noexcept
  {
    SetNodeFlag(NodeFlags::StyleAttributeIsDirty);

    // FIXME: Synchronise immediately (with attribute change invalidation) when the rule sets have selectors
    // for the style attribute, once there's attribute change invalidation.
  }

  void StyledElement::InvalidateStyleAttribute() noexcept
  {
    SetNodeFlag(NodeFlags::StyleAttributeIsDirty);

    // Inline style invalidation optimization does not work if there are selectors targeting the style
    // attribute as some rule may start or stop matching.
    auto selectorsForStyleAttribute =
      style::Scope::ForNode(*this).GetResolver().RuleSets().GetSelectorsForStyleAttribute();
    auto validity = selectorsForStyleAttribute == style::SelectorsForStyleAttribute::None
                      ? style::Validity::InlineStyleInvalid
                      : style::Validity::ElementInvalid;

    Node::InvalidateStyle(validity);

    // FIXME: Invalidate the instances of SVG elements, once there are SVG <use> instances.

    // FIXME: In the rare case of selectors like "[style] ~ div" we need to synchronize immediately to
    // invalidate, once there's attribute change invalidation.
  }

  void StyledElement::InlineStyleChanged() noexcept
  {
    InvalidateStyleAttribute();
  }

  bool StyledElement::SetInlineStyleProperty(css::PropertyId property, css::ValueId identifier,
                                             css::IsImportant important) noexcept
  {
    EnsureMutableInlineStyle()->SetProperty(property, css::PrimitiveValue::Create(identifier), important);
    InlineStyleChanged();
    return true;
  }

  bool StyledElement::SetInlineStyleProperty(css::PropertyId property, css::PropertyId identifier,
                                             css::IsImportant important) noexcept
  {
    EnsureMutableInlineStyle()->SetProperty(property, css::PrimitiveValue::Create(identifier), important);
    InlineStyleChanged();
    return true;
  }

  bool StyledElement::SetInlineStyleProperty(css::PropertyId property, double value, css::UnitType unit,
                                             css::IsImportant important) noexcept
  {
    EnsureMutableInlineStyle()->SetProperty(property, css::PrimitiveValue::Create(value, unit), important);
    InlineStyleChanged();
    return true;
  }

  bool StyledElement::SetInlineStyleProperty(css::PropertyId property, Ref<css::Value> &&value,
                                             css::IsImportant important) noexcept
  {
    EnsureMutableInlineStyle()->SetProperty(property, krys::move(value), important);
    InlineStyleChanged();
    return true;
  }

  bool StyledElement::SetInlineStyleProperty(css::PropertyId property, const css::CSSOMString &value,
                                             css::IsImportant important, bool *didFailParsing) noexcept
  {
    bool changes = EnsureMutableInlineStyle()->SetProperty(
      property, value, css::ParserContext(NodeDocument()), important, didFailParsing);
    if (changes)
    {
      InlineStyleChanged();
    }
    return changes;
  }

  bool StyledElement::SetInlineStyleCustomProperty(const css::CSSOMStringAtom &property,
                                                   const css::CSSOMString &value,
                                                   css::IsImportant important) noexcept
  {
    bool changes = EnsureMutableInlineStyle()->SetCustomProperty(
      css::CSSOMString(property.View()), value, css::ParserContext(NodeDocument()), important);
    if (changes)
    {
      InlineStyleChanged();
    }
    return changes;
  }

  bool StyledElement::SetInlineStyleCustomProperty(Ref<css::Value> &&customPropertyValue,
                                                   css::IsImportant important) noexcept
  {
    EnsureMutableInlineStyle()->AddParsedProperty(
      css::Property(css::PropertyId::Custom, krys::move(customPropertyValue), important));
    InlineStyleChanged();
    return true;
  }

  bool StyledElement::RemoveInlineStyleProperty(css::PropertyId property) noexcept
  {
    if (!InlineStyle())
    {
      return false;
    }
    bool changes = EnsureMutableInlineStyle()->RemoveProperty(property);
    if (changes)
    {
      InlineStyleChanged();
    }
    return changes;
  }

  bool StyledElement::RemoveInlineStyleCustomProperty(const css::CSSOMStringAtom &property) noexcept
  {
    if (!InlineStyle())
    {
      return false;
    }
    bool changes = EnsureMutableInlineStyle()->RemoveCustomProperty(property.View());
    if (changes)
    {
      InlineStyleChanged();
    }
    return changes;
  }

  void StyledElement::RemoveAllInlineStyleProperties() noexcept
  {
    if (!InlineStyle() || InlineStyle()->empty())
    {
      return;
    }
    EnsureMutableInlineStyle()->Clear();
    InlineStyleChanged();
  }
}
