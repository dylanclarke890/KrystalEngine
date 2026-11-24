#pragma once

#include "Krystal.Booey/CSS/Selectors/SelectorEnums.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/Containers/Array.hpp"
#include "Krystal.Core/Containers/EnumSet.hpp"
#include "Krystal.Core/Containers/FixedList.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Memory/UniquePtr.hpp"
#include "Krystal.Core/Strings/ASCII.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  class SelectorList;

  struct SelectorParserContext;

  /// @brief What a simple selector tests.
  enum class SelectorMatch : uint8
  {
    None = 0,
    /// @brief A [type selector](https://drafts.csswg.org/selectors/#type-selectors), or `*`.
    Type,
    /// @brief An ID selector, `#id`.
    Id,
    /// @brief A class selector, `.class`.
    Class,
    PseudoClass,
    PseudoElement,
    /// @brief `[attr]`.
    AttributeExists,
    /// @brief `[attr=value]`.
    AttributeEquals,
    /// @brief `[attr|=value]`: the value, or the value followed by `-`.
    AttributeDash,
    /// @brief `[attr~=value]`: one of the whitespace-separated words.
    AttributeIncludes,
    /// @brief `[attr^=value]`.
    AttributePrefix,
    /// @brief `[attr$=value]`.
    AttributeSuffix,
    /// @brief `[attr*=value]`.
    AttributeSubstring,
    /// @brief The [nesting selector](https://drafts.csswg.org/css-nesting-1/#nest-selector), `&`.
    NestingParent,
    /// @brief The anchor element of a relative selector in `:has()`, which the parser adds on its left.
    HasScope,
    /// @brief An unparseable selector in a forgiving selector list, kept as text to be serialised.
    ForgivingUnknown,
    /// @brief Like `ForgivingUnknown`, for text containing `&`.
    ForgivingUnknownNestContaining,
  };

  consteval SelectorMatch MaxEnumValue(SelectorMatch) noexcept
  {
    return SelectorMatch::ForgivingUnknownNestContaining;
  }

  /// @brief The combinator joining a simple selector to the one on its left: in `div > p`, `p`'s relation is
  /// `Child`. A complex selector's leftmost simple selector has no left neighbour, so its relation is unused,
  /// except in relative selectors, where it holds the leading combinator.
  enum class SelectorRelation : uint8
  {
    /// @brief None: they're in the same compound selector, as in `div.class`.
    Compounding,
    /// @brief `>`.
    Child,
    /// @brief Whitespace.
    Descendant,
    /// @brief `+`.
    NextSibling,
    /// @brief `~`.
    SubsequentSibling,
    /// @brief Implicit, before a pseudo-element in the shadow tree of the left side's element, such as a
    /// user agent part.
    ShadowDescendant,
    /// @brief Implicit, before `::part()`.
    ShadowPartDescendant,
    /// @brief Implicit, before `::slotted()`.
    ShadowSlotted,
  };

  consteval SelectorRelation MaxEnumValue(SelectorRelation) noexcept
  {
    return SelectorRelation::ShadowSlotted;
  }

  enum class SelectorFlag : uint8
  {
    /// @brief An attribute selector with the `i` flag.
    CaseInsensitiveAttributeValueMatching,
    /// @brief The last simple selector of a `SelectorList`'s array.
    IsLastInSelectorList,
    /// @brief A complex selector's leftmost simple selector (`div` in `div > p`), which ends it in the array.
    IsFirstInComplexSelector,
    /// @brief A complex selector's rightmost simple selector (`p` in `div > p`), which starts it in the
    /// array.
    IsLastInComplexSelector,
    /// @brief The selector keeps its data in a `RareData`.
    HasRareData,
    /// @brief A type selector added only to carry the default namespace, which isn't serialised.
    TagIsForNamespaceRule,
    /// @brief A selector the parser added rather than the author, which isn't serialised and adds no
    /// specificity.
    IsImplicit,
    /// @brief An attribute selector with the `s` flag.
    CaseSensitiveAttributeValueMatching,
  };

  consteval SelectorFlag MaxEnumValue(SelectorFlag) noexcept
  {
    return SelectorFlag::CaseSensitiveAttributeValueMatching;
  }

  /// @brief An attribute selector's [case flag](https://drafts.csswg.org/selectors/#attribute-case).
  enum class AttributeCaseFlag : uint8
  {
    /// @brief None: the attribute's own rule applies, which for some in HTML is to ignore case.
    None,
    /// @brief `i`.
    Insensitive,
    /// @brief `s`.
    Sensitive,
  };

  enum class IsCaseSensitive : bool
  {
    No,
    Yes,
  };

  /// @brief Whether an ID or class selector matches its value lowercased, as in quirks mode.
  enum class MatchLowercase : bool
  {
    No,
    Yes,
  };

  /// @brief An identifier that may have been written as a string, as `:lang()`'s arguments can be; it's
  /// serialised as it was written.
  struct PossiblyQuotedIdentifier
  {
    CSSOMStringAtom Identifier {null};
    bool WasQuoted {false};

    KRYS_NODISCARD bool operator==(const PossiblyQuotedIdentifier &) const noexcept = default;

    KRYS_NODISCARD bool IsNull() const noexcept
    {
      return Identifier == null;
    }
  };
}

