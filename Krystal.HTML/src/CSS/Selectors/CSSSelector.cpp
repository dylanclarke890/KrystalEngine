#include "Krystal.HTML/CSS/Selectors/CSSSelector.hpp"
#include "Krystal.HTML/CSS/Selectors/CSSSelectorList.hpp"

namespace Krys::HTML
{
  CSSSelector::CSSSelector(const QualifiedName &name) noexcept
      : _relation(SelectorRelation::Descendant), _match(SelectorMatch::Type)
  {
    _data.TagQualifiedName = name.get();
    _data.TagQualifiedName->AddRef();
  }

  CSSSelector::~CSSSelector() noexcept
  {
    if (HasFlag(_flags, CSSSelectorFlag::HasRareData))
    {
      _data.RareData->SubRef();
      _data.RareData = nullptr;
      _flags = _flags & ~CSSSelectorFlag::HasRareData;
    }
    else if (Match() == SelectorMatch::Type)
    {
      _data.TagQualifiedName->SubRef();
      _data.TagQualifiedName = nullptr;
      _match = SelectorMatch::None;
    }
    else if (_data.Value)
    {
      static_assert(!IsRefCounted<StringAtomStorage>,
                    "Must manually manage AddRef/SubRef for string atom here.");
    }
  }

  const CSSOMStringAtom &CSSSelector::Value() const noexcept
  {
    assert(Match() != SelectorMatch::Type);
    if (HasFlag(_flags, CSSSelectorFlag::HasRareData))
    {
      return _data.RareData->MatchingValue;
    }

    return *reinterpret_cast<const CSSOMStringAtom *>(&_data.Value);
  }

  const CSSOMStringAtom &CSSSelector::SerializingValue() const noexcept
  {
    assert(Match() != SelectorMatch::Type);
    if (HasFlag(_flags, CSSSelectorFlag::HasRareData))
    {
      return _data.RareData->SerializingValue;
    }

    return *reinterpret_cast<const CSSOMStringAtom *>(&_data.Value);
  }

  const QualifiedName &CSSSelector::TagQualifiedName() const noexcept
  {
    assert(Match() == SelectorMatch::Type);
    return *reinterpret_cast<const QualifiedName *>(&_data.TagQualifiedName);
  }

  const CSSOMStringAtom &CSSSelector::TagLocalNameLower() const noexcept
  {
    assert(Match() == SelectorMatch::Type);
    return TagQualifiedName().LocalNameLower();
  }

  int64 CSSSelector::NthA() const noexcept
  {
    assert(HasFlag(_flags, CSSSelectorFlag::HasRareData));
    return _data.RareData->NthA;
  }

  int64 CSSSelector::NthB() const noexcept
  {
    assert(HasFlag(_flags, CSSSelectorFlag::HasRareData));
    return _data.RareData->NthB;
  }

  bool CSSSelector::MatchNth(int64 count) const noexcept
  {
    if (!HasFlag(_flags, CSSSelectorFlag::HasRareData))
    {
      return false;
    }

    return _data.RareData->MatchNth(count);
  }

  bool CSSSelector::SimpleSelectorEqual(const CSSSelector &other) const noexcept
  {
    auto ValuesEqual = [&]
    {
      if (HasFlag(_flags, CSSSelectorFlag::HasRareData))
      {
        return _data.RareData->Equals(*other._data.RareData);
      }

      if (Match() == SelectorMatch::Type)
      {
        return *_data.TagQualifiedName == *other._data.TagQualifiedName;
      }

      return _data.Value == other._data.Value;
    };

    // Relation and selector list bits are ignored.
    return _match == other._match && _pseudoClass == other._pseudoClass
           && _pseudoElement == other._pseudoElement
           && HasFlag(_flags, CSSSelectorFlag::HasRareData)
                == HasFlag(other._flags, CSSSelectorFlag::HasRareData)
           && HasFlag(_flags, CSSSelectorFlag::CaseInsensitiveAttributeValueMatching)
                == HasFlag(other._flags, CSSSelectorFlag::CaseInsensitiveAttributeValueMatching)
           && ValuesEqual();
  }

