#include "Krystal.Web/Css/Selectors/SelectorFilter.hpp"
#include "Krystal.Core/Hashing/FastHash.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Web/Css/Selectors/Selector.hpp"
#include "Krystal.Web/Css/Selectors/SelectorList.hpp"
#include "Krystal.Web/Dom/Nodes/Attr.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Web/Html/Elements/HTMLElement.hpp"
#include <algorithm>
#include <tuple>

namespace krys::web::css
{
  namespace
  {
    // All kinds of identifier share the filter. Multiplying each kind's hashes by its own prime keeps them
    // apart, so an ancestor's `<article>` doesn't satisfy `.article`.
    enum : uint32
    {
      TagNameSalt = 13,
      IdSalt = 17,
      ClassSalt = 19,
      AttributeSalt = 23,
    };

    /// @brief A non-zero hash of @p identifier, as zero ends a `SelectorFilter::Hashes`.
    KRYS_NODISCARD uint32 IdentifierHash(const CSSOMStringAtom &identifier) noexcept
    {
      const size_t hash = FastHash<CSSOMStringAtom> {}(identifier);
      const auto folded = static_cast<uint32>(hash ^ (hash >> 32));
      return folded ? folded : 0x80'00'00'00u;
    }

    /// @brief Whether attribute @p name is left out of the filter. `id` and `class` are hashed as IDs and
    /// classes instead, and `style` isn't hashed at all.
    KRYS_NODISCARD bool IsExcludedAttribute(const CSSOMStringAtom &name) noexcept
    {
      return name.View() == u8"class" || name.View() == u8"id" || name.View() == u8"style";
    }
  }

  void SelectorFilter::CollectElementIdentifierHashes(const web::Element &element,
                                                      SmallList<uint32, 4> &identifierHashes) noexcept
  {
    identifierHashes.push_back(IdentifierHash(element.TagQualifiedName().LocalNameLower()) * TagNameSalt);
    if (auto id = element.IdForStyleResolution(); !id.empty())
    {
      identifierHashes.push_back(IdentifierHash(CSSOMStringAtom(krys::move(id))) * IdSalt);
    }
    for (const auto &className : element.ClassNames())
    {
      identifierHashes.push_back(IdentifierHash(className) * ClassSalt);
    }
    for (const auto &attribute : element.AttributeList())
    {
      // HTML elements' attribute names are already lowercase; selectors' are compared lowercased.
      const auto &name = Is<HTMLElement>(element) ? attribute->QualifiedName().LocalName()
                                                  : attribute->QualifiedName().LocalNameLower();
      if (!IsExcludedAttribute(name))
      {
        identifierHashes.push_back(IdentifierHash(name) * AttributeSalt);
      }
    }
  }

  bool SelectorFilter::ParentStackIsConsistent(const ContainerNode *parentNode) const noexcept
  {
    if (!parentNode || Is<Document>(*parentNode) || Is<ShadowRoot>(*parentNode))
    {
      return _parentStack.empty();
    }
    return !_parentStack.empty() && _parentStack.back().Element == parentNode;
  }

  void SelectorFilter::InitializeParentStack(const web::Element &parent) noexcept
  {
    SmallList<const web::Element *, 20> ancestors;
    for (const auto *ancestor = &parent; ancestor; ancestor = ancestor->ParentElement())
    {
      ancestors.push_back(ancestor);
    }
    _parentStack.reserve(_parentStack.capacity() + ancestors.size());
    for (auto i = ancestors.size(); i--;)
    {
      PushParent(*ancestors[i]);
    }
  }

  void SelectorFilter::PushParent(const web::Element &parent) noexcept
  {
    krys_debug_assert(_parentStack.empty() || _parentStack.back().Element == parent.ParentElement());
    krys_debug_assert(!_parentStack.empty() || !parent.ParentElement());

    auto &frame = _parentStack.emplace_back(ParentStackFrame {.Element = &parent});
    CollectElementIdentifierHashes(parent, frame.IdentifierHashes);
    for (auto hash : frame.IdentifierHashes)
    {
      _ancestorIdentifierFilter.Add(hash);
    }
  }

  void SelectorFilter::PushParentInitializingIfNeeded(const web::Element &parent) noexcept
  {
    if (_parentStack.empty()) KRYS_UNLIKELY
    {
      InitializeParentStack(parent);
      return;
    }
    PushParent(parent);
  }

  void SelectorFilter::PopParent() noexcept
  {
    krys_debug_assert(!_parentStack.empty());
    for (auto hash : _parentStack.back().IdentifierHashes)
    {
      _ancestorIdentifierFilter.Remove(hash);
    }
    _parentStack.pop_back();
    if (_parentStack.empty())
    {
      // Every hash added has been removed; clearing also resets any counter that overflowed.
      krys_debug_assert(_ancestorIdentifierFilter.LikelyEmpty());
      _ancestorIdentifierFilter.Clear();
    }
  }

