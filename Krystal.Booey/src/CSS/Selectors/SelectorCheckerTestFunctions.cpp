#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/HTMLButtonElement.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/HTMLOptionElement.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include <algorithm>

namespace krys::boo::css
{
  namespace
  {
    template <typename T>
    KRYS_NODISCARD const T *DynamicDowncastHTML(const dom::Element &element) noexcept
    {
      auto *htmlElement = DynamicDowncast<html::HTMLElement>(element);
      return htmlElement ? DynamicDowncast<T>(*htmlElement) : nullptr;
    }

    KRYS_NODISCARD bool IsHTMLElement(const dom::Element &element, html::TagName tagName) noexcept
    {
      return Is<html::HTMLElement>(element) && element.TagQualifiedName().TagName() == tagName;
    }

    KRYS_NODISCARD bool IsOneOf(dom::DOMStringView value,
                                std::initializer_list<dom::DOMStringView> values) noexcept
    {
      return std::ranges::find(values, value) != values.end();
    }

    /// @see https://html.spec.whatwg.org/#attr-input-type
    KRYS_NODISCARD dom::DOMString InputType(const dom::Element &input) noexcept
    {
      auto type =
        text::ToASCIILower(dom::DOMStringView(input.GetAttribute(u8"type").value_or(dom::DOMString())));
      if (IsOneOf(type, {u8"hidden",   u8"text",  u8"search", u8"tel",      u8"url",   u8"email",
                         u8"password", u8"date",  u8"month",  u8"week",     u8"time",  u8"datetime-local",
                         u8"number",   u8"range", u8"color",  u8"checkbox", u8"radio", u8"file",
                         u8"submit",   u8"image", u8"reset",  u8"button"}))
      {
        return type;
      }
      return u8"text";
    }

    /// @see https://html.spec.whatwg.org/#attr-button-type
    KRYS_NODISCARD bool IsSubmitButton(const dom::Element &element) noexcept
    {
      if (IsHTMLElement(element, html::TagName::button))
      {
        auto type =
          text::ToASCIILower(dom::DOMStringView(element.GetAttribute(u8"type").value_or(dom::DOMString())));
        return type != u8"reset" && type != u8"button";
      }
      if (IsHTMLElement(element, html::TagName::input))
      {
        auto type = InputType(element);
        return type == u8"submit" || type == u8"image";
      }
      return false;
    }

    /// @brief Whether @p element is a descendant of a `fieldset` with a `disabled` attribute, outside that
    /// fieldset's first `legend` child.
    KRYS_NODISCARD bool IsInDisabledFieldset(const dom::Element &element) noexcept
    {
      for (auto *ancestor = element.ParentElement(); ancestor; ancestor = ancestor->ParentElement())
      {
        if (!IsHTMLElement(*ancestor, html::TagName::fieldset) || !ancestor->HasAttribute(u8"disabled"))
        {
          continue;
        }
        const dom::Element *firstLegend = nullptr;
        for (auto *child = dom::TreeQueries::FirstElementChild(*ancestor); child;
             child = dom::TreeQueries::NextElementSibling(*child))
        {
          if (IsHTMLElement(*child, html::TagName::legend))
          {
            firstLegend = child;
            break;
          }
        }
        if (!firstLegend || !dom::TreeQueries::IsInclusiveAncestor(*firstLegend, element))
        {
          return true;
        }
      }
      return false;
    }

    /// @see https://html.spec.whatwg.org/#concept-fe-disabled
    KRYS_NODISCARD bool IsDisabledFormControl(const dom::Element &element) noexcept
    {
      // FIXME: Form-associated custom elements.
      return element.HasAttribute(u8"disabled") || IsInDisabledFieldset(element);
    }

    KRYS_NODISCARD bool CanBeActuallyDisabled(const dom::Element &element) noexcept
    {
      if (!Is<html::HTMLElement>(element))
      {
        return false;
      }
      switch (element.TagQualifiedName().TagName())
      {
        case html::TagName::button:
        case html::TagName::input:
        case html::TagName::select:
        case html::TagName::textarea:
        case html::TagName::optgroup:
        case html::TagName::option:
        case html::TagName::fieldset: return true;
        default:                      return false;
      }
    }

