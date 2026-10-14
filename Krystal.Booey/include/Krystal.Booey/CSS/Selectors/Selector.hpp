#pragma once

#include "Krystal.Booey/CSS/Selectors/Enums/SelectorMatch.hpp"
#include "Krystal.Booey/CSS/Selectors/Enums/SelectorPagePseudoClass.hpp"
#include "Krystal.Booey/CSS/Selectors/Enums/SelectorRelation.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorEnums.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorFlag.hpp"
#include "Krystal.Booey/CSS/Selectors/Types/IsCaseSensitive.hpp"
#include "Krystal.Booey/CSS/Selectors/Types/MatchLowercase.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/FixedVector.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::css
{
  class SelectorList;
  struct SelectorParserContext;

  struct PossiblyQuotedIdentifier
  {
    CSSOMStringAtom Identifier {CSSOMStringAtom::Null()};
    bool WasQuoted {false};

    KRYS_NODISCARD bool operator==(const PossiblyQuotedIdentifier &) const noexcept = default;

    KRYS_NODISCARD bool IsNull() const noexcept
    {
      return Identifier == CSSOMStringAtom::Null();
    }
  };

  KRYS_NODISCARD inline bool operator==(const PossiblyQuotedIdentifier &a, const CSSOMStringAtom &b) noexcept
  {
    return a.Identifier == b;
  }

  enum class SelectorSpecificityIncrement : uint32
  {
    ClassA = 0x10000,
    ClassB = 0x100,
    ClassC = 1
  };

  /// @brief Represents a simple CSS selector that is part of a SelectorList and may be part of a more
  /// complex selector. For optimisation purposes it does not store a pointer to its preceding selector
  /// component but instead relies on the knowledge that it is stored in a flattened array and can access its
  /// preceding selector component using pointer arithmetic. Various flags in `SelectorFlag` are used to
  /// determine the boundaries of the complex selectors within the selector list and the selector list itself.
  /// @note The preceding selector component is the next selector in memory, not the previous one. (e.g.
  /// `div > p` is stored as `p` followed by `div` in memory, with `p` having a relation of `Child` to `div`).
  class Selector
  {
    friend class MutableSelector;
    friend class SelectorList;

    struct RareData : public RefCounted<RareData>
    {
      // For quirks mode, class and id are case-insensitive. In the case where uppercase letters are used in
      // quirks mode, |MatchingValue| holds the lowercase class/id and |SerialisingValue| holds the original
      // string.
      CSSOMStringAtom MatchingValue {CSSOMStringAtom::Null()};
      CSSOMStringAtom SerialisingValue {CSSOMStringAtom::Null()};

      // Used for :nth-*
      int32 NthA {0};
      int32 NthB {0};

      // Used for attribute selector
      dom::QualifiedName Attribute;

      // Used for :contains and :nth-*
      CSSOMStringAtom Argument {CSSOMStringAtom::Null()};

      // Used for :active-view-transition-type, ::highlight, ::view-transition-{group, image-pair, new, old},
      // ::part arguments.
      FixedVector<CSSOMStringAtom> ArgumentList;

      // Used for :lang arguments.
      FixedVector<PossiblyQuotedIdentifier> LangList;

      // Used for :is(), :not(), :has(), :where(), :host(), :nth-*(... of) and ::slotted().
      UniquePtr<SelectorList> SubSelectors;

      explicit RareData(CSSOMStringAtom value) noexcept;

      RareData(const RareData &other) noexcept;

      ~RareData() noexcept;

      KRYS_NODISCARD bool Equals(const RareData &other) const noexcept;

      KRYS_NODISCARD bool MatchNth(int32 count) const noexcept;

      KRYS_NODISCARD Ref<RareData> DeepCopy() const noexcept;
    };

  public:
    enum class VisitFunctionalPseudoClasses : bool
    {
      No,
      Yes
    };

    enum class VisitOnlySubject : bool
    {
      No,
      Yes
    };

    using VisitFunc = Func<bool(const Selector &)>;

    struct MutableSelectorCopyTag
    {
    };

  private:
    SelectorRelation _relation {SelectorRelation::Descendant};
    SelectorMatch _match {SelectorMatch::None};
    // A SelectorPseudoClass, SelectorPseudoElement or SelectorPagePseudoClass, depending on _match.
    uint8 _pseudoType {0};
    SelectorFlag _flags {SelectorFlag::IsFirstInComplexSelector | SelectorFlag::IsLastInComplexSelector};

    union DataUnion
    {
      StringAtomStorage Value {nullptr};
      dom::QualifiedNameStorage *TagQualifiedName;
      RareData *Rare;
    } _data;

  public:
    Selector() noexcept = default;

    Selector(const Selector &other) noexcept;

    Selector(const Selector &other, MutableSelectorCopyTag) noexcept;

    explicit Selector(const dom::QualifiedName &name, bool tagIsForNamespaceRule = false) noexcept;

    Selector &operator=(const Selector &) = delete;
    KRYS_NON_MOVABLE(Selector);

    ~Selector() noexcept;

    /// @brief Re-creates the selector text from the selector's data.
    KRYS_NODISCARD CSSOMString SelectorText(CSSOMStringView separator = {},
                                            CSSOMStringView rightSide = {}) const noexcept;

    KRYS_NODISCARD uint32 ComputeSpecificity() const noexcept;
    KRYS_NODISCARD Array<uint8, 3> ComputeSpecificityTuple() const noexcept;
    KRYS_NODISCARD uint32 SpecificityForPage() const noexcept;

    bool VisitSimpleSelectors(
      VisitFunc &&func,
      VisitFunctionalPseudoClasses visitFunctionalPseudoClasses = VisitFunctionalPseudoClasses::No,
      VisitOnlySubject visitOnlySubject = VisitOnlySubject::No) const noexcept;

    KRYS_NODISCARD bool HasExplicitNestingParent() const noexcept;
    KRYS_NODISCARD bool HasExplicitPseudoClassScope() const noexcept;
    KRYS_NODISCARD bool HasScope() const noexcept;

    /// @brief Maps from the selector pseudo-element type to the style type. Only pseudo-elements that are not
    /// element-backed have a type in style.
    KRYS_NODISCARD static Maybe<PseudoElementType>
      StylePseudoElementTypeFor(SelectorPseudoElement pseudoElement) noexcept;
    KRYS_NODISCARD static inline bool IsPseudoClassEnabled(SelectorPseudoClass type,
                                                           const SelectorParserContext &context) noexcept;
    KRYS_NODISCARD static inline bool IsPseudoElementEnabled(SelectorPseudoElement type, CSSOMStringView name,
                                                             const SelectorParserContext &context) noexcept;
    KRYS_NODISCARD static Maybe<SelectorPseudoElement>
      ParsePseudoElementName(CSSOMStringView name, const SelectorParserContext &context) noexcept;
    KRYS_NODISCARD static inline bool PseudoClassRequiresArgument(SelectorPseudoClass type) noexcept;
    KRYS_NODISCARD static inline bool PseudoElementRequiresArgument(SelectorPseudoElement type) noexcept;
    KRYS_NODISCARD static inline bool PseudoClassMayHaveArgument(SelectorPseudoClass type) noexcept;
    KRYS_NODISCARD static inline bool PseudoElementMayHaveArgument(SelectorPseudoElement type) noexcept;

    KRYS_NODISCARD static inline CSSOMStringView
      SelectorTextForPseudoClass(SelectorPseudoClass type) noexcept;

#pragma region Traversal

    /// @brief Selectors are kept in an array by SelectorList. The left component of the selector is the next
    /// item in the array.
    KRYS_NODISCARD const Selector *PrecedingComplexSelectorComponent() const noexcept
    {
      return IsFirstInComplexSelector() ? nullptr : this + 1;
    }

    KRYS_NODISCARD const Selector *FirstInCompoundSelector() const noexcept;
    KRYS_NODISCARD const Selector *LastInCompoundSelector() const noexcept;
    KRYS_NODISCARD const Selector *PrecedingCompoundSelectorComponent() const noexcept;

#pragma endregion

    KRYS_NODISCARD const dom::QualifiedName &TagQualifiedName() const noexcept;
    KRYS_NODISCARD const CSSOMStringAtom &TagLocalNameLower() const noexcept;

    KRYS_NODISCARD const CSSOMStringAtom &Value() const noexcept;
    KRYS_NODISCARD const CSSOMStringAtom &SerialisingValue() const noexcept;
    KRYS_NODISCARD const dom::QualifiedName &Attribute() const noexcept;

    KRYS_NODISCARD const CSSOMStringAtom &Argument() const noexcept
    {
      return HasRareData() ? _data.Rare->Argument : CSSOMStringAtom::Null();
    }

    KRYS_NODISCARD bool IsAttributeValueMatchingCaseInsensitive() const noexcept
    {
      return HasFlag(_flags, SelectorFlag::CaseInsensitiveAttributeValueMatching);
    }

    KRYS_NODISCARD const FixedVector<CSSOMStringAtom> *ArgumentList() const noexcept
    {
      return HasRareData() ? &_data.Rare->ArgumentList : nullptr;
    }

    KRYS_NODISCARD const FixedVector<PossiblyQuotedIdentifier> *LangList() const noexcept
    {
      return HasRareData() ? &_data.Rare->LangList : nullptr;
    }

    KRYS_NODISCARD const SelectorList *SubSelectors() const noexcept
    {
      return HasRareData() ? _data.Rare->SubSelectors.get() : nullptr;
    }

    KRYS_NODISCARD SelectorList *SubSelectors() noexcept
    {
      return HasRareData() ? _data.Rare->SubSelectors.get() : nullptr;
    }

    KRYS_NODISCARD bool MatchNth(int32 count) const noexcept;
    KRYS_NODISCARD int32 NthA() const noexcept;
    KRYS_NODISCARD int32 NthB() const noexcept;

    KRYS_NODISCARD bool HasDescendantRelation() const noexcept
    {
      return Relation() == SelectorRelation::Descendant;
    }

    KRYS_NODISCARD bool HasDescendantOrChildRelation() const noexcept
    {
      return Relation() == SelectorRelation::Child || HasDescendantRelation();
    }

    KRYS_NODISCARD SelectorPseudoClass PseudoClass() const noexcept
    {
      krys_debug_assert(Match() == SelectorMatch::PseudoClass);
      return static_cast<SelectorPseudoClass>(_pseudoType);
    }

    KRYS_NODISCARD SelectorPseudoElement PseudoElement() const noexcept
    {
      krys_debug_assert(Match() == SelectorMatch::PseudoElement);
      return static_cast<SelectorPseudoElement>(_pseudoType);
    }

    KRYS_NODISCARD SelectorPagePseudoClass PagePseudoClass() const noexcept
    {
      krys_debug_assert(Match() == SelectorMatch::PagePseudoClass);
      return static_cast<SelectorPagePseudoClass>(_pseudoType);
    }

    KRYS_NODISCARD bool MatchesPseudoElement() const noexcept
    {
      return Match() == SelectorMatch::PseudoElement;
    }

    KRYS_NODISCARD bool IsSiblingSelector() const noexcept;
    KRYS_NODISCARD bool IsAttributeSelector() const noexcept;
    KRYS_NODISCARD bool IsHostPseudoClass() const noexcept;
    KRYS_NODISCARD bool IsScopePseudoClass() const noexcept;

    KRYS_NODISCARD SelectorRelation Relation() const noexcept
    {
      return _relation;
    }

    KRYS_NODISCARD SelectorMatch Match() const noexcept
    {
      return _match;
    }

    KRYS_NODISCARD bool IsLastInSelectorList() const noexcept
    {
      return HasFlag(_flags, SelectorFlag::IsLastInSelectorList);
    }

    KRYS_NODISCARD bool IsFirstInComplexSelector() const noexcept
    {
      return HasFlag(_flags, SelectorFlag::IsFirstInComplexSelector);
    }

    KRYS_NODISCARD bool IsLastInComplexSelector() const noexcept
    {
      return HasFlag(_flags, SelectorFlag::IsLastInComplexSelector);
    }

    // FIXME: This should ideally be private, but StyleRule uses it.
    void SetLastInSelectorList() noexcept
    {
      SetFlag(SelectorFlag::IsLastInSelectorList, true);
    }

    KRYS_NODISCARD bool IsForPage() const noexcept
    {
      return HasFlag(_flags, SelectorFlag::IsForPage);
    }

    /// @brief Whether this selector is not author/UA written.
    KRYS_NODISCARD bool IsImplicit() const noexcept
    {
      return HasFlag(_flags, SelectorFlag::IsImplicit);
    }

    /// @brief Relation and selector list bits are ignored.
    KRYS_NODISCARD bool SimpleSelectorEqual(const Selector &other) const noexcept;

  private:
    KRYS_NODISCARD bool HasRareData() const noexcept
    {
      return HasFlag(_flags, SelectorFlag::HasRareData);
    }

    void SetFlag(SelectorFlag flag, bool value) noexcept
    {
      _flags = value ? (_flags | flag) : (_flags & ~flag);
    }

    void SetValue(const CSSOMStringAtom &value,
                  MatchLowercase matchLowercase = MatchLowercase(false)) noexcept;

    void SetAttribute(const dom::QualifiedName &name, IsCaseSensitive caseSensitive) noexcept;
    void SetNth(int32 a, int32 b) noexcept;
    void SetArgument(const CSSOMStringAtom &value) noexcept;
    void SetArgumentList(FixedVector<CSSOMStringAtom> argumentList) noexcept;
    void SetLangList(FixedVector<PossiblyQuotedIdentifier> langList) noexcept;
    void SetSubSelectors(UniquePtr<SelectorList> subSelectors) noexcept;

    void SetPseudoClass(SelectorPseudoClass pseudoClass) noexcept
    {
      _pseudoType = ToUnderlying(pseudoClass);
    }

    void SetPseudoElement(SelectorPseudoElement pseudoElement) noexcept
    {
      _pseudoType = ToUnderlying(pseudoElement);
    }

    void SetPagePseudoClass(SelectorPagePseudoClass pagePseudoClass) noexcept
    {
      _pseudoType = ToUnderlying(pagePseudoClass);
    }

    void SetRelation(SelectorRelation relation) noexcept
    {
      _relation = relation;
    }

    void SetMatch(SelectorMatch match) noexcept
    {
      _match = match;
    }

    void SetForPage() noexcept
    {
      SetFlag(SelectorFlag::IsForPage, true);
    }

    void SetImplicit() noexcept
    {
      SetFlag(SelectorFlag::IsImplicit, true);
    }

    void CreateRareData() noexcept;
  };

  KRYS_NODISCARD bool ComplexSelectorCanMatchPseudoElement(const Selector &complexSelector) noexcept;
  KRYS_NODISCARD bool
    ComplexSelectorMatchesElementBackedPseudoElement(const Selector &complexSelector) noexcept;

  /// @brief In the IgnoreNonElementBackedPseudoElements mode `.foo::before` and `.foo` compare equal.
  enum class ComplexSelectorsEqualMode : bool
  {
    Full,
    IgnoreNonElementBackedPseudoElements
  };

  KRYS_NODISCARD bool
    ComplexSelectorsEqual(const Selector &a, const Selector &b,
                          ComplexSelectorsEqualMode mode = ComplexSelectorsEqualMode::Full) noexcept;

  /// @brief Hashes a complex selector consistently with ComplexSelectorsEqual in the same @p mode.
  KRYS_NODISCARD size_t
    HashComplexSelector(const Selector &complexSelector,
                        ComplexSelectorsEqualMode mode = ComplexSelectorsEqualMode::Full) noexcept;

  KRYS_NODISCARD constexpr bool PseudoClassIsRelativeToSiblings(SelectorPseudoClass type) noexcept
  {
    return type == SelectorPseudoClass::Empty || type == SelectorPseudoClass::FirstChild
           || type == SelectorPseudoClass::FirstOfType || type == SelectorPseudoClass::LastChild
           || type == SelectorPseudoClass::LastOfType || type == SelectorPseudoClass::OnlyChild
           || type == SelectorPseudoClass::OnlyOfType || type == SelectorPseudoClass::NthChild
           || type == SelectorPseudoClass::NthOfType || type == SelectorPseudoClass::NthLastChild
           || type == SelectorPseudoClass::NthLastOfType;
  }

  KRYS_NODISCARD constexpr bool IsTreeStructuralPseudoClass(SelectorPseudoClass type) noexcept
  {
    return PseudoClassIsRelativeToSiblings(type) || type == SelectorPseudoClass::Root;
  }

  KRYS_NODISCARD constexpr bool IsLogicalCombinationPseudoClass(SelectorPseudoClass pseudoClass) noexcept
  {
    switch (pseudoClass)
    {
      case SelectorPseudoClass::Is:
      case SelectorPseudoClass::Where:
      case SelectorPseudoClass::Not:
      case SelectorPseudoClass::Has:
      {
        return true;
      }
      default:
      {
        return false;
      }
    }
  }

  KRYS_NODISCARD bool IsElementBackedPseudoElement(SelectorPseudoElement pseudoElement) noexcept;

  inline const dom::QualifiedName &Selector::Attribute() const noexcept
  {
    krys_debug_assert(IsAttributeSelector());
    krys_debug_assert(HasRareData());
    return _data.Rare->Attribute;
  }

  inline bool Selector::IsSiblingSelector() const noexcept
  {
    return Relation() == SelectorRelation::NextSibling || Relation() == SelectorRelation::SubsequentSibling
           || (Match() == SelectorMatch::PseudoClass && PseudoClassIsRelativeToSiblings(PseudoClass()));
  }

  inline bool Selector::IsAttributeSelector() const noexcept
  {
    return Match() == SelectorMatch::AttributeEquals || Match() == SelectorMatch::AttributeExists
           || Match() == SelectorMatch::AttributeIncludes || Match() == SelectorMatch::AttributeDash
           || Match() == SelectorMatch::AttributeSubstring || Match() == SelectorMatch::AttributePrefix
           || Match() == SelectorMatch::AttributeSuffix;
  }

  inline void Selector::SetValue(const CSSOMStringAtom &value, MatchLowercase matchLowercase) noexcept
  {
    krys_debug_assert(Match() != SelectorMatch::Type);
    auto matchingValue = matchLowercase ? CSSOMStringAtom(text::ToASCIILower(value.View())) : value;
    if (!HasRareData() && matchingValue != value)
    {
      CreateRareData();
    }

    if (!HasRareData())
    {
      _data.Value = value.Storage();
      return;
    }

    _data.Rare->MatchingValue = matchingValue;
    _data.Rare->SerialisingValue = value;
  }

  inline Selector::~Selector() noexcept
  {
    if (HasRareData())
    {
      _data.Rare->SubRef();
      _data.Rare = nullptr;
      SetFlag(SelectorFlag::HasRareData, false);
    }
    else if (Match() == SelectorMatch::Type)
    {
      _data.TagQualifiedName->SubRef();
      _data.TagQualifiedName = nullptr;
      _match = SelectorMatch::None;
    }
    // Atoms are interned for the lifetime of the program, so a plain value has nothing to release.
  }

  inline const dom::QualifiedName &Selector::TagQualifiedName() const noexcept
  {
    krys_debug_assert(Match() == SelectorMatch::Type);
    // QualifiedName is just a pointer to its storage, so the cast below is safe.
    return *reinterpret_cast<const dom::QualifiedName *>(&_data.TagQualifiedName);
  }

  inline const CSSOMStringAtom &Selector::TagLocalNameLower() const noexcept
  {
    return TagQualifiedName().LocalNameLower();
  }

  inline const CSSOMStringAtom &Selector::Value() const noexcept
  {
    krys_debug_assert(Match() != SelectorMatch::Type);
    if (HasRareData())
    {
      return _data.Rare->MatchingValue;
    }

    // An atom is just a pointer to its interned string, so the cast below is safe.
    return *reinterpret_cast<const CSSOMStringAtom *>(&_data.Value);
  }

  inline const CSSOMStringAtom &Selector::SerialisingValue() const noexcept
  {
    krys_debug_assert(Match() != SelectorMatch::Type);
    if (HasRareData())
    {
      return _data.Rare->SerialisingValue;
    }

    // An atom is just a pointer to its interned string, so the cast below is safe.
    return *reinterpret_cast<const CSSOMStringAtom *>(&_data.Value);
  }
}
