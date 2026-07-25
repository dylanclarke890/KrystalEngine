#pragma once

#include "Krystal.HTML/CSS/Selectors/Enums/CSSSelectorFlag.hpp"
#include "Krystal.HTML/CSS/Selectors/Enums/PseudoClassId.hpp"
#include "Krystal.HTML/CSS/Selectors/Enums/PseudoElementId.hpp"
#include "Krystal.HTML/CSS/Selectors/Enums/SelectorMatch.hpp"
#include "Krystal.HTML/CSS/Selectors/Enums/SelectorRelation.hpp"
#include "Krystal.HTML/CSS/Selectors/Types/IsCaseSensitive.hpp"
#include "Krystal.HTML/CSS/Selectors/Types/MatchLowercase.hpp"
#include "Krystal.HTML/CSS/Types/CSSOMString.hpp"
#include "Krystal.HTML/DOM/Types/QualifiedName.hpp"
#include "Krystal.Lib/Types/Queue.hpp"

namespace Krys::HTML
{
  /// @brief Represents a simple CSS selector that is part of a CSSSelectorList and may be part of a more
  /// complex selector. For optimisation purposes it does not store a pointer to it's preceding selector
  /// component but instead relies on the knowledge that it is stored in a flattened array and can access it's
  /// preceding selector component using pointer arithmetic. Various flags in `CSSSelectorFlag` are used to
  /// determine the boundaries of the complex selectors within the selector list and the selector list itself.
  /// @note The preceding selector component is the next selector in memory, not the previous one. (e.g.
  /// `div > p` is stored as `p` followed by `div` in memory, with `p` having a relation of `Child` to `div`).
  class CSSSelector
  {
    friend class MutableCSSSelector;
    friend class CSSSelectorList;

  private:
    SelectorMatch _match : BitCount<SelectorMatch>();
    SelectorRelation _relation : BitCount<SelectorRelation>();
    CSSSelectorFlag _flags : BitCount<CSSSelectorFlag>();
    PseudoClassId _pseudoClass : BitCount<PseudoClassId>();
    PseudoElementId _pseudoElement : BitCount<PseudoElementId>();
    QualifiedName _tagName {CSSOMStringAtom::Null(), CSSOMStringAtom::Null(), CSSOMStringAtom::Null()};
    QualifiedName _attributeName {CSSOMStringAtom::Null(), CSSOMStringAtom::Null(), CSSOMStringAtom::Null()};
    CSSOMStringAtom _matchingValue {CSSOMStringAtom::Null()};
    CSSOMStringAtom _serializingValue {CSSOMStringAtom::Null()};

  public:
    CSSSelector() noexcept = default;

    explicit CSSSelector(const QualifiedName &name) noexcept : CSSSelector()
    {
      _tagName = name;
    }

    KRYS_NODISCARD SelectorMatch Match() const noexcept
    {
      return _match;
    }

    KRYS_NODISCARD SelectorRelation Relation() const noexcept
    {
      return _relation;
    }

    KRYS_NODISCARD PseudoClassId PseudoClass() const noexcept
    {
      assert(_match == SelectorMatch::PseudoClass);
      return _pseudoClass;
    }

    KRYS_NODISCARD PseudoElementId PseudoElement() const noexcept
    {
      assert(_match == SelectorMatch::PseudoElement);
      return _pseudoElement;
    }

    KRYS_NODISCARD bool IsAttributeValueMatchingCaseInsensitive() const noexcept
    {
      return HasFlag(_flags, CSSSelectorFlag::CaseInsensitiveAttributeValueMatching);
    }

#pragma region Traversal

    KRYS_NODISCARD bool IsFirstInComplexSelector() const noexcept
    {
      return HasFlag(_flags, CSSSelectorFlag::IsFirstInComplexSelector);
    }

    KRYS_NODISCARD bool IsLastInComplexSelector() const noexcept
    {
      return HasFlag(_flags, CSSSelectorFlag::IsLastInComplexSelector);
    }

    KRYS_NODISCARD bool IsLastInSelectorList() const noexcept
    {
      return HasFlag(_flags, CSSSelectorFlag::IsLastInSelectorList);
    }

    KRYS_NODISCARD RawPtr<const CSSSelector> PrecedingComplexSelectorComponent() const noexcept
    {
      if (IsFirstInComplexSelector())
      {
        return nullptr;
      }

      return this + 1;
    }

    KRYS_NODISCARD RawPtr<const CSSSelector> FirstInCompoundSelector() const noexcept
    {
      auto *selector = this;
      while (!selector->IsFirstInComplexSelector())
      {
        if (selector->Relation() != SelectorRelation::Compounding)
        {
          break;
        }

        ++selector;
      }
      return selector;
    }