namespace krys::boo
{
  KRYS_NODISCARD inline bool operator==(const css::PossiblyQuotedIdentifier &a,
                                        const CSSOMStringAtom &b) noexcept
  {
    return a.Identifier == b;
  }
}

namespace krys::boo::css
{
  /// @brief What each kind of simple selector adds to a selector's packed
  /// [specificity](https://drafts.csswg.org/selectors/#specificity-rules), a byte per component.
  enum class SelectorSpecificityIncrement : uint32
  {
    /// @brief An ID.
    ClassA = 0x10000,
    /// @brief A class, attribute or pseudo-class.
    ClassB = 0x100,
    /// @brief A type or pseudo-element.
    ClassC = 1,
  };

  /// @brief A simple selector, in a `SelectorList`'s array of them.
  /// @note A complex selector is stored rightmost first, so `div > p` is `p` (relation `Child`) followed by
  /// `div`, and the simple selector on a selector's left is the next one in the array. Flags mark where each
  /// complex selector, and the list, ends.
  class Selector
  {
    friend class MutableSelector;
    friend class SelectorList;
    // Builds the selector arrays of the rules it splits into.
    friend class StyleRule;

    /// @brief The data of selectors that need more than a value: attribute selectors, those with arguments
    /// and those whose matching and serialised values differ.
    struct RareData : public RefCounted<RareData>
    {
      /// @brief The value an ID or class matches, lowercased in quirks mode.
      CSSOMStringAtom MatchingValue {null};
      /// @brief The value as written.
      CSSOMStringAtom SerialisingValue {null};
      /// @brief `An+B`'s A and B, for `:nth-*()`.
      int32 NthA {0};
      int32 NthB {0};
      QualifiedName Attribute;
      /// @brief `:dir()`'s and `:state()`'s argument, or `:nth-*()`'s `An+B`, serialised.
      CSSOMStringAtom Argument {null};
      /// @brief `::highlight()`'s and `::part()`'s arguments.
      FixedList<CSSOMStringAtom> ArgumentList;
      /// @brief `:lang()`'s arguments.
      FixedList<PossiblyQuotedIdentifier> LangList;
      /// @brief `:heading()`'s levels.
      FixedList<int32> IntegerList;
      /// @brief The selector list argument of `:is()`, `:where()`, `:not()`, `:has()`, `:host()`,
      /// `:nth-*(… of S)`, `::slotted()` and `::cue()`.
      UniquePtr<SelectorList> SubSelectors;

      explicit RareData(const CSSOMStringAtom &value) noexcept;

      RareData(const RareData &other) noexcept;

      ~RareData() noexcept;

      KRYS_NODISCARD bool Equals(const RareData &other) const noexcept;

      /// @brief Whether the element at 1-based position @p count matches `An+B`.
      KRYS_NODISCARD bool MatchNth(int32 count) const noexcept;
    };

  public:
    enum class VisitFunctionalPseudoClasses : bool
    {
      No,
      Yes,
    };

    enum class VisitOnlySubject : bool
    {
      No,
      Yes,
    };

    using VisitFunc = Func<bool(const Selector &)>;

    /// @brief Selects the constructor that copies a selector for a `MutableSelector`, clearing its position
    /// flags.
    struct MutableSelectorCopyTag
    {
    };

