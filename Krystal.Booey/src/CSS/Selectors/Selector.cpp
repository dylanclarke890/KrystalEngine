#include "Krystal.Booey/CSS/Selectors/Selector.hpp"
#include "Krystal.Booey/CSS/CSSOM/Markup.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorInlines.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorList.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorPseudoTypeMap.hpp"
#include "Krystal.Booey/CSS/Selectors/UserAgentParts.hpp"
#include "Krystal.Core/Containers/Queue.hpp"
#include "Krystal.Core/Hashing/FastHash.hpp"
#include "Krystal.Core/Strings/StringBuilder.hpp"
#include <charconv>

namespace krys::boo::css
{
  namespace
  {
    struct SameSizeAsSelector
    {
      uint32 Flags;
      void *UnionPointer;
    };
  }

  static_assert(SelectorRelation::Compounding == static_cast<SelectorRelation>(0u),
                "Compounding must be 0 for ConsumeCombinator.");
  static_assert(sizeof(Selector) == sizeof(SameSizeAsSelector), "Selector should remain small.");
  static_assert(sizeof(CSSOMStringAtom) == sizeof(StringAtomStorage), "Selector::Value() casts the union.");
  static_assert(sizeof(QualifiedName) == sizeof(QualifiedNameStorage *),
                "Selector::TagQualifiedName() casts the union.");

  Selector::Selector(const QualifiedName &name, bool tagIsForNamespaceRule) noexcept
      : _match(SelectorMatch::Type)
  {
    SetFlag(SelectorFlag::TagIsForNamespaceRule, tagIsForNamespaceRule);
    _data.TagQualifiedName = name.get();
    _data.TagQualifiedName->AddRef();
  }

  Selector::Selector(const Selector &other) noexcept
      : _relation(other._relation), _match(other._match), _pseudoType(other._pseudoType), _flags(other._flags)
  {
    // The union's pointers are raw, so their references are taken by hand.
    if (other.HasRareData())
    {
      _data.Rare = CreateRef<RareData>(*other._data.Rare).release();
    }
    else if (other.Match() == SelectorMatch::Type)
    {
      _data.TagQualifiedName = other._data.TagQualifiedName;
      _data.TagQualifiedName->AddRef();
    }
    else
    {
      _data.Value = other._data.Value;
      CSSOMStringAtom::RefStorage(_data.Value);
    }
  }

  Selector::Selector(const Selector &other, MutableSelectorCopyTag) noexcept : Selector(other)
  {
    // A MutableSelector holds one simple selector, so its position in a list is reset.
    SetFlag(SelectorFlag::IsLastInSelectorList, false);
    SetFlag(SelectorFlag::IsFirstInComplexSelector, true);
    SetFlag(SelectorFlag::IsLastInComplexSelector, true);
  }

  void Selector::CreateRareData() noexcept
  {
    krys_debug_assert(Match() != SelectorMatch::Type);
    if (HasRareData())
    {
      return;
    }
    _data.Rare = CreateRef<RareData>(CSSOMStringAtom::AdoptStorage(_data.Value)).release();
    SetFlag(SelectorFlag::HasRareData, true);
  }

#pragma region Specificity

  namespace
  {
    /// @brief A packed specificity whose components saturate at 255 rather than carrying.
    struct SelectorSpecificity
    {
      uint32 Specificity {0};

      SelectorSpecificity() noexcept = default;

      SelectorSpecificity(uint32 specificity) noexcept : Specificity(specificity)
      {
      }

      SelectorSpecificity(SelectorSpecificityIncrement increment) noexcept
          : Specificity(ToUnderlying(increment))
      {
      }

      SelectorSpecificity &operator+=(SelectorSpecificity other) noexcept
      {
        for (uint32 mask : {0xFF0000u, 0xFF00u, 0xFFu})
        {
          const uint32 otherComponent = other.Specificity & mask;
          if (((Specificity & mask) + otherComponent) & ~mask)
          {
            Specificity |= mask;
          }
          else
          {
            Specificity += otherComponent;
          }
        }
        return *this;
      }

      KRYS_NODISCARD Array<uint8, 3> SpecificityTuple() const noexcept
      {
        return {static_cast<uint8>(Specificity >> 16), static_cast<uint8>(Specificity >> 8),
                static_cast<uint8>(Specificity)};
      }
    };

    KRYS_NODISCARD SelectorSpecificity operator+(SelectorSpecificity a, SelectorSpecificity b) noexcept
    {
      return a += b;
    }

    enum class IgnorePseudoElement : bool
    {
      No,
      Yes,
    };

