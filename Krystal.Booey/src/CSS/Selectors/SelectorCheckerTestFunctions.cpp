#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/HTMLTextFormControlElement.hpp"
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
  }

  bool IsFrameFocused(const dom::Element &element) noexcept
  {
    return element.NodeDocument().IsFocusedAndActive();
  }

  namespace
  {
    KRYS_NODISCARD bool DoesShadowTreeContainFocusedElement(const dom::Element &element) noexcept
    {
      auto *shadowRoot = dom::TreeQueries::GetShadowRoot(element);
      return shadowRoot && shadowRoot->ContainsFocusedElement();
    }
  }

  bool MatchesFocusPseudoClass(const dom::Element &element) noexcept
  {
    return (element.Focused() || DoesShadowTreeContainFocusedElement(element)) && IsFrameFocused(element);
  }

  bool MatchesFocusVisiblePseudoClass(const dom::Element &element) noexcept
  {
    return element.HasFocusVisible() && IsFrameFocused(element);
  }

  bool MatchesFocusWithinPseudoClass(const dom::Element &element) noexcept
  {
    return element.HasFocusWithin() && IsFrameFocused(element);
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
    return element.MatchesDefaultPseudoClass();
  }

  bool MatchesDisabledPseudoClass(const dom::Element &element) noexcept
  {
    auto *htmlElement = DynamicDowncast<html::HTMLElement>(element);
    return htmlElement && htmlElement->IsActuallyDisabled();
  }

  bool MatchesEnabledPseudoClass(const dom::Element &element) noexcept
  {
    auto *htmlElement = DynamicDowncast<html::HTMLElement>(element);
    return htmlElement && htmlElement->CanBeActuallyDisabled() && !htmlElement->IsActuallyDisabled();
  }

  bool IsDefinedElement(const dom::Element &element) noexcept
  {
    auto state = element.GetCustomElementState();
    return state == dom::CustomElementState::Custom || state == dom::CustomElementState::Uncustomized;
  }

  bool IsChecked(const dom::Element &element) noexcept
  {
    // Even though WinIE allows checked and indeterminate to co-exist, the CSS selector spec says that you
    // can't be both checked and indeterminate. We will behave like WinIE behind the scenes and just obey the
    // CSS spec here in the test for matching the pseudo.
    if (auto *input = DynamicDowncastHTML<html::HTMLInputElement>(element))
    {
      return input->MatchesCheckedPseudoClass();
    }
    if (auto *option = DynamicDowncastHTML<html::HTMLOptionElement>(element))
    {
      return option->Selected(html::AllowStyleInvalidation::No);
    }
    return false;
  }

  bool IsOptionalFormControl(const dom::Element &element) noexcept
  {
    return element.IsOptionalFormControl();
  }

  bool IsRequiredFormControl(const dom::Element &element) noexcept
  {
    return element.IsRequiredFormControl();
  }

  bool IsPlaceholderShown(const dom::Element &element) noexcept
  {
    auto *formControl = DynamicDowncastHTML<html::HTMLTextFormControlElement>(element);
    return formControl && formControl->IsPlaceholderVisible();
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
    return element.MatchesReadWritePseudoClass();
  }

  bool MatchesIndeterminatePseudoClass(const dom::Element &element) noexcept
  {
    return element.MatchesIndeterminatePseudoClass();
  }

  dom::DOMString EffectiveLang(const dom::Element &element) noexcept
  {
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
    // Otherwise, the document's pragma-set default language, if any.
    // FIXME: The Content-Language header, once responses have headers.
    return element.NodeDocument().ContentLanguage();
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
      if (auto *input = DynamicDowncastHTML<html::HTMLInputElement>(*ancestor);
          input && input->IsTelephoneField())
      {
        return TextDirection::LTR;
      }
    }
    return TextDirection::LTR;
  }
}
