#pragma once

#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/CSS/Selectors/Selector.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/FixedVector.hpp"

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo::css
{
  // The element state behind the pseudo-classes that depend on more than the tree.

  /// @brief Whether @p element is a link: an `a` or `area` element with an `href`, or an SVG `a` with an
  /// `href` or `xlink:href`.
  KRYS_NODISCARD bool IsLink(const dom::Element &element) noexcept;

  /// @brief Whether @p element is a submit button: a `button` that isn't a reset or plain button, or an
  /// `input` of type `submit` or `image`.
  /// @see https://html.spec.whatwg.org/#concept-submit-button
  KRYS_NODISCARD bool IsSubmitButton(const dom::Element &element) noexcept;
  /// @see https://html.spec.whatwg.org/#selector-default
  KRYS_NODISCARD bool MatchesDefaultPseudoClass(const dom::Element &element) noexcept;

  /// @see https://html.spec.whatwg.org/#selector-disabled
  KRYS_NODISCARD bool MatchesDisabledPseudoClass(const dom::Element &element) noexcept;

  /// @see https://html.spec.whatwg.org/#selector-enabled
  KRYS_NODISCARD bool MatchesEnabledPseudoClass(const dom::Element &element) noexcept;

  /// @see https://dom.spec.whatwg.org/#concept-element-defined
  KRYS_NODISCARD bool IsDefinedElement(const dom::Element &element) noexcept;

  /// @see https://html.spec.whatwg.org/#selector-checked
  KRYS_NODISCARD bool IsChecked(const dom::Element &element) noexcept;

  /// @see https://html.spec.whatwg.org/#selector-optional
  KRYS_NODISCARD bool IsOptionalFormControl(const dom::Element &element) noexcept;

  /// @see https://html.spec.whatwg.org/#selector-required
  KRYS_NODISCARD bool IsRequiredFormControl(const dom::Element &element) noexcept;

  /// @see https://html.spec.whatwg.org/#selector-placeholder-shown
  KRYS_NODISCARD bool IsPlaceholderShown(const dom::Element &element) noexcept;

  KRYS_NODISCARD bool ContainsLanguageSubtagMatchingRange(dom::DOMStringView language,
                                                          dom::DOMStringView range, uint32 languageLength,
                                                          uint32 &position) noexcept;

  /// @see https://drafts.csswg.org/selectors/#the-lang-pseudo
  KRYS_NODISCARD bool MatchesLangPseudoClass(const dom::Element &element,
                                             const FixedVector<PossiblyQuotedIdentifier> &langList) noexcept;

  /// @see https://drafts.csswg.org/selectors/#the-dir-pseudo
  KRYS_NODISCARD bool MatchesDirPseudoClass(const dom::Element &element,
                                            const CSSOMStringAtom &argument) noexcept;

  /// @see https://html.spec.whatwg.org/#selector-read-write
  KRYS_NODISCARD bool MatchesReadWritePseudoClass(const dom::Element &element) noexcept;

  /// @see https://html.spec.whatwg.org/#selector-read-only
  KRYS_NODISCARD inline bool MatchesReadOnlyPseudoClass(const dom::Element &element) noexcept
  {
    return !MatchesReadWritePseudoClass(element);
  }

  /// @see https://html.spec.whatwg.org/#selector-indeterminate
  KRYS_NODISCARD bool MatchesIndeterminatePseudoClass(const dom::Element &element) noexcept;

  /// @brief The language of @p element: the nearest `xml:lang` or `lang` attribute on it or an ancestor
  /// (crossing shadow boundaries); empty if there is none.
  /// @see https://html.spec.whatwg.org/#language
  KRYS_NODISCARD dom::DOMString EffectiveLang(const dom::Element &element) noexcept;

  /// @see https://html.spec.whatwg.org/#the-directionality
  KRYS_NODISCARD TextDirection EffectiveTextDirection(const dom::Element &element) noexcept;
}