    KRYS_NODISCARD SelectorSpecificity SimpleSelectorSpecificity(
      const Selector &simpleSelector, IgnorePseudoElement ignorePseudoElement) noexcept;

    KRYS_NODISCARD SelectorSpecificity ComplexSelectorSpecificity(
      const Selector &rightmost, IgnorePseudoElement ignorePseudoElement = IgnorePseudoElement::No) noexcept
    {
      SelectorSpecificity total;
      for (const auto *selector = &rightmost; selector;
           selector = selector->PrecedingComplexSelectorComponent())
      {
        total += SimpleSelectorSpecificity(*selector, ignorePseudoElement);
      }
      return total;
    }

    /// @brief The specificity of the most specific selector in @p selectorList, or zero if it's null.
    KRYS_NODISCARD SelectorSpecificity
      MaxSpecificity(const SelectorList *selectorList,
                     IgnorePseudoElement ignorePseudoElement = IgnorePseudoElement::No) noexcept
    {
      uint32 max = 0;
      if (selectorList)
      {
        for (const auto &selector : *selectorList)
        {
          max = std::max(max, ComplexSelectorSpecificity(selector, ignorePseudoElement).Specificity);
        }
      }
      return max;
    }

    // https://drafts.csswg.org/selectors/#specificity-rules
    SelectorSpecificity SimpleSelectorSpecificity(const Selector &simpleSelector,
                                                  IgnorePseudoElement ignorePseudoElement) noexcept
    {
      if (simpleSelector.IsImplicit()) KRYS_UNLIKELY
      {
        return 0;
      }

      switch (simpleSelector.Match())
      {
        case SelectorMatch::Id: return SelectorSpecificityIncrement::ClassA;
        case SelectorMatch::NestingParent:
          // A nested rule's & is resolved to its parent's selectors (an :is()) first; one left here has no
          // parent rule, "or zero if no such nesting rule exists".
          // https://drafts.csswg.org/css-nesting-1/#nest-selector
          return 0;
        case SelectorMatch::PseudoClass:
          switch (simpleSelector.PseudoClass())
          {
            // :is() can't match pseudo-elements, so any in its arguments don't count.
            case SelectorPseudoClass::Is:
              return MaxSpecificity(simpleSelector.SubSelectors(), IgnorePseudoElement::Yes);
            case SelectorPseudoClass::Not:
            case SelectorPseudoClass::Has:   return MaxSpecificity(simpleSelector.SubSelectors());
            case SelectorPseudoClass::Where: return 0;
            case SelectorPseudoClass::NthChild:
            case SelectorPseudoClass::NthLastChild:
            case SelectorPseudoClass::Host:
              return SelectorSpecificity(SelectorSpecificityIncrement::ClassB)
                     + MaxSpecificity(simpleSelector.SubSelectors());
            default: return SelectorSpecificityIncrement::ClassB;
          }
        case SelectorMatch::Class:
        case SelectorMatch::AttributeExists:
        case SelectorMatch::AttributeEquals:
        case SelectorMatch::AttributeDash:
        case SelectorMatch::AttributeIncludes:
        case SelectorMatch::AttributePrefix:
        case SelectorMatch::AttributeSuffix:
        case SelectorMatch::AttributeSubstring: return SelectorSpecificityIncrement::ClassB;
        case SelectorMatch::Type:
          return simpleSelector.TagQualifiedName().LocalName() == dom::StarAtom()
                   ? SelectorSpecificity()
                   : SelectorSpecificity(SelectorSpecificityIncrement::ClassC);
        case SelectorMatch::PseudoElement:
          if (ignorePseudoElement == IgnorePseudoElement::Yes)
          {
            return 0;
          }
          // ::slotted() only competes with other ::slotted() selectors, so leaving out its own C is
          // unobservable.
          if (simpleSelector.PseudoElement() == SelectorPseudoElement::Slotted)
          {
            return MaxSpecificity(simpleSelector.SubSelectors());
          }
          return SelectorSpecificityIncrement::ClassC;
        case SelectorMatch::HasScope:
        case SelectorMatch::None:
        case SelectorMatch::ForgivingUnknown:
        case SelectorMatch::ForgivingUnknownNestContaining: return 0;
      }
      krys_debug_assert(false);
      return 0;
    }
  }

  uint32 Selector::ComputeSpecificity() const noexcept
  {
    return ComplexSelectorSpecificity(*this).Specificity;
  }

  Array<uint8, 3> Selector::ComputeSpecificityTuple() const noexcept
  {
    return ComplexSelectorSpecificity(*this).SpecificityTuple();
  }

#pragma endregion

#pragma region Pseudo-elements

