#pragma once

#include "Krystal.Booey/Css/Selectors/Selector.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Memory/UniquePtr.hpp"

namespace krys::boo::css
{
  struct SelectorParserContext;

  class MutableSelector;
  using MutableSelectorList = List<UniquePtr<MutableSelector>>;

  /// @brief The selector parser's working form of a complex selector: a simple selector and, through
  /// `PrecedingComplexSelectorComponent`, the ones to its left, each with the relation (combinator or
  /// compounding) that joins it to the one on its left. A `SelectorList` flattens finished chains into its
  /// array.
  class MutableSelector
  {
  private:
    UniquePtr<Selector> _selector;
    UniquePtr<MutableSelector> _precedingComplexSelectorComponent;

  public:
    /// @brief Selects the constructor that copies one simple selector without its chain.
    struct SimpleSelectorTag
    {
    };

    /// @brief An empty simple selector, for the parser to fill in.
    MutableSelector() noexcept;

    /// @brief A copy of @p selector and the chain to its left.
    explicit MutableSelector(const Selector &selector) noexcept;

    /// @brief A copy of @p selector alone.
    MutableSelector(const Selector &selector, SimpleSelectorTag) noexcept;

    /// @brief A type selector.
    explicit MutableSelector(const QualifiedName &tagQualifiedName) noexcept;

    ~MutableSelector() noexcept;

    /// @brief The pseudo-class `:name`, or a legacy pseudo-element written with one colon; null if it's
    /// unknown or disabled.
    KRYS_NODISCARD static UniquePtr<MutableSelector>
      ParsePseudoClassSelector(CSSOMStringView name, const SelectorParserContext &context) noexcept;

    /// @brief The pseudo-element `::name`, or null if it's unknown or disabled.
    KRYS_NODISCARD static UniquePtr<MutableSelector>
      ParsePseudoElementSelector(CSSOMStringView name, const SelectorParserContext &context) noexcept;

    KRYS_NODISCARD UniquePtr<Selector> ReleaseSelector() noexcept
    {
      return krys::move(_selector);
    }

    KRYS_NODISCARD const Selector *GetSelector() const noexcept
    {
      return _selector.get();
    }

    KRYS_NODISCARD Selector *GetSelector() noexcept
    {
      return _selector.get();
    }

    void SetValue(const CSSOMStringAtom &value, MatchLowercase matchLowercase = MatchLowercase::No) noexcept
    {
      _selector->SetValue(value, matchLowercase);
    }

    KRYS_NODISCARD const CSSOMStringAtom &Value() const noexcept
    {
      return _selector->Value();
    }

    void SetAttribute(const QualifiedName &name, AttributeCaseFlag caseFlag) noexcept
    {
      _selector->SetAttribute(name, caseFlag);
    }

    void SetArgument(const CSSOMStringAtom &value) noexcept
    {
      _selector->SetArgument(value);
    }

    void SetNth(int32 a, int32 b) noexcept
    {
      _selector->SetNth(a, b);
    }

    void SetMatch(SelectorMatch match) noexcept
    {
      _selector->SetMatch(match);
    }

    void SetRelation(SelectorRelation relation) noexcept
    {
      _selector->SetRelation(relation);
    }

    KRYS_NODISCARD SelectorMatch Match() const noexcept
    {
      return _selector->Match();
    }

    KRYS_NODISCARD SelectorPseudoElement PseudoElement() const noexcept
    {
      return _selector->PseudoElement();
    }

    KRYS_NODISCARD const SelectorList *SubSelectors() const noexcept
    {
      return _selector->SubSelectors();
    }

    void SetPseudoElement(SelectorPseudoElement type) noexcept
    {
      _selector->SetPseudoElement(type);
    }

    void SetPseudoClass(SelectorPseudoClass type) noexcept
    {
      _selector->SetPseudoClass(type);
    }

    /// @brief Sets the arguments of a functional pseudo-class, such as `:is()`, to @p selectorVector.
    void AdoptSelectorVector(MutableSelectorList &&selectorVector) noexcept;

    void SetArgumentList(FixedList<CSSOMStringAtom> list) noexcept;