    /// @see https://html.spec.whatwg.org/#concept-element-disabled
    KRYS_NODISCARD bool IsActuallyDisabled(const dom::Element &element) noexcept
    {
      if (!Is<html::HTMLElement>(element))
      {
        return false;
      }
      switch (element.TagQualifiedName().TagName())
      {
        case html::TagName::button:
        case html::TagName::input:
        case html::TagName::select:
        case html::TagName::textarea:
        case html::TagName::fieldset: return IsDisabledFormControl(element);
        case html::TagName::optgroup: return element.HasAttribute(u8"disabled");
        case html::TagName::option:
        {
          if (element.HasAttribute(u8"disabled"))
          {
            return true;
          }
          auto *parent = element.ParentElement();
          return parent && IsHTMLElement(*parent, html::TagName::optgroup)
                 && parent->HasAttribute(u8"disabled");
        }
        default: return false;
      }
    }

    /// @brief Whether the `readonly` attribute applies to an `input` of type @p type.
    KRYS_NODISCARD bool ReadOnlyApplies(dom::DOMStringView type) noexcept
    {
      return IsOneOf(type, {u8"text", u8"search", u8"url", u8"tel", u8"email", u8"password", u8"date",
                            u8"month", u8"week", u8"time", u8"datetime-local", u8"number"});
    }

    /// @brief Whether the `required` attribute applies to an `input` of type @p type.
    KRYS_NODISCARD bool RequiredApplies(dom::DOMStringView type) noexcept
    {
      return ReadOnlyApplies(type) || IsOneOf(type, {u8"checkbox", u8"radio", u8"file"});
    }

    /// @brief Whether the `placeholder` attribute applies to an `input` of type @p type.
    KRYS_NODISCARD bool PlaceholderApplies(dom::DOMStringView type) noexcept
    {
      return IsOneOf(type, {u8"text", u8"search", u8"url", u8"tel", u8"email", u8"password", u8"number"});
    }

    /// @see https://html.spec.whatwg.org/#editing-host
    KRYS_NODISCARD bool IsEditable(const dom::Element &element) noexcept
    {
      // FIXME: The document's designMode.
      for (auto *ancestor = &element; ancestor; ancestor = ancestor->ParentElement())
      {
        if (!Is<html::HTMLElement>(*ancestor))
        {
          continue;
        }
        if (auto value = ancestor->GetAttribute(u8"contenteditable"))
        {
          auto state = text::ToASCIILower(dom::DOMStringView(*value));
          if (state.empty() || state == u8"true" || state == u8"plaintext-only")
          {
            return true;
          }
          if (state == u8"false")
          {
            return false;
          }
        }
      }
      return false;
    }
  }

  bool IsLink(const dom::Element &element) noexcept
  {
    if (IsHTMLElement(element, html::TagName::a) || IsHTMLElement(element, html::TagName::area))
    {
      return element.HasAttribute(u8"href");
    }
    if (element.NamespaceURI().View() == infra::Namespaces::SVG && element.LocalName().View() == u8"a")
    {
      return element.HasAttribute(u8"href")
             || element.HasAttributeNS(dom::DOMStringAtom(infra::Namespaces::XLink), u8"href");
    }
    return false;
  }

  bool MatchesDefaultPseudoClass(const dom::Element &element) noexcept
  {
    if (IsHTMLElement(element, html::TagName::input))
    {
      auto type = InputType(element);
      if (type == u8"checkbox" || type == u8"radio")
      {
        return element.HasAttribute(u8"checked");
      }
    }
    if (IsHTMLElement(element, html::TagName::option))
    {
      return element.HasAttribute(u8"selected");
    }
    if (!IsSubmitButton(element))
    {
      return false;
    }

    // The form's default button: the first submit button in tree order whose form owner is the form.
    // FIXME: The form attribute; the form owner is the nearest ancestor form until form owners are tracked.
    auto formOwner = [](const dom::Element &control) -> const dom::Element *
    {
      for (auto *ancestor = control.ParentElement(); ancestor; ancestor = ancestor->ParentElement())
      {
        if (IsHTMLElement(*ancestor, html::TagName::form))
        {
          return ancestor;
        }
      }
      return nullptr;
    };
    auto *form = formOwner(element);
    if (!form)
    {
      return false;
    }
    for (auto *node = dom::TreeQueries::Next(*form, form); node; node = dom::TreeQueries::Next(*node, form))
    {
      auto *candidate = DynamicDowncast<dom::Element>(*node);
      if (candidate && IsSubmitButton(*candidate) && formOwner(*candidate) == form)
      {
        return candidate == &element;
      }
    }
    return false;
  }

  bool MatchesDisabledPseudoClass(const dom::Element &element) noexcept
  {
    return IsActuallyDisabled(element);
  }

  bool MatchesEnabledPseudoClass(const dom::Element &element) noexcept
  {
    return CanBeActuallyDisabled(element) && !IsActuallyDisabled(element);
  }

