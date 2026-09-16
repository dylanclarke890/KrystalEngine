#include "Krystal.Booey/CSS/Selectors/Selector.hpp"
#include "Krystal.Booey/CSS/Parser/SerializerIdioms.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorList.hpp"

namespace krys::boo::css
{
  Selector::Selector(const dom::QualifiedName &name, bool tagIsForNamespaceRule) noexcept
      : _relation(SelectorRelation::Descendant), _match(SelectorMatch::Type)
  {
    _data.TagQualifiedName = name.get();
    _data.TagQualifiedName->AddRef();

    if (tagIsForNamespaceRule)
    {
      _flags = _flags | SelectorFlag::TagIsForNamespaceRule;
    }
  }

  Selector::~Selector() noexcept
  {
    if (HasFlag(_flags, SelectorFlag::HasRareData))
    {
      _data.RareData->SubRef();
      _data.RareData = nullptr;
      _flags = _flags & ~SelectorFlag::HasRareData;
    }
    else if (Match() == SelectorMatch::Type)
    {
      _data.TagQualifiedName->SubRef();
      _data.TagQualifiedName = nullptr;
      _match = SelectorMatch::None;
    }
    else if (_data.Value)
    {
      static_assert(!SupportsRefPtr<StringAtomStorage>,
                    "Must manually manage AddRef/SubRef for string atom here.");
    }
  }

  const CSSOMStringAtom &Selector::Value() const noexcept
  {
    assert(Match() != SelectorMatch::Type);
    if (HasFlag(_flags, SelectorFlag::HasRareData))
    {
      return _data.RareData->MatchingValue;
    }

    return *reinterpret_cast<const CSSOMStringAtom *>(&_data.Value);
  }

  const CSSOMStringAtom &Selector::SerializingValue() const noexcept
  {
    assert(Match() != SelectorMatch::Type);
    if (HasFlag(_flags, SelectorFlag::HasRareData))
    {
      return _data.RareData->SerializingValue;
    }

    return *reinterpret_cast<const CSSOMStringAtom *>(&_data.Value);
  }

  const dom::QualifiedName &Selector::TagQualifiedName() const noexcept
  {
    assert(Match() == SelectorMatch::Type);
    return *reinterpret_cast<const dom::QualifiedName *>(&_data.TagQualifiedName);
  }

  const CSSOMStringAtom &Selector::TagLocalNameLower() const noexcept
  {
    assert(Match() == SelectorMatch::Type);
    return TagQualifiedName().LocalNameLower();
  }

  int64 Selector::NthA() const noexcept
  {
    assert(HasFlag(_flags, SelectorFlag::HasRareData));
    return _data.RareData->NthA;
  }

  int64 Selector::NthB() const noexcept
  {
    assert(HasFlag(_flags, SelectorFlag::HasRareData));
    return _data.RareData->NthB;
  }

  bool Selector::MatchNth(int64 count) const noexcept
  {
    if (!HasFlag(_flags, SelectorFlag::HasRareData))
    {
      return false;
    }

    return _data.RareData->MatchNth(count);
  }

  bool Selector::SimpleSelectorEqual(const Selector &other) const noexcept
  {
    auto ValuesEqual = [&]
    {
      if (HasFlag(_flags, SelectorFlag::HasRareData))
      {
        return _data.RareData->Equals(*other._data.RareData);
      }

      if (Match() == SelectorMatch::Type)
      {
        return *_data.TagQualifiedName == *other._data.TagQualifiedName;
      }

      return _data.Value == other._data.Value;
    };

    auto HasMatchingFlags = [](SelectorFlag flags, SelectorFlag otherFlags, SelectorFlag mask)
    {
      return HasFlag(flags, mask) == HasFlag(otherFlags, mask);
    };

    // Relation and selector list bits are ignored.
    return _match == other._match && _pseudoClass == other._pseudoClass
           && _pseudoElement == other._pseudoElement
           && HasMatchingFlags(_flags, other._flags, SelectorFlag::HasRareData)
           && HasMatchingFlags(_flags, other._flags, SelectorFlag::CaseInsensitiveAttributeValueMatching)
           && HasMatchingFlags(_flags, other._flags, SelectorFlag::TagIsForNamespaceRule)
           && HasMatchingFlags(_flags, other._flags, SelectorFlag::IsImplicit) && ValuesEqual();
  }

