#pragma once

#include "Krystal.Booey/CSS/Selectors/MutableSelector.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorList.hpp"

namespace krys::boo::css::tests
{
  /// @brief Builds selectors the way the selector parser does, for tests that run without it.
  struct SelectorBuilder
  {
    KRYS_NODISCARD static UniquePtr<MutableSelector>
      Type(CSSOMStringView localName, CSSOMStringAtom prefix = CSSOMStringAtom::Null()) noexcept
    {
      return CreateUnique<MutableSelector>(
        dom::QualifiedName(dom::StarAtom(), prefix, CSSOMStringAtom(localName)));
    }

    KRYS_NODISCARD static UniquePtr<MutableSelector> Simple(SelectorMatch match,
                                                            CSSOMStringView value) noexcept
    {
      auto selector = CreateUnique<MutableSelector>();
      selector->SetMatch(match);
      selector->SetValue(CSSOMStringAtom(value));
      return selector;
    }

    KRYS_NODISCARD static UniquePtr<MutableSelector> Id(CSSOMStringView value) noexcept
    {
      return Simple(SelectorMatch::Id, value);
    }

    KRYS_NODISCARD static UniquePtr<MutableSelector> Class(CSSOMStringView value) noexcept
    {
      return Simple(SelectorMatch::Class, value);
    }

    KRYS_NODISCARD static UniquePtr<MutableSelector> NestingParent() noexcept
    {
      auto selector = CreateUnique<MutableSelector>();
      selector->SetMatch(SelectorMatch::NestingParent);
      return selector;
    }

    KRYS_NODISCARD static UniquePtr<MutableSelector>
      Attribute(SelectorMatch match, CSSOMStringView name, CSSOMStringView value = {},
                IsCaseSensitive caseSensitive = IsCaseSensitive(true),
                CSSOMStringAtom prefix = CSSOMStringAtom::Null()) noexcept
    {
      auto selector = CreateUnique<MutableSelector>();
      selector->SetMatch(match);
      selector->SetValue(CSSOMStringAtom(value));
      selector->SetAttribute(dom::QualifiedName(dom::StarAtom(), prefix, CSSOMStringAtom(name)),
                             caseSensitive);
      return selector;
    }

    KRYS_NODISCARD static UniquePtr<MutableSelector> PseudoClass(SelectorPseudoClass pseudoClass) noexcept
    {
      auto selector = CreateUnique<MutableSelector>();
      selector->SetMatch(SelectorMatch::PseudoClass);
      selector->SetPseudoClass(pseudoClass);
      return selector;
    }

    KRYS_NODISCARD static UniquePtr<MutableSelector> PseudoClass(SelectorPseudoClass pseudoClass,
                                                                 MutableSelectorList &&selectorList) noexcept
    {
      auto selector = PseudoClass(pseudoClass);
      selector->AdoptSelectorVector(krys::move(selectorList));
      return selector;
    }

    KRYS_NODISCARD static UniquePtr<MutableSelector> PseudoElement(SelectorPseudoElement pseudoElement,
                                                                   CSSOMStringView name) noexcept
    {
      auto selector = CreateUnique<MutableSelector>();
      selector->SetMatch(SelectorMatch::PseudoElement);
      selector->SetPseudoElement(pseudoElement);
      selector->SetValue(CSSOMStringAtom(name));
      return selector;
    }

    /// @brief Joins simple selectors into a compound selector, in source order.
    template <typename... TRest>
    KRYS_NODISCARD static UniquePtr<MutableSelector> Compound(UniquePtr<MutableSelector> first,
                                                              TRest &&...rest) noexcept
    {
      (first->PrependComplexSelectorComponent(SelectorRelation::Compounding, krys::move(rest)), ...);
      return first;
    }

    /// @brief Puts @p left before @p right in a complex selector, separated by @p relation.
    KRYS_NODISCARD static UniquePtr<MutableSelector> Combine(UniquePtr<MutableSelector> left,
                                                             SelectorRelation relation,
                                                             UniquePtr<MutableSelector> right) noexcept
    {
      right->PrependComplexSelectorComponent(relation, krys::move(left));
      return right;
    }

    template <typename... TSelectors>
    KRYS_NODISCARD static MutableSelectorList MutableList(TSelectors &&...selectors) noexcept
    {
      MutableSelectorList list;
      (list.push_back(krys::move(selectors)), ...);
      return list;
    }

    template <typename... TSelectors>
    KRYS_NODISCARD static SelectorList List(TSelectors &&...selectors) noexcept
    {
      return SelectorList(MutableList(krys::move(selectors)...));
    }
  };
}
