/*
 * Copyright (C) 2014-2016 Apple Inc. All rights reserved.
 * Copyright (C) 2014 Dhi Aurrahman <diorahman@rockybars.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/Selectors/Selector.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Text/WritingMode.hpp"
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

  /// @brief Whether @p element's document is focused and active (its window has system focus).
  KRYS_NODISCARD bool IsFrameFocused(const dom::Element &element) noexcept;
  /// @see https://drafts.csswg.org/selectors/#focus-pseudo
  KRYS_NODISCARD bool MatchesFocusPseudoClass(const dom::Element &element) noexcept;
  /// @see https://drafts.csswg.org/selectors/#focus-visible-pseudo
  KRYS_NODISCARD bool MatchesFocusVisiblePseudoClass(const dom::Element &element) noexcept;
  /// @see https://drafts.csswg.org/selectors/#focus-within-pseudo
  KRYS_NODISCARD bool MatchesFocusWithinPseudoClass(const dom::Element &element) noexcept;
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

  /// @brief The language of @p element: the nearest `xml:lang` attribute, or `lang` attribute on an HTML or
  /// SVG element, on it or an ancestor (crossing shadow boundaries), or else the pragma-set default language;
  /// empty if there is none.
  /// @see https://html.spec.whatwg.org/#language
  KRYS_NODISCARD dom::DOMString EffectiveLang(const dom::Element &element) noexcept;
}
