#include "Krystal.Booey/CSS/Selectors/Selector.hpp"
#include "Krystal.Booey/CSS/Markup.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorInlines.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorList.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorPseudoTypeMap.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/Queue.hpp"
#include <charconv>

namespace krys::boo::css
{
  struct SameSizeAsSelector
  {
    uint32 Flags;
    void *UnionPointer;
  };

  static_assert(SelectorRelation::Compounding == static_cast<SelectorRelation>(0u),
                "Compounding must be 0 for ConsumeCombinator.");
  static_assert(sizeof(Selector) == sizeof(SameSizeAsSelector), "Selector should remain small.");
  static_assert(sizeof(CSSOMStringAtom) == sizeof(StringAtomStorage), "Selector::Value() casts the union.");
  static_assert(sizeof(dom::QualifiedName) == sizeof(dom::QualifiedNameStorage *),
                "Selector::TagQualifiedName() casts the union.");

  Selector::Selector(const dom::QualifiedName &name, bool tagIsForNamespaceRule) noexcept
      : _match(SelectorMatch::Type)
  {
    SetFlag(SelectorFlag::TagIsForNamespaceRule, tagIsForNamespaceRule);
    _data.TagQualifiedName = name.get();
    _data.TagQualifiedName->AddRef();
  }

  void Selector::CreateRareData() noexcept
  {
    krys_debug_assert(Match() != SelectorMatch::Type);
    if (HasRareData())
    {
      return;
    }

    // Move the value to the rare data stucture.
    _data.Rare = CreateRef<RareData>(CSSOMStringAtom(_data.Value)).release();
    SetFlag(SelectorFlag::HasRareData, true);
  }

#pragma region Specificity

  namespace
  {
    struct SelectorSpecificity
    {
      uint32 Specificity {0};

      SelectorSpecificity() noexcept = default;

      SelectorSpecificity(uint32 specificity) noexcept : Specificity(specificity)
      {
      }

      SelectorSpecificity(SelectorSpecificityIncrement specificity) noexcept
          : Specificity(ToUnderlying(specificity))
      {
      }

      SelectorSpecificity &operator+=(SelectorSpecificity other) noexcept
      {
        auto addSaturating = [&](uint32 mask)
        {
          uint32 otherValue = (other.Specificity & mask);
          if (((Specificity & mask) + otherValue) & ~mask)
          {
            Specificity |= mask;
          }
          else
          {
            Specificity += otherValue;
          }
        };
        addSaturating(0xFF0000);
        addSaturating(0xFF00);
        addSaturating(0xFF);
        return *this;
      }

      KRYS_NODISCARD Array<uint8, 3> SpecificityTuple() const noexcept
      {
        auto a = static_cast<uint8>(Specificity >> 16);
        auto b = static_cast<uint8>(Specificity >> 8);
        auto c = static_cast<uint8>(Specificity);
        return {a, b, c};
      }
    };

    KRYS_NODISCARD SelectorSpecificity operator+(SelectorSpecificity a, SelectorSpecificity b) noexcept
    {
      return a += b;
    }

    enum class IgnorePseudoElement : bool
    {
      No,
      Yes
    };

    KRYS_NODISCARD SelectorSpecificity
      SimpleSelectorSpecificity(const Selector &, IgnorePseudoElement = IgnorePseudoElement::No) noexcept;

    KRYS_NODISCARD SelectorSpecificity
      ComplexSelectorSpecificity(const Selector &firstSimpleSelector,
                                 IgnorePseudoElement ignorePseudoElement = IgnorePseudoElement::No) noexcept
    {
      SelectorSpecificity total;
      for (const auto *selector = &firstSimpleSelector; selector;
           selector = selector->PrecedingComplexSelectorComponent())
      {
        total += SimpleSelectorSpecificity(*selector, ignorePseudoElement);
      }
      return total;
    }

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