  private:
    SelectorRelation _relation {SelectorRelation::Descendant};
    SelectorMatch _match {SelectorMatch::None};
    // A SelectorPseudoClass or SelectorPseudoElement, depending on _match.
    uint8 _pseudoType {0};
    EnumSet<SelectorFlag> _flags {SelectorFlag::IsFirstInComplexSelector,
                                  SelectorFlag::IsLastInComplexSelector};

    // A type selector's name, a rare data's, or otherwise the value; each holds a reference.
    union DataUnion
    {
      StringAtomStorage Value {nullptr};
      QualifiedNameStorage *TagQualifiedName;
      RareData *Rare;
    } _data;

  public:
    Selector() noexcept = default;

    /// @brief A copy, with its own copy of any rare data.
    Selector(const Selector &other) noexcept;

    Selector(const Selector &other, MutableSelectorCopyTag) noexcept;

    /// @brief A type selector.
    /// @param tagIsForNamespaceRule Whether it only carries the default namespace.
    explicit Selector(const QualifiedName &name, bool tagIsForNamespaceRule = false) noexcept;

    Selector &operator=(const Selector &) = delete;

    KRYS_NON_MOVABLE(Selector);

    ~Selector() noexcept;

    /// @brief Serialises the complex selector this is the rightmost simple selector of, followed by
    /// @p separator and @p rightSide (which `SelectorText` passes itself as it recurses leftward).
    KRYS_NODISCARD CSSOMString SelectorText(CSSOMStringView separator = {},
                                            CSSOMStringView rightSide = {}) const noexcept;

    /// @brief The complex selector's specificity, packed as in `SelectorSpecificityIncrement`.
    KRYS_NODISCARD uint32 ComputeSpecificity() const noexcept;

    /// @brief The complex selector's specificity as (A, B, C).
    KRYS_NODISCARD Array<uint8, 3> ComputeSpecificityTuple() const noexcept;

    /// @brief Calls @p func with the simple selectors of the complex selector, breadth first, until it
    /// returns true.
    /// @param visitFunctionalPseudoClasses Whether to visit the selectors in selector list arguments too.
    /// @param visitOnlySubject Whether to stop at the end of the rightmost compound selector.
    /// @return Whether @p func returned true.
    bool VisitSimpleSelectors(
      const VisitFunc &func,
      VisitFunctionalPseudoClasses visitFunctionalPseudoClasses = VisitFunctionalPseudoClasses::No,
      VisitOnlySubject visitOnlySubject = VisitOnlySubject::No) const noexcept;

    /// @brief Whether the complex selector contains `&`, arguments included.
    KRYS_NODISCARD bool HasExplicitNestingParent() const noexcept;

    /// @brief Whether the complex selector contains `:scope`, arguments included.
    KRYS_NODISCARD bool HasExplicitPseudoClassScope() const noexcept;

    /// @brief Whether the complex selector contains `:scope`, arguments excluded.
    KRYS_NODISCARD bool HasScope() const noexcept;

    /// @brief The style's pseudo-element type for @p pseudoElement, if it has one. Element-backed
    /// pseudo-elements, which are styled as the elements they are, don't.
    KRYS_NODISCARD static Maybe<PseudoElementType>
      StylePseudoElementTypeFor(SelectorPseudoElement pseudoElement) noexcept;

    KRYS_NODISCARD static inline bool IsPseudoClassEnabled(SelectorPseudoClass type,
                                                           const SelectorParserContext &context) noexcept;

    KRYS_NODISCARD static inline bool IsPseudoElementEnabled(SelectorPseudoElement type, CSSOMStringView name,
                                                             const SelectorParserContext &context) noexcept;

    /// @brief The pseudo-element named @p name (without its colons), if it's known and enabled.
    KRYS_NODISCARD static Maybe<SelectorPseudoElement>
      ParsePseudoElementName(CSSOMStringView name, const SelectorParserContext &context) noexcept;

    KRYS_NODISCARD static inline bool PseudoClassRequiresArgument(SelectorPseudoClass type) noexcept;

    KRYS_NODISCARD static inline bool PseudoElementRequiresArgument(SelectorPseudoElement type) noexcept;

    KRYS_NODISCARD static inline bool PseudoClassMayHaveArgument(SelectorPseudoClass type) noexcept;

    KRYS_NODISCARD static inline bool PseudoElementMayHaveArgument(SelectorPseudoElement type) noexcept;

