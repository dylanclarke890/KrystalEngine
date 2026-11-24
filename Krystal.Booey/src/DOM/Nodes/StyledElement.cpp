#include "Krystal.Booey/DOM/Nodes/StyledElement.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSStyleProperties.hpp"
#include "Krystal.Booey/CSS/Parser/Parser.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Properties/ImmutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/StylePropertiesInlines.hpp"
#include "Krystal.Booey/CSS/TypedOM/InlineStylePropertyMap.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/DOM/Internals/ElementRareData.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Style/Invalidation/AttributeChangeInvalidation.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/Scope/Scope.hpp"
#include "Krystal.Booey/SVG/Elements/SVGElement.hpp"

namespace krys::boo
{
  StyledElement::StyledElement(Document &document, const QualifiedName &name,
                               dom::NodeFlags nodeFlags) noexcept
      : Element(document, name, nodeFlags)
  {
  }

  StyledElement::~StyledElement() noexcept = default;

#pragma region ElementCSSInlineStyle

  CSSStyleProperties &StyledElement::Style() noexcept
  {
    return EnsureMutableInlineStyle()->EnsureInlineCSSStyleProperties(*this);
  }

  StylePropertyMap &StyledElement::AttributeStyleMap() noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-elementcssinlinestyle-attributestylemap
    if (!ExistingAttributeStyleMap())
    {
      SetAttributeStyleMap(InlineStylePropertyMap::Create(*this));
    }
    return *ExistingAttributeStyleMap();
  }

#pragma endregion