  Maybe<PseudoElementType> Selector::StylePseudoElementTypeFor(SelectorPseudoElement pseudoElement) noexcept
  {
    switch (pseudoElement)
    {
      case SelectorPseudoElement::FirstLine:     return PseudoElementType::FirstLine;
      case SelectorPseudoElement::FirstLetter:   return PseudoElementType::FirstLetter;
      case SelectorPseudoElement::GrammarError:  return PseudoElementType::GrammarError;
      case SelectorPseudoElement::SpellingError: return PseudoElementType::SpellingError;
      case SelectorPseudoElement::Selection:     return PseudoElementType::Selection;
      case SelectorPseudoElement::TargetText:    return PseudoElementType::TargetText;
      case SelectorPseudoElement::Highlight:     return PseudoElementType::Highlight;
      case SelectorPseudoElement::Marker:        return PseudoElementType::Marker;
      case SelectorPseudoElement::Backdrop:      return PseudoElementType::Backdrop;
      case SelectorPseudoElement::Before:        return PseudoElementType::Before;
      case SelectorPseudoElement::After:         return PseudoElementType::After;
      case SelectorPseudoElement::Checkmark:     return PseudoElementType::Checkmark;
      case SelectorPseudoElement::PickerIcon:    return PseudoElementType::PickerIcon;
      case SelectorPseudoElement::Cue:
      case SelectorPseudoElement::Slotted:
      case SelectorPseudoElement::Part:
      case SelectorPseudoElement::UserAgentPart: return {};
    }
    krys_debug_assert(false);
    return {};
  }

  Maybe<SelectorPseudoElement> Selector::ParsePseudoElementName(CSSOMStringView name,
                                                                const SelectorParserContext &context) noexcept
  {
    if (name.empty())
    {
      return {};
    }
    auto type = FindPseudoElementName(name);
    if (!type || !IsPseudoElementEnabled(*type, name, context))
    {
      return {};
    }
    return type;
  }

  bool IsElementBackedPseudoElement(SelectorPseudoElement pseudoElement) noexcept
  {
    switch (pseudoElement)
    {
      case SelectorPseudoElement::Part:
      case SelectorPseudoElement::Slotted:
      case SelectorPseudoElement::UserAgentPart:
      case SelectorPseudoElement::Cue:           return true;
      default:                                   return false;
    }
  }

  bool ComplexSelectorCanMatchPseudoElement(const Selector &complexSelector) noexcept
  {
    // The parser rejects pseudo-elements in selector list arguments; nesting resolution can put one in an
    // :is(), but that matches nothing, so those aren't searched.
    for (const auto *selector = &complexSelector; selector;
         selector = selector->PrecedingComplexSelectorComponent())
    {
      if (selector->MatchesPseudoElement())
      {
        return true;
      }
    }
    return false;
  }

  bool ComplexSelectorMatchesElementBackedPseudoElement(const Selector &complexSelector) noexcept
  {
    bool result = false;
    for (const auto *selector = &complexSelector; selector;
         selector = selector->PrecedingCompoundSelectorComponent())
    {
      if (!selector->MatchesPseudoElement())
      {
        continue;
      }
      switch (selector->PseudoElement())
      {
        case SelectorPseudoElement::Slotted:
        case SelectorPseudoElement::Part:
        case SelectorPseudoElement::UserAgentPart: result = true; break;
        default:                                   return false;
      }
    }
    return result;
  }

#pragma endregion

#pragma region Traversal

  const Selector *Selector::FirstInCompoundSelector() const noexcept
  {
    const auto *selector = this;
    while (!selector->IsFirstInComplexSelector() && selector->Relation() == SelectorRelation::Compounding)
    {
      ++selector;
    }
    return selector;
  }

  const Selector *Selector::LastInCompoundSelector() const noexcept
  {
    const auto *selector = this;
    while (!selector->IsLastInComplexSelector()
           && (selector - 1)->Relation() == SelectorRelation::Compounding)
    {
      --selector;
    }
    return selector;
  }

  const Selector *Selector::PrecedingCompoundSelectorComponent() const noexcept
  {
    return Relation() == SelectorRelation::Compounding ? PrecedingComplexSelectorComponent() : nullptr;
  }