  bool IsDefinedElement(const dom::Element &element) noexcept
  {
    auto state = element.GetCustomElementState();
    return state == dom::CustomElementState::Custom || state == dom::CustomElementState::Uncustomized;
  }

  bool IsChecked(const dom::Element &element) noexcept
  {
    if (auto *input = DynamicDowncastHTML<html::HTMLInputElement>(element))
    {
      auto type = InputType(element);
      return (type == u8"checkbox" || type == u8"radio") && input->Checked();
    }
    if (auto *option = DynamicDowncastHTML<html::HTMLOptionElement>(element))
    {
      return option->Selected();
    }
    return false;
  }

  bool IsOptionalFormControl(const dom::Element &element) noexcept
  {
    if (IsHTMLElement(element, html::TagName::input))
    {
      return RequiredApplies(InputType(element)) && !element.HasAttribute(u8"required");
    }
    if (IsHTMLElement(element, html::TagName::select) || IsHTMLElement(element, html::TagName::textarea))
    {
      return !element.HasAttribute(u8"required");
    }
    return false;
  }

  bool IsRequiredFormControl(const dom::Element &element) noexcept
  {
    if (IsHTMLElement(element, html::TagName::input))
    {
      return RequiredApplies(InputType(element)) && element.HasAttribute(u8"required");
    }
    if (IsHTMLElement(element, html::TagName::select) || IsHTMLElement(element, html::TagName::textarea))
    {
      return element.HasAttribute(u8"required");
    }
    return false;
  }

  bool IsPlaceholderShown(const dom::Element &element) noexcept
  {
    auto hasPlaceholder = [&]
    {
      auto placeholder = element.GetAttribute(u8"placeholder");
      return placeholder
             && std::ranges::any_of(*placeholder, [](char8 c) { return c != u8'\r' && c != u8'\n'; });
    };
    if (auto *input = DynamicDowncastHTML<html::HTMLInputElement>(element))
    {
      return PlaceholderApplies(InputType(element)) && hasPlaceholder() && input->Value().empty();
    }
    if (IsHTMLElement(element, html::TagName::textarea))
    {
      // FIXME: The raw value, once textarea has one; its default value is its child text content.
      return hasPlaceholder() && element.TextContent().value_or(dom::DOMString()).empty();
    }
    return false;
  }

  bool ContainsLanguageSubtagMatchingRange(dom::DOMStringView language, dom::DOMStringView range,
                                           uint32 languageLength, uint32 &position) noexcept
  {
    uint32 languageSubtagsStartIndex = position;
    uint32 languageSubtagsEndIndex = languageLength;
    bool isAsteriskRange = range == u8"*";
    do
    {
      if (languageSubtagsStartIndex > 0)
      {
        languageSubtagsStartIndex += 1;
      }

      languageSubtagsEndIndex = static_cast<uint32>(
        std::min<size_t>(language.find(u8'-', languageSubtagsStartIndex), languageLength));

      if (languageSubtagsStartIndex > languageSubtagsEndIndex)
      {
        return false;
      }

      auto languageSubtag =
        language.substr(languageSubtagsStartIndex, languageSubtagsEndIndex - languageSubtagsStartIndex);
      bool isEqual = text::IsASCIICaselessEqual(range, languageSubtag);
      if (!isAsteriskRange)
      {
        if ((!isEqual && !languageSubtagsStartIndex)
            || (languageSubtag.size() == 1 && languageSubtagsStartIndex > 0))
        {
          return false;
        }
      }
      languageSubtagsStartIndex = languageSubtagsEndIndex;
      if (isEqual || isAsteriskRange)
      {
        position = languageSubtagsStartIndex;
        return true;
      }

    } while (languageSubtagsStartIndex < languageLength);
    return false;
  }

