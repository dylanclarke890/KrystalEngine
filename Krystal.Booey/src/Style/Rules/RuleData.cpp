/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 2004-2005 Allan Sandfeld Jensen (kde@carewolf.com)
 * Copyright (C) 2006, 2007 Nicholas Shanks (webkit@nickshanks.com)
 * Copyright (C) 2005-2019 Apple Inc. All rights reserved.
 * Copyright (C) 2007 Alexey Proskuryakov <ap@webkit.org>
 * Copyright (C) 2007, 2008 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2008, 2009 Torch Mobile Inc. All rights reserved. (http://www.torchmobile.com/)
 * Copyright (c) 2011, Code Aurora Forum. All rights reserved.
 * Copyright (C) Research In Motion Limited 2011. All rights reserved.
 * Copyright (C) 2012 Google Inc. All rights reserved.
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

#include "Krystal.Booey/Style/Rules/RuleData.hpp"
#include "Krystal.Booey/CSS/Selectors/Selector.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorChecker.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorList.hpp"
#include "Krystal.Booey/CSS/Selectors/UserAgentParts.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Enum.hpp"

namespace krys::boo::style
{
  namespace
  {
    struct SameSizeAsRuleData
    {
      uint64 A;
      uint32 B;
      uint32 C;
      uint32 D[4];
    };

    KRYS_NODISCARD MatchBasedOnRuleHash ComputeMatchBasedOnRuleHash(const css::Selector &selector) noexcept
    {
      if (selector.PrecedingComplexSelectorComponent())
      {
        return MatchBasedOnRuleHash::None;
      }
      if (selector.Match() == css::SelectorMatch::Type)
      {
        const auto &tagQualifiedName = selector.TagQualifiedName();
        const auto &selectorNamespace = tagQualifiedName.NamespaceURI();
        if (selectorNamespace == dom::StarAtom() || selectorNamespace.View() == infra::Namespaces::HTML)
        {
          if (tagQualifiedName == dom::AnyQualifiedName())
          {
            return MatchBasedOnRuleHash::Universal;
          }
          return MatchBasedOnRuleHash::ClassC;
        }
        return MatchBasedOnRuleHash::None;
      }
      if (css::SelectorChecker::IsCommonPseudoClassSelector(&selector))
      {
        return MatchBasedOnRuleHash::ClassB;
      }
      if (selector.Match() == css::SelectorMatch::Id)
      {
        return MatchBasedOnRuleHash::ClassA;
      }
      if (selector.Match() == css::SelectorMatch::Class)
      {
        return MatchBasedOnRuleHash::ClassB;
      }
      // FIXME: Valueless [attribute] case can be handled here too.
      return MatchBasedOnRuleHash::None;
    }

    KRYS_NODISCARD PropertyAllowlist DeterminePropertyAllowlist(const css::Selector *selector) noexcept
    {
      for (const css::Selector *component = selector; component;
           component = component->PrecedingComplexSelectorComponent())
      {
        // Property allow-list for `::cue`:
        if (component->Match() == css::SelectorMatch::PseudoElement
            && component->PseudoElement() == css::SelectorPseudoElement::UserAgentPart
            && component->Value() == css::UserAgentParts::Cue())
        {
          return PropertyAllowlist::Cue;
        }
        // Property allow-list for `::cue(selector)`:
        if (component->Match() == css::SelectorMatch::PseudoElement
            && component->PseudoElement() == css::SelectorPseudoElement::Cue)
        {
          return PropertyAllowlist::CueSelector;
        }
        // FIXME: The allow-list for '::-internal-cue-background', once there are WebVTT cues.
        if (component->Match() == css::SelectorMatch::PseudoElement
            && component->PseudoElement() == css::SelectorPseudoElement::Marker)
        {
          return PropertyAllowlistForPseudoElement(PseudoElementType::Marker);
        }

        if (const auto *selectorList = selector->SubSelectors())
        {
          for (auto &subSelector : *selectorList)
          {
            auto allowlistType = DeterminePropertyAllowlist(&subSelector);
            if (allowlistType != PropertyAllowlist::None)
            {
              return allowlistType;
            }
          }
        }
      }
      return PropertyAllowlist::None;
    }
  }

  static_assert(sizeof(RuleData) == sizeof(SameSizeAsRuleData), "RuleData should stay small");

  RuleData::RuleData(const css::StyleRule &styleRule, uint32 selectorIndex, uint32 selectorListIndex,
                     uint32 position, EnumSet<UsedRuleType> usedRuleTypes) noexcept
      : _styleRuleWithSelectorIndex(&styleRule, static_cast<uint16>(selectorIndex)),
        _selectorListIndex(selectorListIndex),
        _matchBasedOnRuleHash(ToUnderlying(ComputeMatchBasedOnRuleHash(*Selector()))),
        _canMatchPseudoElement(css::ComplexSelectorCanMatchPseudoElement(*Selector())),
        _propertyAllowlist(ToUnderlying(DeterminePropertyAllowlist(Selector()))),
        _usedRuleTypes(static_cast<uint32>(usedRuleTypes.ToRaw())), _isEnabled(true), _position(position),
        _descendantSelectorIdentifierHashes(css::SelectorFilter::CollectHashes(*Selector()))
  {
    krys_debug_assert(_position == position);
    krys_debug_assert(SelectorIndex() == selectorIndex);
  }
}