  bool Selector::VisitSimpleSelectors(const VisitFunc &func,
                                      VisitFunctionalPseudoClasses visitFunctionalPseudoClasses,
                                      VisitOnlySubject visitOnlySubject) const noexcept
  {
    Queue<const Selector *> worklist;
    worklist.push(this);
    while (!worklist.empty())
    {
      const auto *current = worklist.front();
      worklist.pop();
      if (func(*current))
      {
        return true;
      }
      if (visitFunctionalPseudoClasses == VisitFunctionalPseudoClasses::Yes)
      {
        if (const auto *selectorList = current->SubSelectors())
        {
          for (const auto &selector : *selectorList)
          {
            worklist.push(&selector);
          }
        }
      }
      // The subject is the rightmost compound selector, which a combinator ends.
      if (const auto *next = current->PrecedingComplexSelectorComponent();
          next
          && (current->Relation() == SelectorRelation::Compounding
              || visitOnlySubject == VisitOnlySubject::No))
      {
        worklist.push(next);
      }
    }
    return false;
  }

  bool Selector::HasExplicitNestingParent() const noexcept
  {
    return VisitSimpleSelectors(
      [](const Selector &selector)
      {
        return selector.Match() == SelectorMatch::NestingParent
               || selector.Match() == SelectorMatch::ForgivingUnknownNestContaining;
      },
      VisitFunctionalPseudoClasses::Yes);
  }

  bool Selector::HasExplicitPseudoClassScope() const noexcept
  {
    return VisitSimpleSelectors([](const Selector &selector) { return selector.IsScopePseudoClass(); },
                                VisitFunctionalPseudoClasses::Yes);
  }

  bool Selector::HasScope() const noexcept
  {
    return VisitSimpleSelectors([](const Selector &selector) { return selector.IsScopePseudoClass(); });
  }

  bool Selector::IsHostPseudoClass() const noexcept
  {
    return Match() == SelectorMatch::PseudoClass && PseudoClass() == SelectorPseudoClass::Host;
  }

  bool Selector::IsScopePseudoClass() const noexcept
  {
    return Match() == SelectorMatch::PseudoClass && PseudoClass() == SelectorPseudoClass::Scope;
  }

#pragma endregion

#pragma region Serialisation

  namespace
  {
    void AppendIdentifier(const CSSOMStringAtom &identifier, StringBuilder &builder) noexcept
    {
      SerialiseIdentifier(identifier.ToString(), builder);
    }

    void AppendIdentifierOrStar(const CSSOMStringAtom &identifier, StringBuilder &builder) noexcept
    {
      if (identifier == dom::StarAtom())
      {
        builder.Append(u8'*');
      }
      else
      {
        AppendIdentifier(identifier, builder);
      }
    }

    void AppendPossiblyQuotedIdentifier(const PossiblyQuotedIdentifier &identifier,
                                        StringBuilder &builder) noexcept
    {
      if (identifier.WasQuoted)
      {
        SerialiseString(identifier.Identifier.ToString(), builder);
      }
      else
      {
        AppendIdentifier(identifier.Identifier, builder);
      }
    }

    /// @brief Appends @p list's items with @p append, separated by @p separator.
    template <typename TList, typename TAppend>
    void AppendInterleaved(StringBuilder &builder, const TList &list, CSSOMStringView separator,
                           TAppend append) noexcept
    {
      bool first = true;
      for (const auto &item : list)
      {
        if (!first)
        {
          builder.Append(separator);
        }
        first = false;
        append(item, builder);
      }
    }

    /// @brief Appends @p selectorList's text in parentheses.
    void AppendSelectorListArgument(const SelectorList &selectorList, StringBuilder &builder) noexcept
    {
      builder.Append(u8'(');
      selectorList.BuildSelectorsText(builder);
      builder.Append(u8')');
    }

    // https://drafts.csswg.org/css-syntax/#serializing-anb
    void AppendAnPlusB(const Selector &selector, StringBuilder &builder) noexcept
    {
      if (selector.Argument() == null)
      {
        return;
      }
      const int32 a = selector.NthA();
      const int32 b = selector.NthB();
      if (a == 0)
      {
        builder.Append(b);
        return;
      }
      if (a == -1)
      {
        builder.Append(u8'-');
      }
      else if (a != 1)
      {
        builder.Append(a);
      }
      builder.Append(u8'n');
      if (b > 0)
      {
        builder.Append(u8'+');
      }
      if (b != 0)
      {
        builder.Append(b);
      }
    }