  void CSSSelector::SetValue(const CSSOMStringAtom &value, MatchLowercase matchLowercase) noexcept
  {
    assert(Match() != SelectorMatch::Type);

    auto matchingValue = value;
    if (matchLowercase)
    {
      matchingValue = Krys::Text::ToASCIILowercase(value.View());
    }

    if (!HasFlag(_flags, CSSSelectorFlag::HasRareData) && matchingValue != value)
    {
      CreateRareData();
    }

    if (!HasFlag(_flags, CSSSelectorFlag::HasRareData))
    {
      // We're going to change StringAtomStorage to be ref-counted eventually so this is a reminder to make
      // sure we manage the ref-counting correctly here when that happens.
      static_assert(!IsRefCounted<StringAtomStorage>,
                    "Must manually manage AddRef/SubRef for string atom here.");
      return;
    }

    _data.RareData->MatchingValue = Krys::Move(matchingValue);
    _data.RareData->SerializingValue = value;
  }

  void CSSSelector::SetAttribute(const QualifiedName &name, IsCaseSensitive caseSensitive) noexcept
  {
    CreateRareData();

    _data.RareData->Attribute = name;

    if (!caseSensitive)
    {
      _flags = _flags | CSSSelectorFlag::CaseInsensitiveAttributeValueMatching;
    }
  }

  void CSSSelector::SetArgument(const CSSOMStringAtom &value) noexcept
  {
    CreateRareData();
    _data.RareData->Argument = value;
  }

  void CSSSelector::SetArgumentList(SmallList<CSSOMStringAtom> argumentList) noexcept
  {
    CreateRareData();
    _data.RareData->ArgumentList = Krys::Move(argumentList);
  }

  void CSSSelector::SetLangList(SmallList<PossiblyQuotedIdentifier> langList) noexcept
  {
    CreateRareData();
    _data.RareData->LangList = Krys::Move(langList);
  }

  void CSSSelector::SetSubSelectors(UniquePtr<CSSSelectorList> subSelectors) noexcept
  {
    CreateRareData();
    _data.RareData->SubSelectors = Krys::Move(subSelectors);
  }

  void CSSSelector::SetNth(int64 a, int64 b) noexcept
  {
    CreateRareData();

    _data.RareData->NthA = a;
    _data.RareData->NthB = b;
  }

  static bool ShouldSkipForEqualMode(const CSSSelector &simpleSelector,
                                     ComplexSelectorsEqualMode mode) noexcept
  {
    if (mode == ComplexSelectorsEqualMode::IgnoreNonElementBackedPseudoElements)
    {
      return simpleSelector.Match() == SelectorMatch::PseudoElement
             && !IsElementBackedPseudoElement(simpleSelector.PseudoElement());
    }

    return false;
  };

  bool ComplexSelectorsEqual(const CSSSelector &complexA, const CSSSelector &complexB,
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

  void CSSSelector::CreateRareData() noexcept
  {
    assert(Match() != SelectorMatch::Type);
    if (HasFlag(_flags, CSSSelectorFlag::HasRareData))
    {
      return;
    }

    // Move the value to the rare data stucture.
    _data.RareData = CreateRef<RareData>(_data.Value).release();
    _flags = _flags | CSSSelectorFlag::HasRareData;
  }

  bool CSSSelector::VisitSimpleSelectors(VisitFunctionalPseudoClasses visitFunctionalPseudoClasses,
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

  CSSSelector::RareData::RareData(StringAtomStorage value) noexcept
      : MatchingValue(value), SerializingValue(value)
  {
  }

  CSSSelector::RareData::RareData(const RareData &other) noexcept
      : MatchingValue(other.MatchingValue), SerializingValue(other.SerializingValue), NthA(other.NthA),
        NthB(other.NthB), Attribute(other.Attribute), Argument(other.Argument),
        ArgumentList(other.ArgumentList), LangList(other.LangList)
  {
    if (other.SubSelectors)
    {
      SubSelectors = CreateUnique<CSSSelectorList>(*other.SubSelectors);
    }
  };

  bool CSSSelector::RareData::Equals(const RareData &other) const noexcept
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

  bool CSSSelector::RareData::MatchNth(int64 count) const noexcept
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

  Ref<CSSSelector::RareData> CSSSelector::RareData::DeepCopy() const noexcept
  {
    return CreateRef<RareData>(*this);
  }

#pragma endregion
}