#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/CSS/Markup.hpp"
#include "Krystal.Booey/CSS/Parser/ParserFastPaths.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Ratio.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/RatioValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/SimpleRange.hpp"
#include "Krystal.Booey/DOM/SimulatedClick.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/TextIterator.hpp"
#include "Krystal.Booey/HTML/Attributes/EnumeratedAttributes.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/ElementInternals.hpp"
#include "Krystal.Booey/HTML/HTMLBRElement.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/HTML/LabelsNodeList.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Dimensions.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"

namespace krys::boo::html
{
  HTMLElement::HTMLElement(dom::Document &document, HTMLElementInterface interface, HTMLElementFlags flags,
                           dom::NodeFlags nodeFlags) noexcept
      : StyledElement(document, dom::NullQualifiedName(), nodeFlags | dom::NodeFlags::IsHTMLElement),
        _flags(flags), _interface(interface)
  {
  }

  HTMLElement::HTMLElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::None)
  {
  }

#pragma region HTMLElement

  dom::DOMString HTMLElement::Title() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"title").value_or(u8"");
  }

  void HTMLElement::Title(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"title", krys::move(value));
  }

  dom::DOMString HTMLElement::Lang() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"lang").value_or(u8"");
  }

  void HTMLElement::Lang(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"lang", krys::move(value));
  }

  bool HTMLElement::Translate() const noexcept
  {
    auto value = Reflection::Reflect<dom::DOMString>(*this, u8"translate");
    auto state = EnumeratedAttribute<"translate", HTMLElement>::ResolveState(value);
    return state == AttributeState::Yes;
  }

  void HTMLElement::Translate(bool value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"translate", value ? u8"yes" : u8"no");
  }

  dom::DOMString HTMLElement::Dir() const noexcept
  {
    auto value = Reflection::Reflect<dom::DOMString>(*this, u8"dir");
    return EnumeratedAttribute<"dir", HTMLElement>::ResolveCanonicalKeyword<dom::DOMString>(
      krys::move(value));
  }

  void HTMLElement::Dir(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"dir", krys::move(value));
  }

  BoolOr<dom::DOMString> HTMLElement::Hidden() const noexcept
  {
    auto value = Reflection::Reflect<dom::DOMString>(*this, u8"hidden");

    auto state = EnumeratedAttribute<"hidden", HTMLElement>::ResolveState(value);
    if (state == AttributeState::UntilFound)
    {
      return u8"until-found";
    }

    return state == AttributeState::Hidden;
  }

  bool HTMLElement::IsHiddenUntilFound() const noexcept
  {
    auto value = GetAttribute(u8"hidden");
    return value && infra::StringAlgorithms::ASCIICaseInsensitiveMatch(*value, u8"until-found");
  }

  void HTMLElement::Hidden(dom::DOMString &&value) noexcept
  {
    if (value == u8"until-found")
    {
      dom::ElementAlgorithms::SetAttributeValue(*this, u8"hidden", u8"until-found");
    }
    else if (value.empty())
    {
      dom::ElementAlgorithms::RemoveAttributeByName(u8"hidden", *this);
    }
    else
    {
      dom::ElementAlgorithms::SetAttributeValue(*this, u8"hidden", u8"");
    }
  }

  void HTMLElement::Hidden(bool value) noexcept
  {
    if (value)
    {
      dom::ElementAlgorithms::SetAttributeValue(*this, u8"hidden", u8"");
    }
    else
    {
      dom::ElementAlgorithms::RemoveAttributeByName(u8"hidden", *this);
    }
  }

  void HTMLElement::Hidden(double value) noexcept
  {
    if (value == 0)
    {
      dom::ElementAlgorithms::RemoveAttributeByName(u8"hidden", *this);
    }
    else if (std::isnan(value))
    {
      dom::ElementAlgorithms::RemoveAttributeByName(u8"hidden", *this);
    }
    else
    {
      dom::ElementAlgorithms::SetAttributeValue(*this, u8"hidden", u8"");
    }
  }

  bool HTMLElement::Inert() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"inert");
  }

  void HTMLElement::Inert(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"inert", krys::move(value));
  }

  bool HTMLElement::CanBeActuallyDisabled() const noexcept
  {
    // FIXME: Form-associated custom elements, once there are any.
    return IsHTMLButtonElement() || IsHTMLInputElement() || IsHTMLSelectElement() || IsHTMLTextAreaElement()
           || IsHTMLOptGroupElement() || IsHTMLOptionElement() || IsHTMLFieldSetElement();
  }

  bool HTMLElement::IsActuallyDisabled() const noexcept
  {
    return CanBeActuallyDisabled() && IsDisabledFormControl();
  }

  bool HTMLElement::MatchesReadWritePseudoClass() const noexcept
  {
    return EditabilityFromContentEditableAttr(*this) != dom::Node::Editability::ReadOnly;
  }

  RefPtr<dom::NodeList> HTMLElement::Labels() noexcept
  {
    if (!IsLabelable())
    {
      return nullptr;
    }

    // FIXME: The same list each time ([SameObject]), once nodes cache their lists.
    return CreateRef<LabelsNodeList>(*this);
  }

  void HTMLElement::Click() noexcept
  {
    (void)dom::SimulateClick(*this, nullptr, dom::SimulatedClickMouseEventOptions::SendNoEvents,
                             dom::SimulatedClickVisualOptions::DoNotShowPressedLook,
                             dom::SimulatedClickSource::Bindings);
  }

  dom::DOMString HTMLElement::AccessKey() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"accesskey").value_or(u8"");
  }

  void HTMLElement::AccessKey(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"accesskey", krys::move(value));
  }

  dom::DOMString HTMLElement::AccessKeyLabel() const noexcept
  {
    // TODO(HTMLELEMENT, HTML): Implement AccessKeyLabel() method.
    return {};
  }

  bool HTMLElement::Draggable() const noexcept
  {
    auto value = Reflection::Reflect<dom::DOMString>(*this, u8"draggable");
    switch (EnumeratedAttribute<"draggable", HTMLElement>::ResolveState(value))
    {
      case AttributeState::True:  return true;
      case AttributeState::False: return false;
      case AttributeState::Auto:
      {
        // TODO(HTMLELEMENT, DRAGGABLE, HTML): Implement auto behavior for Draggable() getter.
        //  If the element is an img element, an object element that represents an image, or an a element with
        //  an href content attribute, the draggable IDL attribute must return true; otherwise, the draggable
        //  IDL attribute must return false.
        return false;
      }
    }

    return false;
  }

  void HTMLElement::Draggable(bool value) noexcept
  {
    dom::ElementAlgorithms::SetAttributeValue(*this, u8"draggable", value ? u8"true" : u8"false");
  }

  bool HTMLElement::SpellCheck() const noexcept
  {
    auto value = Reflection::Reflect<dom::DOMString>(*this, u8"spellcheck");
    switch (EnumeratedAttribute<"spellcheck", HTMLElement>::ResolveState(value))
    {
      case AttributeState::True:  return true;
      case AttributeState::False: return false;
      case AttributeState::Default:
      {
        // TODO(HTMLELEMENT, SPELLCHECK, HTML): Implement default behavior for SpellCheck() getter.
        // The spellcheck IDL attribute, on getting, must return true if the element's spellcheck content
        // attribute is in the True state, or if the element's spellcheck content attribute is in the Default
        // state and the element's default behavior is true-by-default, or if the element's spellcheck content
        // attribute is in the Default state and the element's default behavior is inherit-by-default and the
        // element's parent element's spellcheck IDL attribute would return true; otherwise, if none of those
        // conditions applies, then the attribute must instead return false.
        return false;
      }
    }

    return false;
  }

  void HTMLElement::SpellCheck(bool value) noexcept
  {
    dom::ElementAlgorithms::SetAttributeValue(*this, u8"spellcheck", value ? u8"true" : u8"false");
  }

  dom::DOMString HTMLElement::WritingSuggestions() const noexcept
  {
    // TODO(HTMLELEMENT, WRITINGSUGGESTIONS, HTML): Implement WritingSuggestions() method.
    // Return this's computed writing suggestions value.
    return {};
  }

  void HTMLElement::WritingSuggestions(dom::DOMString &&value) noexcept
  {
    Reflection::ReflectSetter(*this, u8"writingsuggestions", krys::move(value));
  }

  dom::DOMString HTMLElement::Autocapitalize() const noexcept
  {
    // TODO(HTMLELEMENT, AUTOCAPITALIZE, HTML): Implement Autocapitalize() method.
    // Let state be the own autocapitalization hint of this.
    // If state is Default, then return the empty string.
    // If state is None, then return "none".
    // If state is Sentences, then return "sentences".
    // Return the keyword value corresponding to state.
    return {};
  }

  void HTMLElement::Autocapitalize(dom::DOMString &&value) noexcept
  {
    Reflection::ReflectSetter(*this, u8"autocapitalize", krys::move(value));
  }

  bool HTMLElement::Autocorrect() const noexcept
  {
    // TODO(HTMLELEMENT, AUTOCORRECT, HTML): Implement Autocorrect() method.
    // The autocorrect getter steps are: return true if the element's used autocorrection state is On and
    // false if the element's used autocorrection state is Off
    return false;
  }

  void HTMLElement::Autocorrect(bool value) noexcept
  {
    dom::ElementAlgorithms::SetAttributeValue(*this, u8"autocorrect", value ? u8"on" : u8"off");
  }

  dom::DOMString HTMLElement::InnerText() const noexcept
  {
    auto &self = const_cast<HTMLElement &>(*this);

    // We need to update layout, since plainText uses line boxes in the render tree.
    self.NodeDocument().UpdateLayoutIgnorePendingStylesheets();

    if (!Renderer())
    {
      return TextContent().value_or(dom::DOMString {});
    }

    if (Renderer()->IsSkippedContent())
    {
      return {};
    }

    return PlainText(dom::MakeRangeSelectingNodeContents(self));
  }

  void HTMLElement::InnerText(dom::DOMString &&value) noexcept
  {
    // FIXME: This doesn't take whitespace collapsing into account at all.

    if (value.find_first_of(u8"\r\n") == dom::DOMString::npos)
    {
      (void)dom::NodeAlgorithms::StringReplaceAll(krys::move(value), *this);
      return;
    }

    if (IsConnected() && IsTextControlInnerTextElement())
    {
      (void)dom::NodeAlgorithms::StringReplaceAll(infra::StringAlgorithms::NormalizeNewlines(value), *this);
      return;
    }

    // Add text nodes and <br> elements.
    auto &document = NodeDocument();
    auto fragment = document.CreateDocumentFragment();
    for (size_t start = 0, length = value.size(); start < length;)
    {
      // Find next line break.
      char8 c = 0;
      size_t i;
      for (i = start; i < length; i++)
      {
        c = value[i];
        if (c == u8'\r' || c == u8'\n')
        {
          break;
        }
      }

      // If text is not the empty string, then append a new Text node whose data is text and node document is
      // document to fragment.
      if (i > start)
      {
        (void)fragment->AppendChild(*document.CreateTextNode(value.substr(start, i - start)));
      }

      if (i == length)
      {
        break;
      }

      (void)fragment->AppendChild(*CreateRef<HTMLBRElement>(document));
      // Make sure \r\n doesn't result in two line breaks.
      if (c == u8'\r' && i + 1 < length && value[i + 1] == u8'\n')
      {
        ++i;
      }

      start = i + 1; // Character after line break.
    }

    (void)dom::MutationAlgorithms::ReplaceAll(fragment.get(), *this);
  }

  dom::DOMString HTMLElement::OuterText() const noexcept
  {
    // TODO(HTMLELEMENT, HTML): Implement OuterText() method.
    krys_not_implemented();
  }

  void HTMLElement::OuterText(dom::DOMString &&value) noexcept
  {
    // TODO(HTMLELEMENT, HTML): Implement OuterText() getter method.
    (void)value;
    krys_not_implemented();
  }

  dom::ExceptionOr<Ref<ElementInternals>> HTMLElement::AttachInternals() noexcept
  {
    // TODO(HTMLELEMENT, ELEMENTINTERNALS, HTML): Implement AttachInternals() method.
    krys_not_implemented();
  }

  uint32 HTMLElement::HeadingOffset() const noexcept
  {
    return Reflection::Reflect<uint32, ReflectRange<uint32>(0u, 8u)>(*this, u8"headingoffset");
  }

  void HTMLElement::HeadingOffset(uint32 value) noexcept
  {
    Reflection::Reflect<uint32>(*this, u8"headingoffset", krys::move(value));
  }

  bool HTMLElement::HeadingReset() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"headingreset");
  }

  void HTMLElement::HeadingReset(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"headingreset", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  namespace
  {
    KRYS_NODISCARD css::ValueId UnicodeBidiAttributeForDirAuto(const HTMLElement &element) noexcept
    {
      auto tagName = element.TagQualifiedName().TagName();
      if (tagName == TagName::pre || tagName == TagName::textarea)
      {
        return css::ValueId::Plaintext;
      }
      return css::ValueId::Isolate;
    }

    enum class ContentEditableType : uint8
    {
      Inherit,
      True,
      False,
      PlaintextOnly
    };

    KRYS_NODISCARD ContentEditableType GetContentEditableType(dom::DOMStringView value) noexcept
    {
      if (value.empty() || infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"true"))
      {
        return ContentEditableType::True;
      }
      if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"false"))
      {
        return ContentEditableType::False;
      }
      if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"plaintext-only"))
      {
        return ContentEditableType::PlaintextOnly;
      }

      return ContentEditableType::Inherit;
    }

  }

  dom::Node::Editability HTMLElement::EditabilityFromContentEditableAttr(const dom::Node &node) noexcept
  {
    // FIXME: A page that's editable as a whole, once there's a page.
    auto *startElement = DynamicDowncast<dom::Element>(node);
    if (!startElement)
    {
      startElement = node.ParentElement();
    }
    for (auto *element = startElement; element; element = element->ParentElement())
    {
      if (!Is<HTMLElement>(*element))
      {
        continue;
      }
      auto value = element->GetAttribute(u8"contenteditable");
      if (!value)
      {
        continue;
      }
      switch (GetContentEditableType(*value))
      {
        case ContentEditableType::True:          return dom::Node::Editability::CanEditRichly;
        case ContentEditableType::PlaintextOnly: return dom::Node::Editability::CanEditPlainText;
        case ContentEditableType::False:         return dom::Node::Editability::ReadOnly;
        case ContentEditableType::Inherit:       break;
      }
    }

    auto *containingShadowRoot = dom::TreeQueries::ContainingShadowRoot(node);
    if (containingShadowRoot && containingShadowRoot->Mode() == dom::ShadowRootMode::UserAgent)
    {
      return dom::Node::Editability::ReadOnly;
    }

    // FIXME: The document's design mode makes the document tree editable, once there's a design mode.
    return dom::Node::Editability::ReadOnly;
  }

  dom::DOMString HTMLElement::AttributeOrEmpty(dom::DOMStringView localName) const noexcept
  {
    return GetAttribute(dom::DOMStringAtom(localName)).value_or(dom::DOMString {});
  }

  uint32 HTMLElement::ParseBorderWidthAttribute(Maybe<dom::DOMStringView> value) const noexcept
  {
    if (value)
    {
      if (auto borderWidth = Numbers::ParseNonNegativeInteger(*value);
          borderWidth.Success() && borderWidth.Value <= std::numeric_limits<int32>::max())
      {
        return static_cast<uint32>(borderWidth.Value);
      }
    }

    return TagQualifiedName().TagName() == TagName::table && value ? 1 : 0;
  }

  void HTMLElement::ApplyBorderAttributeToStyle(dom::DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept
  {
    AddPropertyToPresentationalHintStyle(style, css::PropertyId::BorderWidth,
                                         ParseBorderWidthAttribute(value), css::UnitType::px);
    AddPropertyToPresentationalHintStyle(style, css::PropertyId::BorderStyle, css::ValueId::Solid);
  }

  void HTMLElement::MapLanguageAttributeToLocale(dom::DOMStringView value,
                                                 css::MutableStyleProperties &style) noexcept
  {
    if (!value.empty())
    {
      // Have to quote so the locale id is treated as a string instead of as a CSS keyword.
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::WebkitLocale,
                                           css::SerialiseString(css::CSSOMString(value)));
    }
    else
    {
      // The empty string means the language is explicitly unknown.
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::WebkitLocale, css::ValueId::Auto);
    }
  }

  bool HTMLElement::HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                       dom::DOMStringAtom namespaceURI) const noexcept
  {
    if (namespaceURI == dom::DOMStringAtom::Null())
    {
      if (localName == u8"align" || localName == u8"contenteditable" || localName == u8"hidden"
          || localName == u8"lang" || localName == u8"draggable" || localName == u8"dir")
      {
        return true;
      }
    }
    else if (namespaceURI == infra::Namespaces::XML && localName == u8"lang")
    {
      return true;
    }
    return StyledElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLElement::CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                           dom::DOMStringAtom namespaceURI,
                                                           dom::DOMStringView value,
                                                           css::MutableStyleProperties &style) noexcept
  {
    if (namespaceURI == infra::Namespaces::XML && localName == u8"lang")
    {
      MapLanguageAttributeToLocale(value, style);
      return;
    }
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      StyledElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
      return;
    }

    if (localName == u8"align")
    {
      if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"middle"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::TextAlign, css::ValueId::Center);
      }
      else
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::TextAlign, css::CSSOMString(value));
      }
    }
    else if (localName == u8"contenteditable")
    {
      auto userModifyValue = css::ValueId::ReadWrite;
      switch (GetContentEditableType(value))
      {
        case ContentEditableType::Inherit:
        {
          return;
        }
        case ContentEditableType::False:
        {
          userModifyValue = css::ValueId::ReadOnly;
          break;
        }
        case ContentEditableType::PlaintextOnly:
        case ContentEditableType::True:
        {
          if (GetContentEditableType(value) == ContentEditableType::PlaintextOnly)
          {
            userModifyValue = css::ValueId::ReadWritePlaintextOnly;
          }
          AddPropertyToPresentationalHintStyle(style, css::PropertyId::OverflowWrap, css::ValueId::BreakWord);
          // FIXME: -webkit-nbsp-mode: space is dropped with the property.
          AddPropertyToPresentationalHintStyle(style, css::PropertyId::LineBreak,
                                               css::ValueId::AfterWhiteSpace);
          break;
        }
      }
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::WebkitUserModify, userModifyValue);
    }
    else if (localName == u8"hidden")
    {
      // The hiddenUntilFoundEnabled setting is on by default.
      if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"until-found"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::ContentVisibility, css::ValueId::Hidden);
      }
      else
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::Display, css::ValueId::None);
      }
    }
    else if (localName == u8"draggable")
    {
      if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"true"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::WebkitUserDrag, css::ValueId::Element);
      }
      else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"false"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::WebkitUserDrag, css::ValueId::None);
      }
    }
    else if (localName == u8"dir")
    {
      if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"auto"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::UnicodeBidi,
                                             UnicodeBidiAttributeForDirAuto(*this));
      }
      else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"rtl")
               || infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"ltr"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::Direction, css::CSSOMString(value));
        auto tagName = TagQualifiedName().TagName();
        if (tagName != TagName::bdi && tagName != TagName::bdo && tagName != TagName::output)
        {
          AddPropertyToPresentationalHintStyle(style, css::PropertyId::UnicodeBidi, css::ValueId::Isolate);
        }
      }
    }
    else if (localName == u8"lang")
    {
      // xml:lang has a higher priority than lang.
      if (!HasAttributeNS(dom::DOMStringAtom(infra::Namespaces::XML), u8"lang"))
      {
        MapLanguageAttributeToLocale(value, style);
      }
    }
    else
    {
      StyledElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
    }
  }

  void HTMLElement::ApplyAspectRatioFromWidthAndHeightAttributesToStyle(
    dom::DOMStringView widthAttribute, dom::DOMStringView heightAttribute,
    css::MutableStyleProperties &style) noexcept
  {
    auto dimensionWidth = Dimensions::ParseHTMLDimension(widthAttribute);
    if (!dimensionWidth || dimensionWidth->DimensionType != HTMLDimension::Type::Pixel)
    {
      return;
    }
    auto dimensionHeight = Dimensions::ParseHTMLDimension(heightAttribute);
    if (!dimensionHeight || dimensionHeight->DimensionType != HTMLDimension::Type::Pixel)
    {
      return;
    }

    AddParsedWidthAndHeightToAspectRatioList(dimensionWidth->Number, dimensionHeight->Number, style);
  }

  void HTMLElement::ApplyAspectRatioWithoutDimensionalRulesFromWidthAndHeightAttributesToStyle(
    dom::DOMStringView widthAttribute, dom::DOMStringView heightAttribute,
    css::MutableStyleProperties &style) noexcept
  {
    auto dimensionWidth = Numbers::ParseNonNegativeInteger(widthAttribute);
    if (!dimensionWidth.Success())
    {
      return;
    }
    auto dimensionHeight = Numbers::ParseNonNegativeInteger(heightAttribute);
    if (!dimensionHeight.Success())
    {
      return;
    }

    AddParsedWidthAndHeightToAspectRatioList(static_cast<double>(dimensionWidth.Value),
                                             static_cast<double>(dimensionHeight.Value), style);
  }

  void HTMLElement::AddParsedWidthAndHeightToAspectRatioList(double width, double height,
                                                             css::MutableStyleProperties &style) noexcept
  {
    style.SetProperty(css::PropertyId::AspectRatio, css::ValueList::CreateSpaceSeparated(
                                                      css::PrimitiveValue::Create(css::ValueId::Auto),
                                                      css::RatioValue::Create(css::Ratio {width, height})));
  }

  void HTMLElement::ApplyAlignmentAttributeToStyle(dom::DOMStringView alignment,
                                                   css::MutableStyleProperties &style) noexcept
  {
    // Vertical alignment with respect to the current baseline of the text
    // right or left means floating images.
    auto floatValue = css::ValueId::Invalid;
    auto verticalAlignValue = css::ValueId::Invalid;

    if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(alignment, u8"absmiddle")
        || infra::StringAlgorithms::ASCIICaseInsensitiveMatch(alignment, u8"abscenter"))
    {
      verticalAlignValue = css::ValueId::Middle;
    }
    else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(alignment, u8"absbottom"))
    {
      verticalAlignValue = css::ValueId::Bottom;
    }
    else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(alignment, u8"left"))
    {
      floatValue = css::ValueId::Left;
      verticalAlignValue = css::ValueId::Top;
    }
    else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(alignment, u8"right"))
    {
      floatValue = css::ValueId::Right;
      verticalAlignValue = css::ValueId::Top;
    }
    else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(alignment, u8"top"))
    {
      verticalAlignValue = css::ValueId::Top;
    }
    else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(alignment, u8"middle"))
    {
      verticalAlignValue = css::ValueId::WebkitBaselineMiddle;
    }
    else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(alignment, u8"center"))
    {
      verticalAlignValue = css::ValueId::Middle;
    }
    else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(alignment, u8"bottom"))
    {
      verticalAlignValue = css::ValueId::Baseline;
    }
    else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(alignment, u8"texttop"))
    {
      verticalAlignValue = css::ValueId::TextTop;
    }

    if (floatValue != css::ValueId::Invalid)
    {
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::Float, floatValue);
    }

    if (verticalAlignValue != css::ValueId::Invalid)
    {
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::VerticalAlign, verticalAlignValue);
    }
  }

  void HTMLElement::AddHTMLLengthToStyle(css::MutableStyleProperties &style, css::PropertyId property,
                                         dom::DOMStringView value, AllowPercentage allowPercentage,
                                         UseCSSPXAsUnitType useCSSPX, IsMultiLength isMultiLength,
                                         AllowZeroValue allowZeroValue) noexcept
  {
    auto dimensionValue = isMultiLength == IsMultiLength::No ? Dimensions::ParseHTMLDimension(value)
                                                             : Dimensions::ParseHTMLMultiLength(value);
    if (!dimensionValue || (!dimensionValue->Number && allowZeroValue == AllowZeroValue::No))
    {
      return;
    }
    if (dimensionValue->DimensionType == HTMLDimension::Type::Percentage)
    {
      if (allowPercentage == AllowPercentage::Yes)
      {
        AddPropertyToPresentationalHintStyle(style, property, dimensionValue->Number,
                                             css::UnitType::Percentage);
      }
      return;
    }
    if (useCSSPX == UseCSSPXAsUnitType::Yes)
    {
      AddPropertyToPresentationalHintStyle(style, property, dimensionValue->Number, css::UnitType::px);
    }
    else
    {
      AddPropertyToPresentationalHintStyle(style, property, dimensionValue->Number, css::UnitType::Number);
    }
  }

  void HTMLElement::AddHTMLLengthToStyle(css::MutableStyleProperties &style, css::PropertyId property,
                                         dom::DOMStringView value, AllowZeroValue allowZeroValue) noexcept
  {
    AddHTMLLengthToStyle(style, property, value, AllowPercentage::Yes, UseCSSPXAsUnitType::Yes,
                         IsMultiLength::No, allowZeroValue);
  }

  void HTMLElement::AddHTMLMultiLengthToStyle(css::MutableStyleProperties &style, css::PropertyId property,
                                              dom::DOMStringView value) noexcept
  {
    AddHTMLLengthToStyle(style, property, value, AllowPercentage::Yes, UseCSSPXAsUnitType::Yes,
                         IsMultiLength::Yes);
  }

  void HTMLElement::AddHTMLPixelsToStyle(css::MutableStyleProperties &style, css::PropertyId property,
                                         dom::DOMStringView value) noexcept
  {
    AddHTMLLengthToStyle(style, property, value, AllowPercentage::No, UseCSSPXAsUnitType::Yes,
                         IsMultiLength::No);
  }

  // Color parsing that matches HTML's "rules for parsing a legacy color value"
  Maybe<SRGBA<uint8>> HTMLElement::ParseLegacyColorValue(dom::DOMStringView string) noexcept
  {
    // An empty string doesn't apply a color.
    if (string.empty())
    {
      return {};
    }

    string = infra::StringAlgorithms::StripLeadingAndTrailingWhitespace(string);
    if (string.empty())
    {
      return boo::Color::black;
    }

    // "transparent" doesn't apply a color either.
    if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(string, u8"transparent"))
    {
      return {};
    }

    if (auto namedColor = css::ParserFastPaths::ParseNamedColor(string))
    {
      return namedColor;
    }

    if (string.size() == 4 && string[0] == '#' && text::IsASCIIHexDigit(string[1])
        && text::IsASCIIHexDigit(string[2]) && text::IsASCIIHexDigit(string[3]))
    {
      return SRGBA<uint8> {static_cast<uint8>(text::ToASCIIHexValue(string[1]) * 0x11),
                           static_cast<uint8>(text::ToASCIIHexValue(string[2]) * 0x11),
                           static_cast<uint8>(text::ToASCIIHexValue(string[3]) * 0x11)};
    }

    // Per spec, only look at the first 128 digits of the string.
    constexpr size_t maxColorLength = 128;

    // We'll pad the buffer with two extra 0s later, so reserve two more than the max.
    SmallList<char8, maxColorLength + 2> digitBuffer;

    // Grab the first 128 code points, replacing non-hex characters with 0. Code points outside the BMP become
    // "00", as they're two code units in UTF-16.
    size_t i = 0;
    if (string[0] == '#') // Skip a leading #.
    {
      i = 1;
    }
    while (i < string.size() && digitBuffer.size() < maxColorLength)
    {
      auto lead = static_cast<uint8>(string[i]);
      size_t length = lead < 0x80 ? 1 : lead < 0xE0 ? 2 : lead < 0xF0 ? 3 : 4;
      if (length == 1 && text::IsASCIIHexDigit(string[i]))
      {
        digitBuffer.push_back(string[i]);
      }
      else
      {
        digitBuffer.push_back('0');
        if (length == 4 && digitBuffer.size() < maxColorLength)
        {
          digitBuffer.push_back('0');
        }
      }
      i += length;
    }

    if (digitBuffer.empty())
    {
      return boo::Color::black;
    }

    // Pad the buffer out to at least the next multiple of three in size.
    digitBuffer.push_back('0');
    digitBuffer.push_back('0');

    if (digitBuffer.size() < 6)
    {
      return SRGBA<uint8> {text::ToASCIIHexValue(digitBuffer[0]), text::ToASCIIHexValue(digitBuffer[1]),
                           text::ToASCIIHexValue(digitBuffer[2])};
    }

    // Split the digits into three components, then search the last 8 digits of each component.
    krys_debug_assert(digitBuffer.size() >= 6);
    size_t componentLength = digitBuffer.size() / 3;
    size_t componentSearchWindowLength = std::min<size_t>(componentLength, 8);
    size_t redIndex = componentLength - componentSearchWindowLength;
    size_t greenIndex = componentLength * 2 - componentSearchWindowLength;
    size_t blueIndex = componentLength * 3 - componentSearchWindowLength;
    // Skip digits until one of them is non-zero, or we've only got two digits left in the component.
    while (digitBuffer[redIndex] == '0' && digitBuffer[greenIndex] == '0' && digitBuffer[blueIndex] == '0'
           && (componentLength - redIndex) > 2)
    {
      redIndex++;
      greenIndex++;
      blueIndex++;
    }
    krys_debug_assert(redIndex + 1 < componentLength);
    krys_debug_assert(greenIndex >= componentLength);
    krys_debug_assert(greenIndex + 1 < componentLength * 2);
    krys_debug_assert(blueIndex >= componentLength * 2);
    krys_debug_assert(blueIndex + 1 < digitBuffer.size());

    auto redValue = text::ToASCIIHexValue(digitBuffer[redIndex], digitBuffer[redIndex + 1]);
    auto greenValue = text::ToASCIIHexValue(digitBuffer[greenIndex], digitBuffer[greenIndex + 1]);
    auto blueValue = text::ToASCIIHexValue(digitBuffer[blueIndex], digitBuffer[blueIndex + 1]);
    return SRGBA<uint8> {redValue, greenValue, blueValue};
  }

  void HTMLElement::AddHTMLColorToStyle(css::MutableStyleProperties &style, css::PropertyId property,
                                        dom::DOMStringView attributeValue) noexcept
  {
    if (auto color = ParseLegacyColorValue(attributeValue))
    {
      style.SetProperty(property, css::ValuePool::MainThreadPool().CreateColorValue(boo::Color(*color)));
    }
  }

#pragma endregion
}