    void AppendPseudoClass(const Selector &selector, StringBuilder &builder) noexcept
    {
      builder.Append(Selector::SelectorTextForPseudoClass(selector.PseudoClass()));
      switch (selector.PseudoClass())
      {
        case SelectorPseudoClass::Host:
          if (const auto *selectorList = selector.SubSelectors())
          {
            AppendSelectorListArgument(*selectorList, builder);
          }
          break;
        case SelectorPseudoClass::Heading:
          if (const auto *levels = selector.IntegerList(); levels && !levels->empty())
          {
            builder.Append(u8'(');
            AppendInterleaved(builder, *levels, u8", ",
                              [](int32 level, StringBuilder &b) { b.Append(level); });
            builder.Append(u8')');
          }
          break;
        case SelectorPseudoClass::Lang:
          krys_debug_assert_msg(selector.LangList() && !selector.LangList()->empty(),
                                "The parser never makes an empty :lang().");
          builder.Append(u8'(');
          AppendInterleaved(builder, *selector.LangList(), u8", ", AppendPossiblyQuotedIdentifier);
          builder.Append(u8')');
          break;
        case SelectorPseudoClass::NthChild:
        case SelectorPseudoClass::NthLastChild:
          builder.Append(u8'(');
          AppendAnPlusB(selector, builder);
          if (const auto *selectorList = selector.SubSelectors())
          {
            builder.Append(u8" of ");
            selectorList->BuildSelectorsText(builder);
          }
          builder.Append(u8')');
          break;
        case SelectorPseudoClass::Dir:
        case SelectorPseudoClass::NthOfType:
        case SelectorPseudoClass::NthLastOfType:
          builder.Append(u8'(');
          builder.Append(selector.Argument().View());
          builder.Append(u8')');
          break;
        case SelectorPseudoClass::Has:
        case SelectorPseudoClass::Is:
        case SelectorPseudoClass::Not:
        case SelectorPseudoClass::Where: AppendSelectorListArgument(*selector.SubSelectors(), builder); break;
        case SelectorPseudoClass::State:
          builder.Append(u8'(');
          AppendIdentifier(selector.Argument(), builder);
          builder.Append(u8')');
          break;
        default:
          krys_debug_assert_msg(!Selector::PseudoClassMayHaveArgument(selector.PseudoClass()),
                                "Missing serialisation for pseudo-class argument");
          break;
      }
    }

    void AppendPseudoElement(const Selector &selector, StringBuilder &builder) noexcept
    {
      switch (selector.PseudoElement())
      {
        case SelectorPseudoElement::Slotted:
          builder.Append(u8"::slotted");
          AppendSelectorListArgument(*selector.SubSelectors(), builder);
          break;
        case SelectorPseudoElement::Highlight:
          krys_debug_assert(selector.ArgumentList() && !selector.ArgumentList()->empty());
          builder.Append(u8"::");
          builder.Append(selector.SerialisingValue().View());
          builder.Append(u8'(');
          AppendInterleaved(builder, *selector.ArgumentList(), u8".", AppendIdentifierOrStar);
          builder.Append(u8')');
          break;
        case SelectorPseudoElement::Part:
          krys_debug_assert(selector.ArgumentList() && !selector.ArgumentList()->empty());
          builder.Append(u8"::part(");
          AppendInterleaved(builder, *selector.ArgumentList(), u8" ", AppendIdentifier);
          builder.Append(u8')');
          break;
        case SelectorPseudoElement::Cue:
          builder.Append(u8"::cue");
          if (const auto *selectorList = selector.SubSelectors())
          {
            AppendSelectorListArgument(*selectorList, builder);
          }
          break;
        default:
          krys_debug_assert_msg(!Selector::PseudoElementMayHaveArgument(selector.PseudoElement()),
                                "Missing serialisation for pseudo-element argument");
          if (selector.PseudoElement() == SelectorPseudoElement::UserAgentPart
              && selector.Value() == user_agent_parts::InternalSelectPicker())
          {
            builder.Append(u8"::picker(select)");
            break;
          }
          builder.Append(u8"::");
          AppendIdentifier(selector.SerialisingValue(), builder);
          break;
      }
    }

    void AppendAttributeSelector(const Selector &selector, StringBuilder &builder) noexcept
    {
      builder.Append(u8'[');
      if (const auto &prefix = selector.Attribute().NamespacePrefix();
          prefix != null && prefix != CSSOMStringAtom::Empty())
      {
        AppendIdentifierOrStar(prefix, builder);
        builder.Append(u8'|');
      }
      AppendIdentifierOrStar(selector.Attribute().LocalName(), builder);
      switch (selector.Match())
      {
        case SelectorMatch::AttributeExists:    builder.Append(u8']'); return;
        case SelectorMatch::AttributeEquals:    builder.Append(u8'='); break;
        case SelectorMatch::AttributeIncludes:  builder.Append(u8"~="); break;
        case SelectorMatch::AttributeDash:      builder.Append(u8"|="); break;
        case SelectorMatch::AttributePrefix:    builder.Append(u8"^="); break;
        case SelectorMatch::AttributeSuffix:    builder.Append(u8"$="); break;
        case SelectorMatch::AttributeSubstring: builder.Append(u8"*="); break;
        default:                                break;
      }
      SerialiseString(selector.SerialisingValue().ToString(), builder);
      if (selector.IsAttributeValueMatchingCaseInsensitive())
      {
        builder.Append(u8" i");
      }
      else if (selector.IsAttributeValueMatchingCaseSensitive())
      {
        builder.Append(u8" s");
      }
      builder.Append(u8']');
    }