    SelectorSpecificity SimpleSelectorSpecificity(const Selector &simpleSelector,
                                                  IgnorePseudoElement ignorePseudoElement) noexcept
    {
      krys_debug_assert_msg(!simpleSelector.IsForPage(),
                            "Page selectors are not treated as real selectors that are matched. The value "
                            "computed here only account for real selectors.");

      if (simpleSelector.IsImplicit()) KRYS_UNLIKELY
      {
        return 0;
      }

      switch (simpleSelector.Match())
      {
        case SelectorMatch::Id:
        {
          return SelectorSpecificityIncrement::ClassA;
        }
        case SelectorMatch::NestingParent:
        case SelectorMatch::PagePseudoClass:
        {
          break;
        }
        case SelectorMatch::PseudoClass:
        {
          switch (simpleSelector.PseudoClass())
          {
            case SelectorPseudoClass::Is:
            {
              return MaxSpecificity(simpleSelector.SubSelectors(), IgnorePseudoElement::Yes);
            }
            case SelectorPseudoClass::Not:
            case SelectorPseudoClass::Has:
            {
              return MaxSpecificity(simpleSelector.SubSelectors());
            }
            case SelectorPseudoClass::Where:
            {
              return 0;
            }
            case SelectorPseudoClass::NthChild:
            case SelectorPseudoClass::NthLastChild:
            case SelectorPseudoClass::Host:
            {
              return SelectorSpecificity(SelectorSpecificityIncrement::ClassB)
                     + MaxSpecificity(simpleSelector.SubSelectors());
            }
            default:
            {
              return SelectorSpecificityIncrement::ClassB;
            }
          }
        }
        case SelectorMatch::AttributeEquals:
        case SelectorMatch::Class:
        case SelectorMatch::AttributeExists:
        case SelectorMatch::AttributeIncludes:
        case SelectorMatch::AttributeDash:
        case SelectorMatch::AttributeSubstring:
        case SelectorMatch::AttributePrefix:
        case SelectorMatch::AttributeSuffix:
        {
          return SelectorSpecificityIncrement::ClassB;
        }
        case SelectorMatch::Type:
        {
          if (simpleSelector.TagQualifiedName().LocalName() == dom::StarAtom())
          {
            return 0;
          }
          return SelectorSpecificityIncrement::ClassC;
        }
        case SelectorMatch::PseudoElement:
        {
          if (ignorePseudoElement == IgnorePseudoElement::Yes)
          {
            return 0;
          }
          switch (simpleSelector.PseudoElement())
          {
            // Slotted only competes with other slotted selectors for specificity, so whether we add the
            // ClassC specificity shouldn't be observable.
            case SelectorPseudoElement::Slotted:
            {
              return MaxSpecificity(simpleSelector.SubSelectors());
            }
            case SelectorPseudoElement::ViewTransitionGroup:
            case SelectorPseudoElement::ViewTransitionImagePair:
            case SelectorPseudoElement::ViewTransitionNew:
            case SelectorPseudoElement::ViewTransitionOld:
            {
              krys_debug_assert(simpleSelector.ArgumentList() && simpleSelector.ArgumentList()->size());
              // Standalone universal selector gets 0 specificity.
              if (simpleSelector.ArgumentList()->first() == dom::StarAtom()
                  && simpleSelector.ArgumentList()->size() == 1)
              {
                return 0;
              }
              break;
            }
            default:
            {
              break;
            }
          }
          return SelectorSpecificityIncrement::ClassC;
        }
        case SelectorMatch::HasScope:
        case SelectorMatch::None:
        case SelectorMatch::ForgivingUnknown:
        case SelectorMatch::ForgivingUnknownNestContaining:
        {
          return 0;
        }
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

  uint32 Selector::SpecificityForPage() const noexcept
  {
    krys_debug_assert(IsForPage());

    // See https://drafts.csswg.org/css-page-3/#cascading-and-page-context
    uint32 s = 0;

    for (const auto *component = this; component; component = component->PrecedingComplexSelectorComponent())
    {
      switch (component->Match())
      {
        case SelectorMatch::Type:
        {
          s += component->TagQualifiedName().LocalName() == dom::StarAtom() ? 0 : 4;
          break;
        }
        case SelectorMatch::PagePseudoClass:
        {
          switch (component->PagePseudoClass())
          {
            case SelectorPagePseudoClass::First:
            {
              s += 2;
              break;
            }
            case SelectorPagePseudoClass::Left:
            case SelectorPagePseudoClass::Right:
            {
              s += 1;
              break;
            }
          }
          break;
        }
        default:
        {
          break;
        }
      }
    }
    return s;
  }

#pragma endregion

  Maybe<PseudoElementType> Selector::StylePseudoElementTypeFor(SelectorPseudoElement type) noexcept
  {
    switch (type)
    {
      case SelectorPseudoElement::FirstLine:
      {
        return PseudoElementType::FirstLine;
      }
      case SelectorPseudoElement::FirstLetter:
      {
        return PseudoElementType::FirstLetter;
      }
      case SelectorPseudoElement::GrammarError:
      {
        return PseudoElementType::GrammarError;
      }
      case SelectorPseudoElement::SpellingError:
      {
        return PseudoElementType::SpellingError;
      }
      case SelectorPseudoElement::Selection:
      {
        return PseudoElementType::Selection;
      }
      case SelectorPseudoElement::TargetText:
      {
        return PseudoElementType::TargetText;
      }
      case SelectorPseudoElement::Highlight:
      {
        return PseudoElementType::Highlight;
      }
      case SelectorPseudoElement::Marker:
      {
        return PseudoElementType::Marker;
      }
      case SelectorPseudoElement::Backdrop:
      {
        return PseudoElementType::Backdrop;
      }
      case SelectorPseudoElement::Before:
      {
        return PseudoElementType::Before;
      }
      case SelectorPseudoElement::After:
      {
        return PseudoElementType::After;
      }
      case SelectorPseudoElement::Checkmark:
      {
        return PseudoElementType::Checkmark;
      }
      case SelectorPseudoElement::ViewTransition:
      {
        return PseudoElementType::ViewTransition;
      }
      case SelectorPseudoElement::ViewTransitionGroup:
      {
        return PseudoElementType::ViewTransitionGroup;
      }
      case SelectorPseudoElement::ViewTransitionImagePair:
      {
        return PseudoElementType::ViewTransitionImagePair;
      }
      case SelectorPseudoElement::ViewTransitionOld:
      {
        return PseudoElementType::ViewTransitionOld;
      }
      case SelectorPseudoElement::ViewTransitionNew:
      {
        return PseudoElementType::ViewTransitionNew;
      }
      case SelectorPseudoElement::Cue:
      case SelectorPseudoElement::Slotted:
      case SelectorPseudoElement::Part:
      case SelectorPseudoElement::UserAgentPart:
      {
        return {};
      }
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
    if (!type)
    {
      return {};
    }

    if (!IsPseudoElementEnabled(*type, name, context))
    {
      return {};
    }

    return *type;
  }

  const Selector *Selector::FirstInCompoundSelector() const noexcept
  {
    const auto *selector = this;
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

  const Selector *Selector::LastInCompoundSelector() const noexcept
  {
    const auto *selector = this;
    while (!selector->IsLastInComplexSelector())
    {
      const auto *next = selector - 1;
      if (next->Relation() != SelectorRelation::Compounding)
      {
        break;
      }
      selector = next;
    }
    return selector;
  }

  const Selector *Selector::PrecedingCompoundSelectorComponent() const noexcept
  {
    if (Relation() != SelectorRelation::Compounding)
    {
      return nullptr;
    }
    return PrecedingComplexSelectorComponent();
  }

#pragma region Serialisation

  namespace
  {
    void AppendIdentifier(const CSSOMStringAtom &identifier, StringBuilder &builder) noexcept
    {
      SerialiseIdentifier(CSSOMString(identifier.View()), builder);
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

    void AppendPseudoClassFunctionTail(StringBuilder &builder, const Selector *selector) noexcept
    {
      switch (selector->PseudoClass())
      {
        case SelectorPseudoClass::Dir:
        case SelectorPseudoClass::NthOfType:
        case SelectorPseudoClass::NthLastOfType:
        {
          builder.Append(selector->Argument().View());
          builder.Append(u8')');
          break;
        }
        default:
        {
          krys_debug_assert(false);
          break;
        }
      }
    }

    void AppendPossiblyQuotedIdentifier(StringBuilder &builder,
                                        const PossiblyQuotedIdentifier &identifier) noexcept
    {
      if (!identifier.WasQuoted)
      {
        AppendIdentifier(identifier.Identifier, builder);
      }
      else
      {
        SerialiseString(CSSOMString(identifier.Identifier.View()), builder);
      }
    }

    void AppendCommaSeparatedPossiblyQuotedIdentifierList(
      StringBuilder &builder, const FixedVector<PossiblyQuotedIdentifier> &list) noexcept
    {
      bool first = true;
      for (const auto &identifier : list)
      {
        if (!first)
        {
          builder.Append(u8", ");
        }
        first = false;
        AppendPossiblyQuotedIdentifier(builder, identifier);
      }
    }

    template <typename TAppend>
    void AppendInterleaved(StringBuilder &builder, const FixedVector<CSSOMStringAtom> &list,
                           CSSOMStringView separator, TAppend &&append) noexcept
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

    // https://drafts.csswg.org/css-syntax/#serializing-anb
    void OutputNthChildAnPlusB(const Selector &selector, StringBuilder &builder) noexcept
    {
      auto outputFirstTerm = [&builder](int32 a)
      {
        switch (a)
        {
          case 1:
          {
            break;
          }
          case -1:
          {
            builder.Append(u8'-');
            break;
          }
          default:
          {
            builder.Append(a);
            break;
          }
        }
      };

      if (selector.Argument() == CSSOMStringAtom::Null())
      {
        return;
      }

      int32 a = selector.NthA();
      int32 b = selector.NthB();
      if (a == 0 && b == 0)
      {
        builder.Append(u8'0');
      }
      else if (a == 0)
      {
        builder.Append(b);
      }
      else if (b == 0)
      {
        outputFirstTerm(a);
        builder.Append(u8'n');
      }
      else if (b < 0)
      {
        outputFirstTerm(a);
        builder.Append(u8'n');
        builder.Append(b);
      }
      else
      {
        outputFirstTerm(a);
        builder.Append(u8"n+");
        builder.Append(b);
      }
    }
  }

  CSSOMString Selector::SelectorText(CSSOMStringView separator, CSSOMStringView rightSide) const noexcept
  {
    StringBuilder builder;

    if (Match() == SelectorMatch::Type && !HasFlag(_flags, SelectorFlag::TagIsForNamespaceRule))
    {
      if (const auto &prefix = TagQualifiedName().NamespacePrefix(); prefix != CSSOMStringAtom::Null())
      {
        AppendIdentifierOrStar(prefix, builder);
        builder.Append(u8'|');
      }
      AppendIdentifierOrStar(TagQualifiedName().LocalName(), builder);
    }

    const auto *selector = this;
    while (true)
    {
      if (selector->IsImplicit())
      {
        // Remove the space before the implicit selector.
        separator = separator.substr(1);
        break;
      }
      if (selector->Match() == SelectorMatch::Id)
      {
        builder.Append(u8'#');
        AppendIdentifier(selector->SerialisingValue(), builder);
      }
      else if (selector->Match() == SelectorMatch::NestingParent)
      {
        builder.Append(u8'&');
      }
      else if (selector->Match() == SelectorMatch::Class)
      {
        builder.Append(u8'.');
        AppendIdentifier(selector->SerialisingValue(), builder);
      }
      else if (selector->Match() == SelectorMatch::ForgivingUnknown
               || selector->Match() == SelectorMatch::ForgivingUnknownNestContaining)
      {
        builder.Append(selector->Value().View());
      }
      else if (selector->Match() == SelectorMatch::HasScope)
      {
        // Remove the space from the start to generate a relative selector string like in ":has(> foo)".
        return CSSOMString(separator.substr(1)) + CSSOMString(rightSide);
      }
      else if (selector->Match() == SelectorMatch::PseudoClass)
      {
        builder.Append(SelectorTextForPseudoClass(selector->PseudoClass()));

        // Handle serialisation of functional variants.
        switch (selector->PseudoClass())
        {
          case SelectorPseudoClass::Host:
          {
            if (const auto *selectorList = selector->SubSelectors())
            {
              builder.Append(u8'(');
              selectorList->BuildSelectorsText(builder);
              builder.Append(u8')');
            }
            break;
          }
          case SelectorPseudoClass::Lang:
          {
            builder.Append(u8'(');
            krys_debug_assert_msg(selector->LangList() && !selector->LangList()->isEmpty(),
                                  "An empty :lang() is invalid and should never be generated by the parser.");
            AppendCommaSeparatedPossiblyQuotedIdentifierList(builder, *selector->LangList());
            builder.Append(u8')');
            break;
          }
          case SelectorPseudoClass::NthChild:
          case SelectorPseudoClass::NthLastChild:
          {
            builder.Append(u8'(');
            OutputNthChildAnPlusB(*selector, builder);
            if (const auto *selectorList = selector->SubSelectors())
            {
              builder.Append(u8" of ");
              selectorList->BuildSelectorsText(builder);
            }
            builder.Append(u8')');
            break;
          }
          case SelectorPseudoClass::Dir:
          case SelectorPseudoClass::NthOfType:
          case SelectorPseudoClass::NthLastOfType:
          {
            builder.Append(u8'(');
            AppendPseudoClassFunctionTail(builder, selector);
            break;
          }
          case SelectorPseudoClass::Has:
          case SelectorPseudoClass::Is:
          case SelectorPseudoClass::Not:
          case SelectorPseudoClass::Where:
          {
            builder.Append(u8'(');
            selector->SubSelectors()->BuildSelectorsText(builder);
            builder.Append(u8')');
            break;
          }
          case SelectorPseudoClass::State:
          {
            builder.Append(u8'(');
            AppendIdentifier(selector->Argument(), builder);
            builder.Append(u8')');
            break;
          }
          case SelectorPseudoClass::ActiveViewTransitionType:
          {
            krys_debug_assert_msg(selector->ArgumentList() && !selector->ArgumentList()->isEmpty(),
                                  "An empty :active-view-transition-type() is invalid and should never be "
                                  "generated by the parser.");
            builder.Append(u8'(');
            AppendInterleaved(builder, *selector->ArgumentList(), u8", ", AppendIdentifier);
            builder.Append(u8')');
            break;
          }
          default:
          {
            krys_debug_assert_msg(!PseudoClassMayHaveArgument(selector->PseudoClass()),
                                  "Missing serialisation for pseudo-class argument");
            break;
          }
        }
      }
      else if (selector->Match() == SelectorMatch::PseudoElement)
      {
        switch (selector->PseudoElement())
        {
          case SelectorPseudoElement::Slotted:
          {
            builder.Append(u8"::slotted(");
            selector->SubSelectors()->BuildSelectorsText(builder);
            builder.Append(u8')');
            break;
          }
          case SelectorPseudoElement::Highlight:
          case SelectorPseudoElement::ViewTransitionGroup:
          case SelectorPseudoElement::ViewTransitionImagePair:
          case SelectorPseudoElement::ViewTransitionOld:
          case SelectorPseudoElement::ViewTransitionNew:
          {
            // Name or universal selector always comes first, followed by classes.
            krys_debug_assert(selector->ArgumentList() && !selector->ArgumentList()->isEmpty());

            builder.Append(u8"::");
            builder.Append(selector->SerialisingValue().View());
            builder.Append(u8'(');
            AppendInterleaved(builder, *selector->ArgumentList(), u8".", AppendIdentifierOrStar);
            builder.Append(u8')');
            break;
          }
          case SelectorPseudoElement::Part:
          {
            krys_debug_assert(selector->ArgumentList() && !selector->ArgumentList()->isEmpty());

            builder.Append(u8"::part(");
            AppendInterleaved(builder, *selector->ArgumentList(), u8" ", AppendIdentifier);
            builder.Append(u8')');
            break;
          }
          case SelectorPseudoElement::Cue:
          {
            builder.Append(u8"::cue");
            if (const auto *selectorList = selector->SubSelectors())
            {
              builder.Append(u8'(');
              selectorList->BuildSelectorsText(builder);
              builder.Append(u8')');
            }
            break;
          }
          default:
          {
            krys_debug_assert_msg(!PseudoElementMayHaveArgument(selector->PseudoElement()),
                                  "Missing serialisation for pseudo-element argument");
            builder.Append(u8"::");
            AppendIdentifier(selector->SerialisingValue(), builder);
            break;
          }
        }
      }
      else if (selector->IsAttributeSelector())
      {
        builder.Append(u8'[');
        if (const auto &prefix = selector->Attribute().NamespacePrefix();
            prefix != CSSOMStringAtom::Null() && prefix != CSSOMStringAtom::Empty())
        {
          AppendIdentifierOrStar(prefix, builder);
          builder.Append(u8'|');
        }
        AppendIdentifierOrStar(selector->Attribute().LocalName(), builder);
        switch (selector->Match())
        {
          case SelectorMatch::AttributeEquals:
          {
            builder.Append(u8'=');
            break;
          }
          case SelectorMatch::AttributeExists:
          {
            // AttributeExists has no operator or value, just the attribute name.
            builder.Append(u8']');
            break;
          }
          case SelectorMatch::AttributeIncludes:
          {
            builder.Append(u8"~=");
            break;
          }
          case SelectorMatch::AttributeDash:
          {
            builder.Append(u8"|=");
            break;
          }
          case SelectorMatch::AttributePrefix:
          {
            builder.Append(u8"^=");
            break;
          }
          case SelectorMatch::AttributeSuffix:
          {
            builder.Append(u8"$=");
            break;
          }
          case SelectorMatch::AttributeSubstring:
          {
            builder.Append(u8"*=");
            break;
          }
          default:
          {
            break;
          }
        }
        if (selector->Match() != SelectorMatch::AttributeExists)
        {
          SerialiseString(CSSOMString(selector->SerialisingValue().View()), builder);
          if (selector->IsAttributeValueMatchingCaseInsensitive())
          {
            builder.Append(u8" i]");
          }
          else
          {
            builder.Append(u8']');
          }
        }
      }
      else if (selector->Match() == SelectorMatch::PagePseudoClass)
      {
        switch (selector->PagePseudoClass())
        {
          case SelectorPagePseudoClass::First:
          {
            builder.Append(u8":first");
            break;
          }
          case SelectorPagePseudoClass::Left:
          {
            builder.Append(u8":left");
            break;
          }
          case SelectorPagePseudoClass::Right:
          {
            builder.Append(u8":right");
            break;
          }
        }
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

    auto separatorTextForNestingRelative = [&]() -> Maybe<CSSOMStringView>
    {
      switch (selector->Relation())
      {
        case SelectorRelation::Child:
        {
          return u8"> ";
        }
        case SelectorRelation::NextSibling:
        {
          return u8"+ ";
        }
        case SelectorRelation::SubsequentSibling:
        {
          return u8"~ ";
        }
        default:
        {
          return {};
        }
      }
    };

    if (const auto *previousSelector = selector->PrecedingComplexSelectorComponent())
    {
      CSSOMStringView relationSeparator;
      switch (selector->Relation())
      {
        case SelectorRelation::Descendant:
        {
          relationSeparator = u8" ";
          break;
        }
        case SelectorRelation::Child:
        {
          relationSeparator = u8" > ";
          break;
        }
        case SelectorRelation::NextSibling:
        {
          relationSeparator = u8" + ";
          break;
        }
        case SelectorRelation::SubsequentSibling:
        {
          relationSeparator = u8" ~ ";
          break;
        }
        case SelectorRelation::Compounding:
        {
          krys_debug_assert(false);
          break;
        }
        case SelectorRelation::ShadowDescendant:
        case SelectorRelation::ShadowPartDescendant:
        case SelectorRelation::ShadowSlotted:
        {
          break;
        }
      }
      return previousSelector->SelectorText(relationSeparator, builder);
    }
    else if (auto separatorText = separatorTextForNestingRelative())
    {
      // We have a separator but no preceding selector which can happen with implicit relative nesting
      // selector.
      return CSSOMString(*separatorText) + CSSOMString(builder.View());
    }

    return krys::move(builder).ToString();
  }

#pragma endregion

  void Selector::SetAttribute(const dom::QualifiedName &value, IsCaseSensitive caseSensitive) noexcept
  {
    CreateRareData();
    _data.Rare->Attribute = value;
    SetFlag(SelectorFlag::CaseInsensitiveAttributeValueMatching, !caseSensitive);
  }

  void Selector::SetArgument(const CSSOMStringAtom &value) noexcept
  {
    CreateRareData();
    _data.Rare->Argument = value;
  }

  void Selector::SetArgumentList(FixedVector<CSSOMStringAtom> argumentList) noexcept
  {
    CreateRareData();
    _data.Rare->ArgumentList = krys::move(argumentList);
  }

  void Selector::SetLangList(FixedVector<PossiblyQuotedIdentifier> langList) noexcept
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

#pragma region RareData

  Selector::RareData::RareData(CSSOMStringAtom value) noexcept
      : MatchingValue(value), SerialisingValue(value), Attribute(dom::AnyQualifiedName())
  {
  }

  Selector::RareData::RareData(const RareData &other) noexcept
      : MatchingValue(other.MatchingValue), SerialisingValue(other.SerialisingValue), NthA(other.NthA),
        NthB(other.NthB), Attribute(other.Attribute), Argument(other.Argument),
        ArgumentList(other.ArgumentList), LangList(other.LangList)
  {
    if (other.SubSelectors)
    {
      SubSelectors = CreateUnique<SelectorList>(*other.SubSelectors);
    }
  }

  Selector::RareData::~RareData() noexcept = default;

  Ref<Selector::RareData> Selector::RareData::DeepCopy() const noexcept
  {
    return CreateRef<RareData>(*this);
  }

  bool Selector::RareData::MatchNth(int32 count) const noexcept
  {
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
           && Argument == other.Argument && ArgumentList == other.ArgumentList && LangList == other.LangList;
  }

#pragma endregion

  Selector::Selector(const Selector &other) noexcept
      : _relation(other._relation), _match(other._match), _pseudoType(other._pseudoType), _flags(other._flags)
  {
    // Manually ref count the _data union because the pointers are stored raw, not as Ref.
    if (other.HasRareData())
    {
      _data.Rare = other._data.Rare->DeepCopy().release();
    }
    else if (other.Match() == SelectorMatch::Type)
    {
      _data.TagQualifiedName = other._data.TagQualifiedName;
      _data.TagQualifiedName->AddRef();
    }
    else
    {
      _data.Value = other._data.Value;
    }
  }

  Selector::Selector(const Selector &other, MutableSelectorCopyTag) noexcept : Selector(other)
  {
    // Restore the selector list bits to the initial state when copying to a MutableSelector.
    SetFlag(SelectorFlag::IsLastInSelectorList, false);
    SetFlag(SelectorFlag::IsFirstInComplexSelector, true);
    SetFlag(SelectorFlag::IsLastInComplexSelector, true);
  }

  bool Selector::VisitSimpleSelectors(VisitFunc &&func,
                                      VisitFunctionalPseudoClasses visitFunctionalPseudoClasses,
                                      VisitOnlySubject visitOnlySubject) const noexcept
  {
    Queue<const Selector *> worklist;
    worklist.push(this);
    while (!worklist.empty())
    {
      auto current = worklist.front();
      worklist.pop();

      if (func(*current))
      {
        return true;
      }

      // Visit the selector list member (if any) recursively (such as: :has(<list>), :is(<list>),...)
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

      // Visit the next simple selector
      if (auto next = current->PrecedingComplexSelectorComponent())
      {
        // We stop visiting at the end of the compound selector (= when relation is anything else than
        // compounding) if we are in subject only mode.
        if (current->Relation() == SelectorRelation::Compounding || visitOnlySubject != VisitOnlySubject::Yes)
        {
          worklist.push(next);
        }
      }
    }
    return false;
  }

  bool Selector::HasExplicitNestingParent() const noexcept
  {
    return VisitSimpleSelectors(
      [](const Selector &selector)
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
      },
      VisitFunctionalPseudoClasses::Yes);
  }

  bool Selector::HasExplicitPseudoClassScope() const noexcept
  {
    return VisitSimpleSelectors([](const Selector &selector) { return selector.IsScopePseudoClass(); },
                                VisitFunctionalPseudoClasses::Yes);
  }

  bool Selector::IsHostPseudoClass() const noexcept
  {
    return Match() == SelectorMatch::PseudoClass && PseudoClass() == SelectorPseudoClass::Host;
  }

  bool Selector::IsScopePseudoClass() const noexcept
  {
    return Match() == SelectorMatch::PseudoClass && PseudoClass() == SelectorPseudoClass::Scope;
  }

  bool Selector::HasScope() const noexcept
  {
    return VisitSimpleSelectors([](const Selector &selector) { return selector.IsScopePseudoClass(); });
  }

  bool ComplexSelectorCanMatchPseudoElement(const Selector &complexSelector) noexcept
  {
    const auto *selector = &complexSelector;
    do
    {
      if (selector->MatchesPseudoElement())
      {
        return true;
      }

      // FIXME: This is probably unneeded as functional pseudo-classes can't contain valid pseudo elements.
      if (const auto *selectorList = selector->SubSelectors())
      {
        for (const auto &subSelector : *selectorList)
        {
          if (ComplexSelectorCanMatchPseudoElement(subSelector))
          {
            return true;
          }
        }
      }

      selector = selector->PrecedingComplexSelectorComponent();
    } while (selector);
    return false;
  }

  bool ComplexSelectorMatchesElementBackedPseudoElement(const Selector &complexSelector) noexcept
  {
    auto isElementBacked = [](SelectorPseudoElement pseudoElement)
    {
      switch (pseudoElement)
      {
        case SelectorPseudoElement::Slotted:
        case SelectorPseudoElement::Part:
        case SelectorPseudoElement::UserAgentPart:
        {
          return true;
        }
        default:
        {
          return false;
        }
      }
    };

    auto result = false;
    for (const auto *simpleSelector = &complexSelector; simpleSelector;
         simpleSelector = simpleSelector->PrecedingCompoundSelectorComponent())
    {
      if (simpleSelector->MatchesPseudoElement())
      {
        if (!isElementBacked(simpleSelector->PseudoElement()))
        {
          return false;
        }
        result = true;
      }
    }
    return result;
  }

  bool Selector::SimpleSelectorEqual(const Selector &other) const noexcept
  {
    auto valuesEqual = [&]
    {
      if (HasRareData())
      {
        return _data.Rare->Equals(*other._data.Rare);
      }
      if (Match() == SelectorMatch::Type)
      {
        return *_data.TagQualifiedName == *other._data.TagQualifiedName;
      }
      return _data.Value == other._data.Value;
    };

    auto sameFlag = [&](SelectorFlag flag)
    {
      return HasFlag(_flags, flag) == HasFlag(other._flags, flag);
    };

    // Relation and selector list bits are ignored.
    return _match == other._match && _pseudoType == other._pseudoType && sameFlag(SelectorFlag::HasRareData)
           && sameFlag(SelectorFlag::TagIsForNamespaceRule)
           && sameFlag(SelectorFlag::CaseInsensitiveAttributeValueMatching)
           && sameFlag(SelectorFlag::IsImplicit) && valuesEqual();
  }

  bool IsElementBackedPseudoElement(SelectorPseudoElement pseudoElement) noexcept
  {
    switch (pseudoElement)
    {
      case SelectorPseudoElement::Part:
      case SelectorPseudoElement::Slotted:
      case SelectorPseudoElement::UserAgentPart:
      case SelectorPseudoElement::Cue:
      {
        return true;
      }
      default:
      {
        return false;
      }
    }
  }

  namespace
  {
    KRYS_NODISCARD bool ShouldSkipForEqualMode(const Selector &simpleSelector,
                                               ComplexSelectorsEqualMode mode) noexcept
    {
      if (mode == ComplexSelectorsEqualMode::IgnoreNonElementBackedPseudoElements)
      {
        return simpleSelector.MatchesPseudoElement()
               && !IsElementBackedPseudoElement(simpleSelector.PseudoElement());
      }
      return false;
    }

    KRYS_NODISCARD size_t HashSimpleSelector(size_t seed, const Selector &simpleSelector) noexcept
    {
      // This hash does try to include every possible thing in a selector.
      seed = Hash::Combine(seed, simpleSelector.Match());

      switch (simpleSelector.Match())
      {
        case SelectorMatch::Type:
        {
          seed = Hash::Combine(seed, simpleSelector.TagQualifiedName().get());
          break;
        }
        case SelectorMatch::PseudoClass:
        {
          seed = Hash::Combine(seed, simpleSelector.PseudoClass());
          break;
        }
        case SelectorMatch::PseudoElement:
        {
          seed = Hash::Combine(seed, simpleSelector.PseudoElement());
          break;
        }
        case SelectorMatch::AttributeEquals:
        case SelectorMatch::AttributeExists:
        case SelectorMatch::AttributeIncludes:
        case SelectorMatch::AttributeDash:
        case SelectorMatch::AttributePrefix:
        case SelectorMatch::AttributeSuffix:
        case SelectorMatch::AttributeSubstring:
        {
          seed = Hash::Combine(seed, simpleSelector.Attribute().get(), simpleSelector.Value().Storage());
          break;
        }
        default:
        {
          seed = Hash::Combine(seed, simpleSelector.Value().Storage());
          break;
        }
      }
      if (const auto *selectorList = simpleSelector.SubSelectors())
      {
        seed = Hash::Combine(seed, DefaultHash<SelectorList>::Hash(*selectorList));
      }
      return seed;
    }
  }

  bool ComplexSelectorsEqual(const Selector &complexA, const Selector &complexB,
                             ComplexSelectorsEqualMode mode) noexcept
  {
    auto aRelation = SelectorRelation::Compounding;
    auto bRelation = SelectorRelation::Compounding;

    for (auto a = &complexA, b = &complexB; a || b;
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
      if (aRelation != bRelation)
      {
        return false;
      }
      if (!a->SimpleSelectorEqual(*b))
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
    for (auto simpleSelector = &complexSelector; simpleSelector;
         simpleSelector = simpleSelector->PrecedingComplexSelectorComponent())
    {
      if (ShouldSkipForEqualMode(*simpleSelector, mode))
      {
        relationToRight = simpleSelector->Relation();
        continue;
      }
      seed = Hash::Combine(seed, relationToRight);
      seed = HashSimpleSelector(seed, *simpleSelector);
      relationToRight = simpleSelector->Relation();
    }
    return seed;
  }
}
