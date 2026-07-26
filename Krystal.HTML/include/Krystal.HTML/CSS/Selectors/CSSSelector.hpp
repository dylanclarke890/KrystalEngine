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
#include "Krystal.Lib/Types/Func.hpp"
#include "Krystal.Lib/Types/Queue.hpp"

namespace Krys::HTML
{
  struct PossiblyQuotedIdentifier
  {
    CSSOMStringAtom Identifier;
    bool WasQuoted {false};

    KRYS_NODISCARD bool operator==(const PossiblyQuotedIdentifier &) const noexcept = default;

    KRYS_NODISCARD bool IsNull() const
    {
      return Identifier == CSSOMStringAtom::Null();
    }
  };

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

    struct RareData : public RefCounted<RareData>
    {
      // Used for :nth-*
      int64 NthA {0};
      int64 NthB {0};

      // used for attribute selector
      QualifiedName Attribute {NullQualifiedName()};

      // Used for :contains and :nth-*
      CSSOMStringAtom Argument {CSSOMStringAtom::Null()};

      // Used for :active-view-transition-type, ::highlight, ::view-transition-{group, image-pair, new, old},
      // ::part arguments.
      SmallList<CSSOMStringAtom> ArgumentList;

      // Used for :lang arguments.
      SmallList<PossiblyQuotedIdentifier> LangList;

      // Used for :is(), :matches(), and :not().
      UniquePtr<CSSSelectorList> SubSelectors;

      // For quirks mode, class and id are case-insensitive. In the case where uppercase letters are used in
      // quirks mode, |MatchingValue| holds the lowercase class/id and |SerializingValue| holds the original
      // string.
      CSSOMStringAtom MatchingValue {CSSOMStringAtom::Null()};
      CSSOMStringAtom SerializingValue {CSSOMStringAtom::Null()};

      RareData(StringAtomStorage value) noexcept;

      RareData(const RareData &other) noexcept;

      ~RareData() noexcept = default;

      bool Equals(const RareData &) const noexcept;

      bool MatchNth(int64 count) const noexcept;

      Ref<RareData> DeepCopy() const noexcept;
    };

  private:
    SelectorMatch _match : BitCount<SelectorMatch>();
    SelectorRelation _relation : BitCount<SelectorRelation>();
    CSSSelectorFlag _flags : BitCount<CSSSelectorFlag>() {CSSSelectorFlag::IsFirstInComplexSelector
                                                          | CSSSelectorFlag::IsLastInComplexSelector};
    PseudoClassId _pseudoClass : BitCount<PseudoClassId>();
    PseudoElementId _pseudoElement : BitCount<PseudoElementId>();

    union DataUnion
    {
      StringAtomStorage Value;
      RawPtr<QualifiedNameStorage> TagName;
      RawPtr<RareData> RareData;
    } _data;
    static_assert(SameType<StringAtomStorage, const utf8_string *>, "StringAtomStorage type has changed.");

  public:
    CSSSelector() noexcept = default;

    explicit CSSSelector(const QualifiedName &name) noexcept;

    ~CSSSelector() noexcept;

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

    KRYS_NODISCARD RawPtr<CSSSelectorList> SubSelectors() const noexcept
    {
      if (HasFlag(_flags, CSSSelectorFlag::HasRareData))
      {
        return _data.RareData->SubSelectors.get();
      }

      return nullptr;
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

    KRYS_NODISCARD bool SimpleSelectorEqual(const CSSSelector &other) const noexcept;

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

    void SetValue(const CSSOMStringAtom &value, MatchLowercase matchLowercase) noexcept;

    void SetAttribute(const QualifiedName &name, IsCaseSensitive caseSensitive) noexcept;

    void CreateRareData() noexcept;

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
                              VisitOnlySubject visitOnlySubject, VisitFunc &&func) const noexcept;

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

  // In the AllowNonElementBackedPseudoElements mode `.foo::before` and `.foo` compare equal.
  enum class ComplexSelectorsEqualMode : bool
  {
    Full,
    IgnoreNonElementBackedPseudoElements
  };

  KRYS_NODISCARD bool
    ComplexSelectorsEqual(const CSSSelector &a, const CSSSelector &b,
                          ComplexSelectorsEqualMode mode = ComplexSelectorsEqualMode::Full) noexcept;

  KRYS_NODISCARD bool IsElementBackedPseudoElement(PseudoElementId pseudoElement) noexcept;

}