    /// @brief The text of @p relation between two compound selectors.
    KRYS_NODISCARD CSSOMStringView CombinatorText(SelectorRelation relation) noexcept
    {
      switch (relation)
      {
        case SelectorRelation::Descendant:           return u8" ";
        case SelectorRelation::Child:                return u8" > ";
        case SelectorRelation::NextSibling:          return u8" + ";
        case SelectorRelation::SubsequentSibling:    return u8" ~ ";
        case SelectorRelation::Compounding:          krys_debug_assert(false); return {};
        // These implicit combinators aren't written.
        case SelectorRelation::ShadowDescendant:
        case SelectorRelation::ShadowPartDescendant:
        case SelectorRelation::ShadowSlotted:        return {};
      }
      return {};
    }
  }

  // https://drafts.csswg.org/cssom/#serialize-a-selector
  CSSOMString Selector::SelectorText(CSSOMStringView separator, CSSOMStringView rightSide) const noexcept
  {
    StringBuilder builder;

    // A compound's type selector is its rightmost simple selector, so it's this one if it has one.
    if (Match() == SelectorMatch::Type && !_flags.contains(SelectorFlag::TagIsForNamespaceRule))
    {
      if (const auto &prefix = TagQualifiedName().NamespacePrefix(); prefix != null)
      {
        AppendIdentifierOrStar(prefix, builder);
        builder.Append(u8'|');
      }
      AppendIdentifierOrStar(TagQualifiedName().LocalName(), builder);
    }

    // Then the compound's other simple selectors.
    const auto *selector = this;
    while (true)
    {
      if (selector->IsImplicit())
      {
        // An implicit selector isn't written, so neither is the space between it and the combinator.
        separator = separator.substr(1);
        break;
      }
      switch (selector->Match())
      {
        case SelectorMatch::Id:
          builder.Append(u8'#');
          AppendIdentifier(selector->SerialisingValue(), builder);
          break;
        case SelectorMatch::Class:
          builder.Append(u8'.');
          AppendIdentifier(selector->SerialisingValue(), builder);
          break;
        case SelectorMatch::NestingParent:                  builder.Append(u8'&'); break;
        case SelectorMatch::ForgivingUnknown:
        case SelectorMatch::ForgivingUnknownNestContaining: builder.Append(selector->Value().View()); break;
        case SelectorMatch::HasScope:
          // The anchor of a relative selector isn't written either, giving `> a` in `:has(> a)`.
          return CSSOMString(separator.substr(1)) + CSSOMString(rightSide);
        case SelectorMatch::PseudoClass:        AppendPseudoClass(*selector, builder); break;
        case SelectorMatch::PseudoElement:      AppendPseudoElement(*selector, builder); break;
        case SelectorMatch::AttributeExists:
        case SelectorMatch::AttributeEquals:
        case SelectorMatch::AttributeDash:
        case SelectorMatch::AttributeIncludes:
        case SelectorMatch::AttributePrefix:
        case SelectorMatch::AttributeSuffix:
        case SelectorMatch::AttributeSubstring: AppendAttributeSelector(*selector, builder); break;
        case SelectorMatch::Type:
        case SelectorMatch::None:               break;
      }

      if (selector->Relation() != SelectorRelation::Compounding
          || !selector->PrecedingComplexSelectorComponent())
      {
        break;
      }
      selector = selector->PrecedingComplexSelectorComponent();
    }

    builder.Append(separator);
    builder.Append(rightSide);

    // Recurse leftward, with this compound and what follows it as the right side.
    if (const auto *preceding = selector->PrecedingComplexSelectorComponent())
    {
      return preceding->SelectorText(CombinatorText(selector->Relation()), builder);
    }

    // A relative selector with an implicit `&` (in a nested rule) keeps its leading combinator.
    switch (selector->Relation())
    {
      case SelectorRelation::Child:             return CSSOMString(u8"> ") + CSSOMString(builder.View());
      case SelectorRelation::NextSibling:       return CSSOMString(u8"+ ") + CSSOMString(builder.View());
      case SelectorRelation::SubsequentSibling: return CSSOMString(u8"~ ") + CSSOMString(builder.View());
      default:                                  return krys::move(builder).ToString();
    }
  }

#pragma endregion

#pragma region Setters and accessors