#pragma region The style attribute

  void StyledElement::OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                                         Maybe<DOMStringView> value,
                                         const DOMStringAtom &namespaceURI) noexcept
  {
    Element::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (oldValue == value)
    {
      return;
    }
    if (localName == u8"style" && namespaceURI == null)
    {
      StyleAttributeChanged(value);
    }
    else if (HasPresentationalHintsForAttribute(localName, namespaceURI))
    {
      _presentationalHintStyleIsDirty = true;
      InvalidateStyleInternal();
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

    // The attribute is the source now, so there's nothing to write back.
    _nodeFlags.Remove(dom::NodeFlag::StyleAttributeIsDirty);
    Node::InvalidateStyle(style::Validity::InlineStyleInvalid);
  }

  void StyledElement::SetInlineStyleFromString(DOMStringView newStyleString) noexcept
  {
    // A style already made mutable (it has a CSSOM wrapper, or was changed) is reparsed in place, so the
    // wrapper stays valid. Otherwise the text is parsed into a new immutable block, which the matched
    // declarations cache can keep (ElementRuleCollector only caches an immutable inline style).
    if (auto *mutableStyleProperties = DynamicDowncast<css::MutableStyleProperties>(_inlineStyle.get()))
    {
      mutableStyleProperties->ParseDeclaration(CSSOMString(newStyleString),
                                               css::ParserContext(NodeDocument()));
    }
    else
    {
      _inlineStyle = css::Parser::ParseInlineStyleDeclaration(newStyleString, *this);
    }
  }

  void StyledElement::SynchronizeStyleAttributeInternal() noexcept
  {
    krys_debug_assert(_nodeFlags.contains(dom::NodeFlag::StyleAttributeIsDirty));
    _nodeFlags.Remove(dom::NodeFlag::StyleAttributeIsDirty);
    if (const auto *inlineStyle = InlineStyle())
    {
      SetSynchronizedLazyAttribute(u8"style", inlineStyle->AsText(css::DefaultSerialisationContext()));
    }
  }

  void StyledElement::SynchronizeStyleAttributeWithInvalidation() noexcept
  {
    if (const auto *inlineStyle = InlineStyle())
    {
      _nodeFlags.Remove(dom::NodeFlag::StyleAttributeIsDirty);
      auto newValue = inlineStyle->AsText(css::DefaultSerialisationContext());
      auto oldValue = GetAttribute(u8"style");
      style::AttributeChangeInvalidation styleInvalidation(*this, QualifiedName {null, null, u8"style"},
                                                           oldValue ? Maybe<DOMStringView>(*oldValue) : null,
                                                           newValue);
      SetSynchronizedLazyAttribute(u8"style", krys::move(newValue));
    }
  }

  void StyledElement::DirtyStyleAttribute() noexcept
  {
    _nodeFlags.Add(dom::NodeFlag::StyleAttributeIsDirty);

    // A selector that tests the style attribute needs its text now to match.
    if (StyleResolver().RuleSets().GetSelectorsForStyleAttribute() != style::SelectorsForStyleAttribute::None)
    {
      SynchronizeStyleAttributeWithInvalidation();
    }
  }

  void StyledElement::InvalidateStyleAttribute() noexcept
  {
    _nodeFlags.Add(dom::NodeFlag::StyleAttributeIsDirty);

    // Only the inline style needs recomputing, unless a selector tests the style attribute: then a rule
    // may start or stop matching the element.
    auto selectorsForStyleAttribute = StyleResolver().RuleSets().GetSelectorsForStyleAttribute();
    auto validity = selectorsForStyleAttribute == style::SelectorsForStyleAttribute::None
                      ? style::Validity::InlineStyleInvalid
                      : style::Validity::ElementInvalid;
    Node::InvalidateStyle(validity);

    if (auto *svgElement = DynamicDowncast<SVGElement>(*this))
    {
      svgElement->InvalidateInstances();
    }

    // A selector that tests it on some other element than its subject (`[style] ~ div`) affects other
    // elements, which only the invalidation for the attribute's change finds.
    if (selectorsForStyleAttribute == style::SelectorsForStyleAttribute::NonSubjectPosition)
    {
      SynchronizeStyleAttributeWithInvalidation();
    }
  }

#pragma endregion

#pragma region Inline style

  const css::StyleProperties *StyledElement::InlineStyle() const noexcept
  {
    return _inlineStyle.get();
  }

  Ref<css::MutableStyleProperties> StyledElement::EnsureMutableInlineStyle() noexcept
  {
    if (!_inlineStyle)
    {
      // Strict for an HTML element in a standards-mode document, as parsing the attribute's text is.
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

  bool StyledElement::SetInlineStyleProperty(css::PropertyId property, css::ValueId identifier,
                                             css::IsImportant important) noexcept
  {
    EnsureMutableInlineStyle()->SetProperty(property, css::PrimitiveValue::Create(identifier), important);
    InvalidateStyleAttribute();
    return true;
  }

  bool StyledElement::SetInlineStyleProperty(css::PropertyId property, double value, css::UnitType unit,
                                             css::IsImportant important) noexcept
  {
    EnsureMutableInlineStyle()->SetProperty(property, css::PrimitiveValue::Create(value, unit), important);
    InvalidateStyleAttribute();
    return true;
  }

  bool StyledElement::SetInlineStyleProperty(css::PropertyId property, const CSSOMString &value,
                                             css::IsImportant important, bool *didFailParsing) noexcept
  {
    bool changed = EnsureMutableInlineStyle()->SetProperty(
      property, value, css::ParserContext(NodeDocument()), important, didFailParsing);
    if (changed)
    {
      InvalidateStyleAttribute();
    }
    return changed;
  }

  bool StyledElement::SetInlineStyleProperty(css::PropertyId property, Ref<css::Value> &&value,
                                             css::IsImportant important) noexcept
  {
    EnsureMutableInlineStyle()->SetProperty(property, krys::move(value), important);
    InvalidateStyleAttribute();
    return true;
  }

  bool StyledElement::SetInlineStyleCustomProperty(Ref<css::Value> &&customPropertyValue,
                                                   css::IsImportant important) noexcept
  {
    EnsureMutableInlineStyle()->AddParsedProperty(
      css::Property(css::PropertyId::Custom, krys::move(customPropertyValue), important));
    InvalidateStyleAttribute();
    return true;
  }

  bool StyledElement::RemoveInlineStyleProperty(css::PropertyId property) noexcept
  {
    if (!InlineStyle())
    {
      return false;
    }
    bool changed = EnsureMutableInlineStyle()->RemoveProperty(property);
    if (changed)
    {
      InvalidateStyleAttribute();
    }
    return changed;
  }

  bool StyledElement::RemoveInlineStyleCustomProperty(const CSSOMStringAtom &property) noexcept
  {
    if (!InlineStyle())
    {
      return false;
    }
    bool changed = EnsureMutableInlineStyle()->RemoveCustomProperty(property.View());
    if (changed)
    {
      InvalidateStyleAttribute();
    }
    return changed;
  }

  void StyledElement::RemoveAllInlineStyleProperties() noexcept
  {
    if (!InlineStyle() || InlineStyle()->empty())
    {
      return;
    }
    EnsureMutableInlineStyle()->Clear();
    InvalidateStyleAttribute();
  }

#pragma endregion

#pragma region Presentational hints

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
    // Hints are parsed in SVG attribute mode for SVG elements, and in quirks mode otherwise, whatever the
    // document's mode.
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

    // Identical hint styles are shared, unless the element is an SVG one positioned by a non-zero x, y, cx
    // or cy, which other elements are unlikely to share.
    const auto isWorthDeduplicating = [&]
    {
      if (!isSVG)
      {
        return true;
      }
      for (auto property : {css::PropertyId::X, css::PropertyId::Y, css::PropertyId::Cx, css::PropertyId::Cy})
      {
        auto value = style->GetPropertyCSSValue(property);
        auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value.get());
        if (primitiveValue && !primitiveValue->IsZero().value_or(false))
        {
          return false;
        }
      }
      return true;
    }();
    _presentationalHintStyle =
      isWorthDeduplicating ? style->ImmutableDeduplicatedCopy() : style->ImmutableCopy();
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
                                                           const CSSOMString &value) noexcept
  {
    style.SetProperty(property, value, NodeDocument().CssParserContext());
  }

  void StyledElement::AddPropertyToPresentationalHintStyle(css::MutableStyleProperties &style,
                                                           css::PropertyId property,
                                                           Ref<css::Value> &&value) noexcept
  {
    style.SetProperty(property, krys::move(value));
  }

#pragma endregion
}