  void SelectorFilter::PopParentsUntil(const web::Element *parent) noexcept
  {
    while (!_parentStack.empty() && (!parent || _parentStack.back().Element != parent))
    {
      PopParent();
    }
  }

  void SelectorFilter::CollectSimpleSelectorHash(CollectedSelectorHashes &collectedHashes,
                                                 const Selector &selector) noexcept
  {
    switch (selector.Match())
    {
      case SelectorMatch::Id:
        if (!selector.Value().View().empty())
        {
          collectedHashes.Ids.push_back(IdentifierHash(selector.Value()) * IdSalt);
        }
        break;
      case SelectorMatch::Class:
        if (!selector.Value().View().empty())
        {
          collectedHashes.Classes.push_back(IdentifierHash(selector.Value()) * ClassSalt);
        }
        break;
      case SelectorMatch::Type:
        if (const auto &name = selector.TagLocalNameLower(); name != dom::StarAtom())
        {
          collectedHashes.Tags.push_back(IdentifierHash(name) * TagNameSalt);
        }
        break;
      case SelectorMatch::AttributeEquals:
      case SelectorMatch::AttributeExists:
      case SelectorMatch::AttributeIncludes:
      case SelectorMatch::AttributeDash:
      case SelectorMatch::AttributeSubstring:
      case SelectorMatch::AttributePrefix:
      case SelectorMatch::AttributeSuffix:
        if (const auto &name = selector.Attribute().LocalNameLower(); !IsExcludedAttribute(name))
        {
          collectedHashes.Attributes.push_back(IdentifierHash(name) * AttributeSalt);
        }
        break;
      case SelectorMatch::PseudoClass:
      {
        // With one argument, :is() and :where() require what it requires. With several, the ancestor needs
        // only one argument's identifiers, which a single hash can't say.
        auto pseudoClass = selector.PseudoClass();
        if ((pseudoClass == SelectorPseudoClass::Is || pseudoClass == SelectorPseudoClass::Where)
            && selector.SubSelectors()->SelectorCount() == 1)
        {
          CollectSelectorHashes(collectedHashes, *selector.SubSelectors()->FirstSelectorComponent(),
                                IncludeRightmost::Yes);
        }
        break;
      }
      default: break;
    }
  }

  void SelectorFilter::CollectSelectorHashes(CollectedSelectorHashes &collectedHashes,
                                             const Selector &rightmostSelector,
                                             IncludeRightmost includeRightmost) noexcept
  {
    // Without the rightmost compound (the element itself), collecting starts at the compound before it.
    auto [selector, relation, skipping] = [&]
    {
      if (includeRightmost == IncludeRightmost::No)
      {
        return std::tuple {rightmostSelector.PrecedingComplexSelectorComponent(),
                           rightmostSelector.Relation(), true};
      }
      return std::tuple {&rightmostSelector, SelectorRelation::Compounding, false};
    }();

    for (; selector; selector = selector->PrecedingComplexSelectorComponent())
    {
      // Only compounds reached through descendant and child combinators describe ancestors; one reached
      // through a sibling or shadow combinator describes another element, so it's skipped until the next
      // descendant or child combinator.
      switch (relation)
      {
        case SelectorRelation::Compounding:
          if (!skipping)
          {
            CollectSimpleSelectorHash(collectedHashes, *selector);
          }
          break;
        case SelectorRelation::NextSibling:
        case SelectorRelation::SubsequentSibling:
        case SelectorRelation::ShadowDescendant:
        case SelectorRelation::ShadowPartDescendant:
        case SelectorRelation::ShadowSlotted:        skipping = true; break;
        case SelectorRelation::Descendant:
        case SelectorRelation::Child:
          skipping = false;
          CollectSimpleSelectorHash(collectedHashes, *selector);
          break;
      }
      relation = selector->Relation();
    }
  }

  SelectorFilter::Hashes SelectorFilter::ChooseSelectorHashesForFilter(
    const CollectedSelectorHashes &collectedSelectorHashes) noexcept
  {
    Hashes chosen {};
    size_t count = 0;
    // The rarer a kind of identifier, the more often its absence rejects a selector.
    for (const auto *kind : {&collectedSelectorHashes.Ids, &collectedSelectorHashes.Attributes,
                             &collectedSelectorHashes.Classes, &collectedSelectorHashes.Tags})
    {
      for (auto hash : *kind)
      {
        if (std::ranges::find(chosen.begin(), chosen.begin() + count, hash) != chosen.begin() + count)
        {
          continue;
        }
        chosen[count++] = hash;
        if (count == chosen.size())
        {
          return chosen;
        }
      }
    }
    return chosen;
  }

  SelectorFilter::Hashes SelectorFilter::CollectHashes(const Selector &selector) noexcept
  {
    CollectedSelectorHashes collectedHashes;
    CollectSelectorHashes(collectedHashes, selector, IncludeRightmost::No);
    return ChooseSelectorHashesForFilter(collectedHashes);
  }
}