  void Selector::SetAttribute(const QualifiedName &name, AttributeCaseFlag caseFlag) noexcept
  {
    CreateRareData();
    _data.Rare->Attribute = name;
    SetFlag(SelectorFlag::CaseInsensitiveAttributeValueMatching, caseFlag == AttributeCaseFlag::Insensitive);
    SetFlag(SelectorFlag::CaseSensitiveAttributeValueMatching, caseFlag == AttributeCaseFlag::Sensitive);
  }

  void Selector::SetArgument(const CSSOMStringAtom &value) noexcept
  {
    CreateRareData();
    _data.Rare->Argument = value;
  }

  void Selector::SetArgumentList(FixedList<CSSOMStringAtom> argumentList) noexcept
  {
    CreateRareData();
    _data.Rare->ArgumentList = krys::move(argumentList);
  }

  void Selector::SetIntegerList(FixedList<int32> integerList) noexcept
  {
    CreateRareData();
    _data.Rare->IntegerList = krys::move(integerList);
  }

  void Selector::SetLangList(FixedList<PossiblyQuotedIdentifier> langList) noexcept
  {
    CreateRareData();
    _data.Rare->LangList = krys::move(langList);
  }

  void Selector::SetSubSelectors(UniquePtr<SelectorList> subSelectors) noexcept
  {
    CreateRareData();
    _data.Rare->SubSelectors = krys::move(subSelectors);
  }

  void Selector::SetNth(int32 a, int32 b) noexcept
  {
    CreateRareData();
    _data.Rare->NthA = a;
    _data.Rare->NthB = b;
  }

  bool Selector::MatchNth(int32 count) const noexcept
  {
    krys_debug_assert(HasRareData());
    return _data.Rare->MatchNth(count);
  }

  int32 Selector::NthA() const noexcept
  {
    krys_debug_assert(HasRareData());
    return _data.Rare->NthA;
  }

  int32 Selector::NthB() const noexcept
  {
    krys_debug_assert(HasRareData());
    return _data.Rare->NthB;
  }

#pragma endregion

#pragma region RareData

  Selector::RareData::RareData(const CSSOMStringAtom &value) noexcept
      : MatchingValue(value), SerialisingValue(value), Attribute(dom::AnyQualifiedName())
  {
  }

  // NOLINTNEXTLINE(bugprone-copy-constructor-init): a copy starts with its own reference count.
  Selector::RareData::RareData(const RareData &other) noexcept
      : MatchingValue(other.MatchingValue), SerialisingValue(other.SerialisingValue), NthA(other.NthA),
        NthB(other.NthB), Attribute(other.Attribute), Argument(other.Argument),
        ArgumentList(other.ArgumentList), LangList(other.LangList), IntegerList(other.IntegerList),
        SubSelectors(other.SubSelectors ? CreateUnique<SelectorList>(*other.SubSelectors)
                                        : UniquePtr<SelectorList>())
  {
  }

  Selector::RareData::~RareData() noexcept = default;

  // https://drafts.csswg.org/selectors/#anb-microsyntax
  bool Selector::RareData::MatchNth(int32 count) const noexcept
  {
    // Whether some n >= 0 gives An+B = count.
    if (NthA > 0)
    {
      return count >= NthB && !((count - NthB) % NthA);
    }
    if (NthA < 0)
    {
      return count <= NthB && !((NthB - count) % -NthA);
    }
    return count == NthB;
  }

  bool Selector::RareData::Equals(const RareData &other) const noexcept
  {
    if (SubSelectors || other.SubSelectors)
    {
      if (!SubSelectors || !other.SubSelectors || *SubSelectors != *other.SubSelectors)
      {
        return false;
      }
    }
    return MatchingValue == other.MatchingValue && SerialisingValue == other.SerialisingValue
           && NthA == other.NthA && NthB == other.NthB && Attribute == other.Attribute
           && Argument == other.Argument && ArgumentList == other.ArgumentList && LangList == other.LangList
           && IntegerList == other.IntegerList;
  }

#pragma endregion

#pragma region Equality and hashing