  bool MatchesLangPseudoClass(const dom::Element &element,
                              const FixedVector<PossiblyQuotedIdentifier> &langList) noexcept
  {
    // FIXME: WebVTT elements use their cue's language.
    auto language = EffectiveLang(element);

    if (language.empty())
    {
      return false;
    }

    // Implement basic and extended filterings of given language tags as specified in
    // www.ietf.org/rfc/rfc4647.txt.
    dom::DOMStringView languageStringView = language;
    auto languageLength = static_cast<uint32>(language.size());
    for (auto &range : langList)
    {
      auto rangeStringView = range.Identifier.View();
      if (rangeStringView.empty())
      {
        continue;
      }
      if (rangeStringView == u8"*")
      {
        return true;
      }
      if (text::IsASCIICaselessEqual(languageStringView, rangeStringView)
          && languageStringView.find(u8'-') == dom::DOMStringView::npos)
      {
        return true;
      }

      auto rangeLength = static_cast<uint32>(rangeStringView.size());
      uint32 rangeSubtagsStartIndex = 0;
      uint32 lastMatchedLanguageSubtagIndex = 0;

      bool matchedRange = true;
      do
      {
        if (rangeSubtagsStartIndex > 0)
        {
          rangeSubtagsStartIndex += 1;
        }
        if (rangeSubtagsStartIndex > languageLength)
        {
          return false;
        }
        auto rangeSubtagsEndIndex = static_cast<uint32>(
          std::min<size_t>(rangeStringView.find(u8'-', rangeSubtagsStartIndex), rangeLength));
        auto rangeSubtag =
          rangeStringView.substr(rangeSubtagsStartIndex, rangeSubtagsEndIndex - rangeSubtagsStartIndex);
        if (!ContainsLanguageSubtagMatchingRange(languageStringView, rangeSubtag, languageLength,
                                                 lastMatchedLanguageSubtagIndex))
        {
          matchedRange = false;
          break;
        }
        rangeSubtagsStartIndex = rangeSubtagsEndIndex;
      } while (rangeSubtagsStartIndex < rangeLength);
      if (matchedRange)
      {
        return true;
      }
    }
    return false;
  }

  bool MatchesDirPseudoClass(const dom::Element &element, const CSSOMStringAtom &argument) noexcept
  {
    switch (EffectiveTextDirection(element))
    {
      case TextDirection::LTR:
        return text::IsASCIICaselessEqual(argument.View(), dom::DOMStringView(u8"ltr"));
      case TextDirection::RTL:
        return text::IsASCIICaselessEqual(argument.View(), dom::DOMStringView(u8"rtl"));
    }

    return false;
  }

  bool MatchesReadWritePseudoClass(const dom::Element &element) noexcept
  {
    if (IsHTMLElement(element, html::TagName::input))
    {
      return ReadOnlyApplies(InputType(element)) && !element.HasAttribute(u8"readonly")
             && !IsDisabledFormControl(element);
    }
    if (IsHTMLElement(element, html::TagName::textarea))
    {
      return !element.HasAttribute(u8"readonly") && !IsDisabledFormControl(element);
    }
    return IsEditable(element);
  }

  bool MatchesIndeterminatePseudoClass(const dom::Element &element) noexcept
  {
    if (auto *input = DynamicDowncastHTML<html::HTMLInputElement>(element))
    {
      // FIXME: Radio buttons whose group has no checked button, once radio button groups exist.
      return InputType(element) == u8"checkbox" && input->Indeterminate();
    }
    if (IsHTMLElement(element, html::TagName::progress))
    {
      return !element.HasAttribute(u8"value");
    }
    return false;
  }

  dom::DOMString EffectiveLang(const dom::Element &element) noexcept
  {
    // FIXME: The document's default language (a Content-Language pragma or header).
    dom::DOMStringAtom xmlNamespace(infra::Namespaces::XML);
    for (auto *ancestor = &element; ancestor;
         ancestor = dom::TreeQueries::ParentOrShadowHostElement(*ancestor))
    {
      if (auto lang = ancestor->GetAttributeNS(xmlNamespace, u8"lang"))
      {
        return *lang;
      }
      if (auto lang = ancestor->GetAttributeNS(dom::DOMStringAtom::Null(), u8"lang"))
      {
        return *lang;
      }
    }
    return {};
  }

  TextDirection EffectiveTextDirection(const dom::Element &element) noexcept
  {
    for (auto *ancestor = &element; ancestor;
         ancestor = dom::TreeQueries::ParentOrShadowHostElement(*ancestor))
    {
      if (!Is<html::HTMLElement>(*ancestor))
      {
        continue;
      }
      auto dir =
        text::ToASCIILower(dom::DOMStringView(ancestor->GetAttribute(u8"dir").value_or(dom::DOMString())));
      if (dir == u8"ltr")
      {
        return TextDirection::LTR;
      }
      if (dir == u8"rtl")
      {
        return TextDirection::RTL;
      }
      if (dir == u8"auto" || (dir.empty() && IsHTMLElement(*ancestor, html::TagName::bdi)))
      {
        // FIXME: Auto directionality (the first strong character of the text), once there is bidi data.
        return TextDirection::LTR;
      }
      if (IsHTMLElement(*ancestor, html::TagName::input) && InputType(*ancestor) == u8"tel")
      {
        return TextDirection::LTR;
      }
    }
    return TextDirection::LTR;
  }
}