    /// @brief @p type's name, with its colon.
    KRYS_NODISCARD static inline CSSOMStringView
      SelectorTextForPseudoClass(SelectorPseudoClass type) noexcept;

#pragma region Traversal

    /// @brief The simple selector on the left, the next in the array, or null if this is the leftmost.
    KRYS_NODISCARD const Selector *PrecedingComplexSelectorComponent() const noexcept
    {
      return IsFirstInComplexSelector() ? nullptr : this + 1;
    }

    /// @brief The leftmost simple selector of this one's compound selector.
    KRYS_NODISCARD const Selector *FirstInCompoundSelector() const noexcept;

    /// @brief The rightmost simple selector of this one's compound selector.
    KRYS_NODISCARD const Selector *LastInCompoundSelector() const noexcept;

    /// @brief The simple selector on the left if it's in the same compound selector, or null.
    KRYS_NODISCARD const Selector *PrecedingCompoundSelectorComponent() const noexcept;

#pragma endregion

#pragma region Data

    KRYS_NODISCARD SelectorMatch Match() const noexcept
    {
      return _match;
    }

    KRYS_NODISCARD SelectorRelation Relation() const noexcept
    {
      return _relation;
    }

    KRYS_NODISCARD const QualifiedName &TagQualifiedName() const noexcept;

    KRYS_NODISCARD const CSSOMStringAtom &TagLocalNameLower() const noexcept;

    /// @brief The value to match: an ID, class, attribute value, or a pseudo-element's name.
    KRYS_NODISCARD const CSSOMStringAtom &Value() const noexcept;

    /// @brief The value as written, which differs from `Value` when quirks mode lowercases it.
    KRYS_NODISCARD const CSSOMStringAtom &SerialisingValue() const noexcept;

    KRYS_NODISCARD const QualifiedName &Attribute() const noexcept;

    KRYS_NODISCARD const CSSOMStringAtom &Argument() const noexcept KRYS_LIFETIME_BOUND
    {
      return HasRareData() ? _data.Rare->Argument : CSSOMStringAtom::Null();
    }

    /// @brief Whether the `s` flag makes the selector match case-sensitively, whatever the attribute.
    KRYS_NODISCARD bool IsAttributeValueMatchingCaseSensitive() const noexcept
    {
      return _flags.contains(SelectorFlag::CaseSensitiveAttributeValueMatching);
    }

    /// @brief Whether the `i` flag makes the selector match case-insensitively.
    KRYS_NODISCARD bool IsAttributeValueMatchingCaseInsensitive() const noexcept
    {
      return _flags.contains(SelectorFlag::CaseInsensitiveAttributeValueMatching);
    }

    KRYS_NODISCARD const FixedList<CSSOMStringAtom> *ArgumentList() const noexcept
    {
      return HasRareData() ? &_data.Rare->ArgumentList : nullptr;
    }

    KRYS_NODISCARD const FixedList<PossiblyQuotedIdentifier> *LangList() const noexcept
    {
      return HasRareData() ? &_data.Rare->LangList : nullptr;
    }

    /// @brief The levels of `:heading()`.
    KRYS_NODISCARD const FixedList<int32> *IntegerList() const noexcept
    {
      return HasRareData() ? &_data.Rare->IntegerList : nullptr;
    }

    /// @brief The selector list argument, if the selector has one.
    KRYS_NODISCARD const SelectorList *SubSelectors() const noexcept
    {
      return HasRareData() ? _data.Rare->SubSelectors.get() : nullptr;
    }

    KRYS_NODISCARD SelectorList *SubSelectors() noexcept
    {
      return HasRareData() ? _data.Rare->SubSelectors.get() : nullptr;
    }

    /// @brief Whether the element at 1-based position @p count matches this `:nth-*()`'s `An+B`.
    KRYS_NODISCARD bool MatchNth(int32 count) const noexcept;

    KRYS_NODISCARD int32 NthA() const noexcept;

    KRYS_NODISCARD int32 NthB() const noexcept;

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

#pragma endregion

#pragma region Queries

    KRYS_NODISCARD bool MatchesPseudoElement() const noexcept
    {
      return Match() == SelectorMatch::PseudoElement;
    }

    KRYS_NODISCARD bool IsAttributeSelector() const noexcept;

    KRYS_NODISCARD bool IsHostPseudoClass() const noexcept;

    KRYS_NODISCARD bool IsScopePseudoClass() const noexcept;

