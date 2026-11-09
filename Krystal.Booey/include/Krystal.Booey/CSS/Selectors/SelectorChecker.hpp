/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 2004-2005 Allan Sandfeld Jensen (kde@carewolf.com)
 * Copyright (C) 2006, 2007 Nicholas Shanks (webkit@nickshanks.com)
 * Copyright (C) 2005-2016 Apple Inc. All rights reserved.
 * Copyright (C) 2007 Alexey Proskuryakov <ap@webkit.org>
 * Copyright (C) 2007, 2008 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2008, 2009 Torch Mobile Inc. All rights reserved. (http://www.torchmobile.com/)
 * Copyright (c) 2011, Code Aurora Forum. All rights reserved.
 * Copyright (C) Research In Motion Limited 2011. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#pragma once

#include "Krystal.Booey/CSS/Selectors/Selector.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/PseudoElementIdentifier.hpp"
#include "Krystal.Booey/Style/Relations.hpp"
#include "Krystal.Booey/Style/Scope/ScopeOrdinal.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class ContainerNode;
  class Document;
  class Element;
  class QualifiedName;
}

namespace krys::boo::style
{
  struct SelectorMatchingState;
}

namespace krys::boo::css
{
  class SelectorList;
  class ScopeRule;

  /// @brief Matches selectors against elements.
  class SelectorChecker
  {
    KRYS_NON_COPYABLE(SelectorChecker);

    enum class MatchOutcome
    {
      SelectorMatches,
      SelectorFailsLocally,
      SelectorFailsAllSiblings,
      SelectorFailsCompletely
    };

    enum class MatchType
    {
      VirtualPseudoElementOnly,
      Element
    };

    struct MatchResult
    {
      MatchOutcome Outcome;
      MatchType Type;

      KRYS_NODISCARD static MatchResult Matches(MatchType matchType) noexcept
      {
        return {MatchOutcome::SelectorMatches, matchType};
      }

      KRYS_NODISCARD static MatchResult UpdateWithMatchType(MatchResult result, MatchType matchType) noexcept
      {
        if (matchType == MatchType::VirtualPseudoElementOnly)
        {
          result.Type = MatchType::VirtualPseudoElementOnly;
        }
        return result;
      }

      KRYS_NODISCARD static MatchResult Fails(MatchOutcome outcome) noexcept
      {
        return {outcome, MatchType::Element};
      }
    };

  public:
    enum class Mode : uint8
    {
      ResolvingStyle = 0,
      CollectingRules,
      StyleInvalidation,
      // This is used for querySelector() API
      QueryingRules
    };

    struct CheckingContext
    {
      const Mode ResolvingMode;

    private:
      Maybe<style::PseudoElementIdentifier> _requestedPseudoElement;

    public:
      List<CSSOMStringAtom> ClassList;
      RefPtr<const dom::ContainerNode> Scope;
      const dom::Element *HasScope {nullptr};
      bool MatchesAllHasScopes {false};
      style::ScopeOrdinal StyleScopeOrdinal {style::ScopeOrdinal::Element};
      style::SelectorMatchingState *SelectorMatchingState {nullptr};

      // FIXME: It would be nicer to have a separate object for return values.
      style::Relations StyleRelations;
      EnumSet<PseudoElementType> PublicPseudoElements;
      bool MatchedInsideScope {false};
      bool DisallowHasPseudoClass {false};

      explicit CheckingContext(Mode resolvingMode) noexcept : ResolvingMode(resolvingMode)
      {
      }

      void SetRequestedPseudoElement(style::PseudoElementIdentifier pseudoElementIdentifier) noexcept
      {
        _requestedPseudoElement = krys::move(pseudoElementIdentifier);
      }

      KRYS_NODISCARD const Maybe<style::PseudoElementIdentifier> &RequestedPseudoElement() const noexcept
      {
        return _requestedPseudoElement;
      }
    };

    struct LocalContext;

  private:
    bool _strictParsing;
    bool _documentIsHTML;

  public:
    explicit SelectorChecker(const dom::Document &document) noexcept;

    KRYS_NODISCARD bool Match(const Selector &selector, const dom::Element &element,
                              CheckingContext &checkingContext) const noexcept;

    KRYS_NODISCARD bool MatchHostPseudoClass(const Selector &selector, const dom::Element &element,
                                             CheckingContext &checkingContext) const noexcept;

    KRYS_NODISCARD static bool IsCommonPseudoClassSelector(const Selector *selector) noexcept;
    KRYS_NODISCARD static bool AttributeSelectorMatches(const dom::Element &element,
                                                        const dom::QualifiedName &attributeName,
                                                        const CSSOMStringAtom &attributeValue,
                                                        const Selector &selector) noexcept;

  private:
    KRYS_NODISCARD MatchResult
      MatchRecursively(CheckingContext &checkingContext, LocalContext &context,
                       EnumSet<PseudoElementType> &collectedPseudoElements) const noexcept;
    KRYS_NODISCARD bool CheckOne(CheckingContext &checkingContext, LocalContext &context,
                                 MatchType &matchType) const noexcept;
    KRYS_NODISCARD bool MatchSelectorList(CheckingContext &checkingContext, const LocalContext &context,
                                          const dom::Element &element,
                                          const SelectorList &selectorList) const noexcept;
    KRYS_NODISCARD bool MatchHasPseudoClass(CheckingContext &checkingContext, const dom::Element &element,
                                            const Selector &hasSelector) const noexcept;
  };

  inline bool SelectorChecker::IsCommonPseudoClassSelector(const Selector *selector) noexcept
  {
    if (selector->Match() != SelectorMatch::PseudoClass)
    {
      return false;
    }
    auto pseudoType = selector->PseudoClass();
    return pseudoType == SelectorPseudoClass::Link || pseudoType == SelectorPseudoClass::AnyLink
           || pseudoType == SelectorPseudoClass::Visited || pseudoType == SelectorPseudoClass::Focus;
  }
}