  bool Selector::SimpleSelectorEqual(const Selector &other) const noexcept
  {
    const auto sameFlag = [&](SelectorFlag flag)
    {
      return _flags.contains(flag) == other._flags.contains(flag);
    };
    if (_match != other._match || _pseudoType != other._pseudoType || !sameFlag(SelectorFlag::HasRareData)
        || !sameFlag(SelectorFlag::TagIsForNamespaceRule)
        || !sameFlag(SelectorFlag::CaseInsensitiveAttributeValueMatching)
        || !sameFlag(SelectorFlag::CaseSensitiveAttributeValueMatching)
        || !sameFlag(SelectorFlag::IsImplicit))
    {
      return false;
    }
    if (HasRareData())
    {
      return _data.Rare->Equals(*other._data.Rare);
    }
    if (Match() == SelectorMatch::Type)
    {
      return *_data.TagQualifiedName == *other._data.TagQualifiedName;
    }
    return _data.Value == other._data.Value;
  }

  namespace
  {
    KRYS_NODISCARD bool ShouldSkipForEqualMode(const Selector &simpleSelector,
                                               ComplexSelectorsEqualMode mode) noexcept
    {
      return mode == ComplexSelectorsEqualMode::IgnoreNonElementBackedPseudoElements
             && simpleSelector.MatchesPseudoElement()
             && !IsElementBackedPseudoElement(simpleSelector.PseudoElement());
    }

    /// @brief Combines @p seed with a hash of what @p simpleSelector tests, which may leave out some of its
    /// data: selectors that compare equal hash equally, but not necessarily the reverse.
    KRYS_NODISCARD size_t HashSimpleSelector(size_t seed, const Selector &simpleSelector) noexcept
    {
      seed = CombineFastHashes(seed, simpleSelector.Match());
      switch (simpleSelector.Match())
      {
        case SelectorMatch::Type:
          seed = CombineFastHashes(seed, simpleSelector.TagQualifiedName().get());
          break;
        case SelectorMatch::PseudoClass: seed = CombineFastHashes(seed, simpleSelector.PseudoClass()); break;
        case SelectorMatch::PseudoElement:
          seed = CombineFastHashes(seed, simpleSelector.PseudoElement());
          break;
        case SelectorMatch::AttributeExists:
        case SelectorMatch::AttributeEquals:
        case SelectorMatch::AttributeDash:
        case SelectorMatch::AttributeIncludes:
        case SelectorMatch::AttributePrefix:
        case SelectorMatch::AttributeSuffix:
        case SelectorMatch::AttributeSubstring:
          seed =
            CombineFastHashes(seed, simpleSelector.Attribute().get(), simpleSelector.Value().ExistingHash());
          break;
        default: seed = CombineFastHashes(seed, simpleSelector.Value().ExistingHash()); break;
      }
      if (const auto *selectorList = simpleSelector.SubSelectors())
      {
        seed = CombineFastHashes(seed, FastHash<SelectorList> {}(*selectorList));
      }
      return seed;
    }
  }

  bool ComplexSelectorsEqual(const Selector &complexA, const Selector &complexB,
                             ComplexSelectorsEqualMode mode) noexcept
  {
    // Each side's relation to the simple selector compared before, on its right.
    auto aRelation = SelectorRelation::Compounding;
    auto bRelation = SelectorRelation::Compounding;
    for (const auto *a = &complexA, *b = &complexB; a || b;
         a = a->PrecedingComplexSelectorComponent(), b = b->PrecedingComplexSelectorComponent())
    {
      if (a && ShouldSkipForEqualMode(*a, mode))
      {
        aRelation = a->Relation();
        a = a->PrecedingComplexSelectorComponent();
      }
      if (b && ShouldSkipForEqualMode(*b, mode))
      {
        bRelation = b->Relation();
        b = b->PrecedingComplexSelectorComponent();
      }
      if (!a || !b)
      {
        return a == b;
      }
      if (aRelation != bRelation || !a->SimpleSelectorEqual(*b))
      {
        return false;
      }
      aRelation = a->Relation();
      bRelation = b->Relation();
    }
    return true;
  }

  size_t HashComplexSelector(const Selector &complexSelector, ComplexSelectorsEqualMode mode) noexcept
  {
    size_t seed = 0;
    auto relationToRight = SelectorRelation::Compounding;
    for (const auto *simpleSelector = &complexSelector; simpleSelector;
         simpleSelector = simpleSelector->PrecedingComplexSelectorComponent())
    {
      if (!ShouldSkipForEqualMode(*simpleSelector, mode))
      {
        seed = CombineFastHashes(seed, relationToRight);
        seed = HashSimpleSelector(seed, *simpleSelector);
      }
      relationToRight = simpleSelector->Relation();
    }
    return seed;
  }

#pragma endregion
}
