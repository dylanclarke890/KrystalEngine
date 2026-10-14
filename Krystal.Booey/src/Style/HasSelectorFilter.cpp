#include "Krystal.Booey/Style/HasSelectorFilter.hpp"
#include "Krystal.Booey/CSS/Selectors/Selector.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorFilter.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Style/RuleFeature.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::style
{
  // FIXME: Support additional pseudo-classes.
  static constexpr uint32 HoverSalt = 101;

  HasSelectorFilter::HasSelectorFilter(const dom::Element &element, Type type) noexcept : _type(type)
  {
    switch (type)
    {
      case Type::Descendants:
        for (auto *node = dom::TreeQueries::Next(element, &element); node;
             node = dom::TreeQueries::Next(*node, &element))
        {
          if (auto *descendant = DynamicDowncast<dom::Element>(*node))
          {
            Add(*descendant);
          }
        }
        break;
      case Type::Children:
        for (auto *child = dom::TreeQueries::FirstElementChild(element); child;
             child = dom::TreeQueries::NextElementSibling(*child))
        {
          Add(*child);
        }
        break;
    }
  }

  Maybe<HasSelectorFilter::Type> HasSelectorFilter::TypeForMatchElement(MatchElement matchElement) noexcept
  {
    switch (matchElement)
    {
      case MatchElement::HasChild:      return Type::Children;
      case MatchElement::HasDescendant: return Type::Descendants;
      default:                          return {};
    }
  }

  HasSelectorFilter::Key HasSelectorFilter::MakeKey(const css::Selector &hasSelector) noexcept
  {
    css::SelectorFilter::CollectedSelectorHashes hashes;
    bool hasHoverInCompound = false;
    for (auto *simpleSelector = &hasSelector; simpleSelector;
         simpleSelector = simpleSelector->PrecedingComplexSelectorComponent())
    {
      if (simpleSelector->Match() == css::SelectorMatch::PseudoClass
          && simpleSelector->PseudoClass() == css::SelectorPseudoClass::Hover)
      {
        hasHoverInCompound = true;
      }
      css::SelectorFilter::CollectSimpleSelectorHash(hashes, *simpleSelector);
      if (!hashes.Ids.empty())
      {
        break;
      }
      if (simpleSelector->Relation() != css::SelectorRelation::Compounding)
      {
        break;
      }
    }

    auto pickKey = [&](auto &hashVector) -> Key
    {
      if (hashVector.empty())
      {
        return 0;
      }
      if (hasHoverInCompound)
      {
        return hashVector[0] * HoverSalt;
      }
      return hashVector[0];
    };

    if (auto key = pickKey(hashes.Ids))
    {
      return key;
    }
    if (auto key = pickKey(hashes.Classes))
    {
      return key;
    }
    if (auto key = pickKey(hashes.Attributes))
    {
      return key;
    }
    return pickKey(hashes.Tags);
  }

  void HasSelectorFilter::Add(const dom::Element &element) noexcept
  {
    SmallList<uint32, 4> elementHashes;
    css::SelectorFilter::CollectElementIdentifierHashes(element, elementHashes);

    for (auto hash : elementHashes)
    {
      _filter.Add(hash);
    }

    // FIXME: Also add the hashes salted with HoverSalt for a hovered element, once there is input handling.
  }
}