    void SetLangList(FixedList<PossiblyQuotedIdentifier> list) noexcept;

    void SetIntegerList(FixedList<int32> list) noexcept;

    void SetSubSelectors(UniquePtr<SelectorList> selectorList) noexcept;

    void SetImplicit() noexcept
    {
      _selector->SetImplicit();
    }

    KRYS_NODISCARD SelectorPseudoClass PseudoClass() const noexcept
    {
      return _selector->PseudoClass();
    }

    /// @brief Whether this simple selector, or one in its arguments, matches a pseudo-element.
    KRYS_NODISCARD bool MatchesPseudoElement() const noexcept;

    KRYS_NODISCARD bool IsHostPseudoClass() const noexcept
    {
      return _selector->IsHostPseudoClass();
    }

    /// @brief Whether the chain contains `&`.
    KRYS_NODISCARD bool HasExplicitNestingParent() const noexcept;

    /// @brief Whether the chain contains `:scope`.
    KRYS_NODISCARD bool HasExplicitPseudoClassScope() const noexcept;

    /// @brief Whether this is a pseudo-element in an element's shadow tree (`::part()`, `::slotted()`, a
    /// user agent part or `::cue`). It's matched across a shadow combinator from its host, which the parser
    /// adds, along with a universal selector to carry it when nothing precedes the pseudo-element.
    KRYS_NODISCARD bool NeedsImplicitShadowCombinatorForMatching() const noexcept
    {
      return Match() == SelectorMatch::PseudoElement
             && (PseudoElement() == SelectorPseudoElement::UserAgentPart
                 || PseudoElement() == SelectorPseudoElement::Cue
                 || PseudoElement() == SelectorPseudoElement::Part
                 || PseudoElement() == SelectorPseudoElement::Slotted);
    }

    /// @brief The simple selector to the left of this one, or null if it's the leftmost.
    KRYS_NODISCARD MutableSelector *PrecedingComplexSelectorComponent() const noexcept
    {
      return _precedingComplexSelectorComponent.get();
    }

    KRYS_NODISCARD MutableSelector *LeftmostSimpleSelector() noexcept;

    KRYS_NODISCARD const MutableSelector *LeftmostSimpleSelector() const noexcept;

    /// @brief Whether the chain is a relative selector starting with a combinator other than a descendant
    /// one, such as `> a` in `:has(> a)`.
    KRYS_NODISCARD bool StartsWithExplicitCombinator() const noexcept;

    void SetPrecedingComplexSelectorComponent(UniquePtr<MutableSelector> selector) noexcept
    {
      _precedingComplexSelectorComponent = krys::move(selector);
    }

    /// @brief Puts @p selector to the left of the chain, joined by @p relation.
    void PrependComplexSelectorComponent(SelectorRelation relation,
                                         UniquePtr<MutableSelector> selector) noexcept;

    /// @brief Puts @p selector to the left of a relative selector, joined by its leading combinator, or by a
    /// descendant combinator if it has none.
    void PrependComplexSelectorComponentAsRelative(UniquePtr<MutableSelector> selector) noexcept;

    /// @brief Makes a type selector for @p tagQualifiedName this compound's last simple selector, moving the
    /// current one to its left.
    /// @param tagIsForNamespaceRule Whether the type selector only carries the default namespace.
    void AppendTagInComplexSelector(const QualifiedName &tagQualifiedName,
                                    bool tagIsForNamespaceRule = false) noexcept;

    /// @brief Detaches and returns the chain to the left, leaving this simple selector compounding.
    KRYS_NODISCARD UniquePtr<MutableSelector> ReleaseFromComplexSelector() noexcept;

  private:
    /// @brief Whether @p predicate holds for any simple selector in the chain.
    template <typename TPredicate>
    KRYS_NODISCARD bool AnyInChain(TPredicate predicate) const noexcept
    {
      for (const auto *selector = this; selector; selector = selector->PrecedingComplexSelectorComponent())
      {
        if (predicate(*selector->_selector))
        {
          return true;
        }
      }
      return false;
    }
  };
}