  CSSOMString Selector::SelectorText(CSSOMStringView separator, CSSOMStringView rightside) const noexcept
  {
    auto SerializeIdentifierOrStar = [](const CSSOMStringAtom &identifier, utf32_string &output)
    {
      if (identifier == dom::StarAtom())
      {
        output.push_back(U'*');
      }
      else
      {
        SerializerIdioms::SerializeIdentifier(identifier.View(), output);
      }
    };

    utf32_string output;

    if (Match() == SelectorMatch::Type && !HasFlag(_flags, SelectorFlag::TagIsForNamespaceRule))
    {
      if (auto &prefix = TagQualifiedName().NamespacePrefix(); prefix != CSSOMStringAtom::Null())
      {
        SerializeIdentifierOrStar(prefix, output);
        output.push_back(U'|');
      }

      SerializeIdentifierOrStar(TagQualifiedName().LocalName(), output);
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
        output.push_back(U'#');
        SerializerIdioms::SerializeIdentifier(selector->SerializingValue().View(), output);
      }
      else if (selector->Match() == SelectorMatch::NestingParent)
      {
        output.push_back(U'&');
      }
      else if (selector->Match() == SelectorMatch::Class)
      {
        output.push_back(U'.');
        SerializerIdioms::SerializeIdentifier(selector->SerializingValue().View(), output);
      }
      else if (selector->Match() == SelectorMatch::ForgivingUnknown
               || selector->Match() == SelectorMatch::ForgivingUnknownNestContaining)
      {
        auto decodeResult = krys::text::Decode<krys::text::UTF8>(selector->Value().View());
        krys_debug_assert(decodeResult.Error == krys::text::DecodeError::None);
        output.append_range(decodeResult.Output);
      }
      else if (selector->Match() == SelectorMatch::HasScope)
      {
        // Remove the space from the start to generate a relative selector string like in ":has(> foo)".
        return CSSOMString(separator.substr(1)) + CSSOMString(rightside);
      }
      else if (selector->IsAttributeSelector())
      {
        output.push_back(U'[');
        if (auto &prefix = selector->Attribute().NamespacePrefix();
            prefix != CSSOMStringAtom::Null() && prefix != CSSOMStringAtom::Empty())
        {
          SerializeIdentifierOrStar(prefix, output);
          output.push_back(U'|');
        }
        SerializeIdentifierOrStar(selector->Attribute().LocalName(), output);
        switch (selector->Match())
        {
          case SelectorMatch::AttributeEquals:
          {
            output.push_back(U'=');
            break;
          }
          case SelectorMatch::AttributeExists:
          {
            // set has no operator or value, just the attrName
            output.push_back(U']');
            break;
          }
          case SelectorMatch::AttributeIncludes:
          {
            output.append(U"~=");
            break;
          }
          case SelectorMatch::AttributeDash:
          {
            output.append(U"|=");
            break;
          }
          case SelectorMatch::AttributePrefix:
          {
            output.append(U"^=");
            break;
          }
          case SelectorMatch::AttributeSuffix:
          {
            output.append(U"$=");
            break;
          }
          case SelectorMatch::AttributeSubstring:
          {
            output.append(U"*=");
            break;
          }
          default:
          {
            break;
          }
        }

        if (selector->Match() != SelectorMatch::AttributeExists)
        {
          SerializerIdioms::SerializeString(selector->SerializingValue().View(), output);
          if (selector->IsAttributeValueMatchingCaseInsensitive())
          {
            output.append(U" i]");
          }
          else
          {
            output.push_back(U']');
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

    output.append(separator.begin(), separator.end());
    output.append(rightside.begin(), rightside.end());

    if (auto *previousSelector = selector->PrecedingComplexSelectorComponent())
    {
      CSSOMString separator = [relation = selector->Relation()]() noexcept -> CSSOMString
      {
        switch (relation)
        {
          case SelectorRelation::Descendant:
          {
            return u8" ";
          }
          case SelectorRelation::Child:
          {
            return u8" > ";
          }
          case SelectorRelation::NextSibling:
          {
            return u8" + ";
          }
          case SelectorRelation::SubsequentSibling:
          {
            return u8" ~ ";
          }
          case SelectorRelation::Compounding:
          default:
          {
            assert(false);
            return u8"";
          }
        }
      }();

      auto encodeResult = krys::text::Encode<krys::text::UTF8>(output);
      krys_debug_assert(encodeResult.Error == krys::text::EncodeError::None);
      return previousSelector->SelectorText(separator, encodeResult.Output);
    }

    auto encodeResult = krys::text::Encode<krys::text::UTF8>(output);
    krys_debug_assert(encodeResult.Error == krys::text::EncodeError::None);
    return encodeResult.Output;
  }

  void Selector::SetValue(const CSSOMStringAtom &value, MatchLowercase matchLowercase) noexcept
  {
    assert(Match() != SelectorMatch::Type);

    auto matchingValue = value;
    if (matchLowercase)
    {
      matchingValue = krys::text::ToASCIILower(value.View());
    }

    if (!HasFlag(_flags, SelectorFlag::HasRareData) && matchingValue != value)
    {
      CreateRareData();
    }

    if (!HasFlag(_flags, SelectorFlag::HasRareData))
    {
      _data.Value = *reinterpret_cast<const StringAtomStorage *>(&matchingValue);
      // We're going to change StringAtomStorage to be ref-counted eventually so this is a reminder to make
      // sure we manage the ref-counting correctly here when that happens.
      static_assert(!SupportsRefPtr<StringAtomStorage>,
                    "Must manually manage AddRef/SubRef for string atom here.");
      return;
    }

    _data.RareData->MatchingValue = krys::move(matchingValue);
    _data.RareData->SerializingValue = value;
  }

  void Selector::SetAttribute(const dom::QualifiedName &name, IsCaseSensitive caseSensitive) noexcept
  {
    CreateRareData();

    _data.RareData->Attribute = name;

    if (!caseSensitive)
    {
      _flags = _flags | SelectorFlag::CaseInsensitiveAttributeValueMatching;
    }
  }

  void Selector::SetArgument(const CSSOMStringAtom &value) noexcept
  {
    CreateRareData();
    _data.RareData->Argument = value;
  }

  void Selector::SetArgumentList(SmallList<CSSOMStringAtom> argumentList) noexcept
  {
    CreateRareData();
    _data.RareData->ArgumentList = krys::move(argumentList);
  }

  void Selector::SetLangList(SmallList<PossiblyQuotedIdentifier> langList) noexcept
  {
    CreateRareData();
    _data.RareData->LangList = krys::move(langList);
  }

  void Selector::SetSubSelectors(UniquePtr<SelectorList> subSelectors) noexcept
  {
    CreateRareData();
    _data.RareData->SubSelectors = krys::move(subSelectors);
  }

  void Selector::SetNth(int64 a, int64 b) noexcept
  {
    CreateRareData();

    _data.RareData->NthA = a;
    _data.RareData->NthB = b;
  }

  static bool ShouldSkipForEqualMode(const Selector &simpleSelector,
                                     ComplexSelectorsEqualMode mode) noexcept
  {
    if (mode == ComplexSelectorsEqualMode::IgnoreNonElementBackedPseudoElements)
    {
      return simpleSelector.Match() == SelectorMatch::PseudoElement
             && !IsElementBackedPseudoElement(simpleSelector.PseudoElement());
    }

    return false;
  };

  bool ComplexSelectorsEqual(const Selector &complexA, const Selector &complexB,
                             ComplexSelectorsEqualMode mode) noexcept
  {
    auto aRelation = SelectorRelation::Compounding;
    auto bRelation = SelectorRelation::Compounding;

    for (auto a = &complexA, b = &complexB; a != nullptr || b != nullptr;
         a = a->PrecedingComplexSelectorComponent(), b = b->PrecedingComplexSelectorComponent())
    {
      if (a != nullptr && ShouldSkipForEqualMode(*a, mode))
      {
        aRelation = a->Relation();
        a = a->PrecedingComplexSelectorComponent();
      }

      if (b != nullptr && ShouldSkipForEqualMode(*b, mode))
      {
        bRelation = b->Relation();
        b = b->PrecedingComplexSelectorComponent();
      }

      if (a == nullptr || b == nullptr)
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

  void Selector::CreateRareData() noexcept
  {
    assert(Match() != SelectorMatch::Type);
    if (HasFlag(_flags, SelectorFlag::HasRareData))
    {
      return;
    }

    // Move the value to the rare data stucture.
    _data.RareData = CreateRef<RareData>(_data.Value).release();
    _flags = _flags | SelectorFlag::HasRareData;
  }

  bool Selector::VisitSimpleSelectors(VisitFunctionalPseudoClasses visitFunctionalPseudoClasses,
                                         VisitOnlySubject visitOnlySubject, VisitFunc &&func) const noexcept
  {
    Queue<const Selector *> worklist;
    worklist.push(this);

    while (!worklist.empty())
    {
      auto current = worklist.front();
      worklist.pop();

      // Effective C++ advices for this cast to deal with generic const/non-const member function.
      if (func(*const_cast<Selector *>(current)))
      {
        return true;
      }

      // Visit the subselectors (if any) recursively (such as: :has(<list>), :is(<list>),...)
      if (visitFunctionalPseudoClasses)
      {
        if (auto subSelectors = current->SubSelectors())
        {
          for (auto &subSelector : *subSelectors)
          {
            worklist.push(&subSelector);
          }
        }
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

#pragma region RareData

  Selector::RareData::RareData(StringAtomStorage value) noexcept
      : MatchingValue(value), SerializingValue(value), Attribute(dom::AnyQualifiedName())
  {
  }

  Selector::RareData::RareData(const RareData &other) noexcept
      : MatchingValue(other.MatchingValue), SerializingValue(other.SerializingValue), NthA(other.NthA),
        NthB(other.NthB), Attribute(other.Attribute), Argument(other.Argument),
        ArgumentList(other.ArgumentList), LangList(other.LangList)
  {
    if (other.SubSelectors)
    {
      SubSelectors = CreateUnique<SelectorList>(*other.SubSelectors);
    }
  };

  bool Selector::RareData::Equals(const RareData &other) const noexcept
  {
    if (SubSelectors || other.SubSelectors)
    {
      if (!SubSelectors || !other.SubSelectors || *SubSelectors != *other.SubSelectors)
      {
        return false;
      }
    }
    return MatchingValue == other.MatchingValue && SerializingValue == other.SerializingValue
           && NthA == other.NthA && NthB == other.NthB && Attribute == other.Attribute
           && Argument == other.Argument && ArgumentList == other.ArgumentList && LangList == other.LangList
           && SerializingValue == other.SerializingValue;
  }

  bool Selector::RareData::MatchNth(int64 count) const noexcept
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

  Ref<Selector::RareData> Selector::RareData::DeepCopy() const noexcept
  {
    return CreateRef<RareData>(*this);
  }

#pragma endregion
}