    KRYS_NODISCARD bool IsLastInSelectorList() const noexcept
    {
      return _flags.contains(SelectorFlag::IsLastInSelectorList);
    }

    KRYS_NODISCARD bool IsFirstInComplexSelector() const noexcept
    {
      return _flags.contains(SelectorFlag::IsFirstInComplexSelector);
    }

    KRYS_NODISCARD bool IsLastInComplexSelector() const noexcept
    {
      return _flags.contains(SelectorFlag::IsLastInComplexSelector);
    }

    /// @brief Whether the parser added this selector rather than the author.
    KRYS_NODISCARD bool IsImplicit() const noexcept
    {
      return _flags.contains(SelectorFlag::IsImplicit);
    }

    /// @brief Whether the two simple selectors test the same thing; their relations and positions are
    /// ignored.
    KRYS_NODISCARD bool SimpleSelectorEqual(const Selector &other) const noexcept;

#pragma endregion

  private:
    KRYS_NODISCARD bool HasRareData() const noexcept
    {
      return _flags.contains(SelectorFlag::HasRareData);
    }

    void SetFlag(SelectorFlag flag, bool value) noexcept
    {
      _flags.Set(flag, value);
    }

    void SetValue(const CSSOMStringAtom &value, MatchLowercase matchLowercase = MatchLowercase::No) noexcept;

    void SetAttribute(const QualifiedName &name, AttributeCaseFlag caseFlag) noexcept;

    void SetNth(int32 a, int32 b) noexcept;

    void SetArgument(const CSSOMStringAtom &value) noexcept;

    void SetArgumentList(FixedList<CSSOMStringAtom> argumentList) noexcept;

    void SetLangList(FixedList<PossiblyQuotedIdentifier> langList) noexcept;

    void SetIntegerList(FixedList<int32> integerList) noexcept;

    void SetSubSelectors(UniquePtr<SelectorList> subSelectors) noexcept;

    void SetPseudoClass(SelectorPseudoClass pseudoClass) noexcept
    {
      _pseudoType = ToUnderlying(pseudoClass);
    }

    void SetPseudoElement(SelectorPseudoElement pseudoElement) noexcept
    {
      _pseudoType = ToUnderlying(pseudoElement);
    }

    void SetRelation(SelectorRelation relation) noexcept
    {
      _relation = relation;
    }

    void SetMatch(SelectorMatch match) noexcept
    {
      _match = match;
    }

    void SetImplicit() noexcept
    {
      SetFlag(SelectorFlag::IsImplicit, true);
    }

    void SetLastInSelectorList() noexcept
    {
      SetFlag(SelectorFlag::IsLastInSelectorList, true);
    }

    /// @brief Moves the value into a new `RareData`, if the selector has none.
    void CreateRareData() noexcept;
  };

  /// @brief Whether the complex selector ending at @p complexSelector has a pseudo-element.
  KRYS_NODISCARD bool ComplexSelectorCanMatchPseudoElement(const Selector &complexSelector) noexcept;

  /// @brief Whether the rightmost compound selector has pseudo-elements, all of them `::slotted()`,
  /// `::part()` or user agent parts.
  KRYS_NODISCARD bool
    ComplexSelectorMatchesElementBackedPseudoElement(const Selector &complexSelector) noexcept;

  enum class ComplexSelectorsEqualMode : bool
  {
    Full,
    /// @brief Skips pseudo-elements that aren't element-backed, so `.foo::before` and `.foo` compare equal.
    IgnoreNonElementBackedPseudoElements,
  };

  /// @brief Whether two complex selectors are the same, relations included.
  KRYS_NODISCARD bool
    ComplexSelectorsEqual(const Selector &a, const Selector &b,
                          ComplexSelectorsEqualMode mode = ComplexSelectorsEqualMode::Full) noexcept;

  /// @brief A hash of a complex selector, equal for selectors `ComplexSelectorsEqual` in the same @p mode.
  KRYS_NODISCARD size_t
    HashComplexSelector(const Selector &complexSelector,
                        ComplexSelectorsEqualMode mode = ComplexSelectorsEqualMode::Full) noexcept;