    KRYS_NODISCARD RawPtr<const CSSSelector> LastInCompoundSelector() const noexcept
    {
      auto *selector = this;
      while (!selector->IsLastInComplexSelector())
      {
        auto *next = selector - 1;
        if (next->Relation() != SelectorRelation::Compounding)
        {
          break;
        }

        selector = next;
      }
      return selector;
    }

    KRYS_NODISCARD RawPtr<const CSSSelector> PrecedingCompoundSelectorComponent() const noexcept
    {
      if (Relation() != SelectorRelation::Compounding)
      {
        return nullptr;
      }

      return PrecedingComplexSelectorComponent();
    }

#pragma endregion

  private:
    void SetMatch(SelectorMatch match) noexcept
    {
      _match = match;
    }

    void SetRelation(SelectorRelation relation) noexcept
    {
      _relation = relation;
    }

    void SetPseudoElement(PseudoElementId type) noexcept
    {
      _pseudoElement = type;
    }

    void SetPseudoClass(PseudoClassId type) noexcept
    {
      _pseudoClass = type;
    }

    void SetIsLastInSelectorList(bool value) noexcept
    {
      if (value)
      {
        _flags = _flags | CSSSelectorFlag::IsLastInSelectorList;
      }
      else
      {
        _flags = _flags & ~CSSSelectorFlag::IsLastInSelectorList;
      }
    }

    void SetIsFirstInComplexSelector(bool value) noexcept
    {
      if (value)
      {
        _flags = _flags | CSSSelectorFlag::IsFirstInComplexSelector;
      }
      else
      {
        _flags = _flags & ~CSSSelectorFlag::IsFirstInComplexSelector;
      }
    }

    void SetIsLastInComplexSelector(bool value) noexcept
    {
      if (value)
      {
        _flags = _flags | CSSSelectorFlag::IsLastInComplexSelector;
      }
      else
      {
        _flags = _flags & ~CSSSelectorFlag::IsLastInComplexSelector;
      }
    }

    void SetValue(const CSSOMStringAtom &value, MatchLowercase matchLowercase) noexcept
    {
      assert(Match() != SelectorMatch::Type);

      auto matchingValue = value;
      if (matchLowercase)
      {
        matchingValue = Krys::Text::ToASCIILowercase(value.View());
      }

      _matchingValue = Krys::Move(matchingValue);
      _serializingValue = value;
    }

    void SetAttribute(const QualifiedName &name, IsCaseSensitive caseSensitive) noexcept
    {
      _attributeName = name;

      if (caseSensitive)
      {
        _flags = _flags | CSSSelectorFlag::CaseInsensitiveAttributeValueMatching;
      }
    }

#pragma region Traversal

    struct VisitFunctionalPseudoClasses : public StronglyTypedBool<VisitFunctionalPseudoClasses>
    {
      using Base::Base;
    };

    struct VisitOnlySubject : public StronglyTypedBool<VisitOnlySubject>
    {
      using Base::Base;
    };

    using VisitFunc = Func<bool(const CSSSelector &selector)>;

    bool VisitSimpleSelectors(VisitFunctionalPseudoClasses visitFunctionalPseudoClasses,
                              VisitOnlySubject visitOnlySubject, VisitFunc &&func) const noexcept
    {
      Queue<RawPtr<const CSSSelector>> worklist;
      worklist.push(this);

      while (!worklist.empty())
      {
        auto current = worklist.front();
        worklist.pop();

        // Effective C++ advices for this cast to deal with generic const/non-const member function.
        if (func(*const_cast<RawPtr<CSSSelector>>(current)))
        {
          return true;
        }

        // TODO: Visit the selector list member (if any) recursively (such as: :has(<list>), :is(<list>),...)
        if (visitFunctionalPseudoClasses)
        {
          // if (auto selectorList = current->selectorList())
          // {
          //   for (auto &selector : *selectorList)
          //   {
          //     worklist.push(&selector);
          //   }
          // }
        }

        // Visit the next simple selector
        if (auto next = current->PrecedingComplexSelectorComponent())
        {
          // We stop visiting at the end of the compound selector (= when relation is anything else than
          // subselector) if we are in subject only mode.
          if (current->Relation() != SelectorRelation::Compounding || !visitOnlySubject)
          {
            worklist.push(next);
          }
        }
      }
      return false;
    }

#pragma endregion

#pragma region Queries

    bool HasExplicitNestingParent() const
    {
      return VisitSimpleSelectors(VisitFunctionalPseudoClasses(true), VisitOnlySubject(false),
                                  [](const CSSSelector &selector)
                                  {
                                    if (selector.Match() == SelectorMatch::NestingParent)
                                    {
                                      return true;
                                    }

                                    if (selector.Match() == SelectorMatch::ForgivingUnknownNestContaining)
                                    {
                                      return true;
                                    }

                                    return false;
                                  });
    }

#pragma endregion
  };
}