  /// @brief Whether @p type depends on the element's siblings (the child-indexed and typed child-indexed
  /// pseudo-classes, and `:empty`).
  KRYS_NODISCARD constexpr bool PseudoClassIsRelativeToSiblings(SelectorPseudoClass type) noexcept
  {
    switch (type)
    {
      case SelectorPseudoClass::Empty:
      case SelectorPseudoClass::FirstChild:
      case SelectorPseudoClass::FirstOfType:
      case SelectorPseudoClass::LastChild:
      case SelectorPseudoClass::LastOfType:
      case SelectorPseudoClass::OnlyChild:
      case SelectorPseudoClass::OnlyOfType:
      case SelectorPseudoClass::NthChild:
      case SelectorPseudoClass::NthOfType:
      case SelectorPseudoClass::NthLastChild:
      case SelectorPseudoClass::NthLastOfType: return true;
      default:                                 return false;
    }
  }

  /// @brief Whether @p type is a [tree-structural
  /// pseudo-class](https://drafts.csswg.org/selectors/#structural-pseudos).
  KRYS_NODISCARD constexpr bool IsTreeStructuralPseudoClass(SelectorPseudoClass type) noexcept
  {
    return PseudoClassIsRelativeToSiblings(type) || type == SelectorPseudoClass::Root;
  }

  /// @brief Whether @p pseudoClass is a [logical
  /// combination](https://drafts.csswg.org/selectors/#logical-combination).
  KRYS_NODISCARD constexpr bool IsLogicalCombinationPseudoClass(SelectorPseudoClass pseudoClass) noexcept
  {
    switch (pseudoClass)
    {
      case SelectorPseudoClass::Is:
      case SelectorPseudoClass::Where:
      case SelectorPseudoClass::Not:
      case SelectorPseudoClass::Has:   return true;
      default:                         return false;
    }
  }

  /// @brief Whether @p pseudoElement is an [element-backed
  /// pseudo-element](https://drafts.csswg.org/css-pseudo-4/#element-backed), one that is a real element.
  KRYS_NODISCARD bool IsElementBackedPseudoElement(SelectorPseudoElement pseudoElement) noexcept;

  inline const QualifiedName &Selector::Attribute() const noexcept
  {
    krys_debug_assert(IsAttributeSelector());
    krys_debug_assert(HasRareData());
    return _data.Rare->Attribute;
  }

  inline bool Selector::IsAttributeSelector() const noexcept
  {
    switch (Match())
    {
      case SelectorMatch::AttributeExists:
      case SelectorMatch::AttributeEquals:
      case SelectorMatch::AttributeDash:
      case SelectorMatch::AttributeIncludes:
      case SelectorMatch::AttributePrefix:
      case SelectorMatch::AttributeSuffix:
      case SelectorMatch::AttributeSubstring: return true;
      default:                                return false;
    }
  }

  inline void Selector::SetValue(const CSSOMStringAtom &value, MatchLowercase matchLowercase) noexcept
  {
    krys_debug_assert(Match() != SelectorMatch::Type);
    auto matchingValue = matchLowercase == MatchLowercase::Yes ? value.ToASCIILower() : value;
    // The two values only need rare data when they differ.
    if (!HasRareData() && matchingValue != value)
    {
      CreateRareData();
    }
    if (HasRareData())
    {
      _data.Rare->MatchingValue = krys::move(matchingValue);
      _data.Rare->SerialisingValue = value;
      return;
    }
    // Take the new reference before releasing the old, which `value` may be.
    auto storage = CSSOMStringAtom(value).LeakStorage();
    CSSOMStringAtom::DerefStorage(_data.Value);
    _data.Value = storage;
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
    else
    {
      CSSOMStringAtom::DerefStorage(_data.Value);
      _data.Value = nullptr;
    }
  }

  inline const QualifiedName &Selector::TagQualifiedName() const noexcept
  {
    krys_debug_assert(Match() == SelectorMatch::Type);
    // A QualifiedName is only a pointer to its storage.
    return *reinterpret_cast<const QualifiedName *>(&_data.TagQualifiedName);
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
    // An atom is only a pointer to its interned string.
    return *reinterpret_cast<const CSSOMStringAtom *>(&_data.Value);
  }

  inline const CSSOMStringAtom &Selector::SerialisingValue() const noexcept
  {
    krys_debug_assert(Match() != SelectorMatch::Type);
    if (HasRareData())
    {
      return _data.Rare->SerialisingValue;
    }
    return *reinterpret_cast<const CSSOMStringAtom *>(&_data.Value);
  }
}
