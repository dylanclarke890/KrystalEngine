#include "Krystal.Booey/CSS/Selectors/SelectorFilter.hpp"
#include "Krystal.Booey/CSS/Selectors/Selector.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorList.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include <functional>
#include <tuple>

namespace krys::boo::css
{
  namespace
  {
    // Salt to separate otherwise identical string hashes so a class-selector like .article won't match
    // <article> elements.
    enum : uint32
    {
      TagNameSalt = 13,
      IdSalt = 17,
      ClassSalt = 19,
      AttributeSalt = 23
    };

    /// @brief A non-zero hash of @p identifier (zero ends a `SelectorFilter::Hashes`).
    KRYS_NODISCARD uint32 IdentifierHash(const CSSOMStringAtom &identifier) noexcept
    {
      const size_t hash = std::hash<CSSOMStringAtom> {}(identifier);
      const auto result = static_cast<uint32>(hash ^ (hash >> 32));
      return result ? result : 0x80'00'00'00u;
    }

    KRYS_NODISCARD bool IsExcludedAttribute(const CSSOMStringAtom &name) noexcept
    {
      return name.View() == u8"class" || name.View() == u8"id" || name.View() == u8"style";
    }
  }

  void SelectorFilter::CollectElementIdentifierHashes(const dom::Element &element,
                                                      SmallList<uint32, 4> &identifierHashes) noexcept
  {
    identifierHashes.push_back(IdentifierHash(element.TagQualifiedName().LocalNameLower()) * TagNameSalt);

    auto id = element.IdForStyleResolution();
    if (!id.empty())
    {
      identifierHashes.push_back(IdentifierHash(CSSOMStringAtom(krys::move(id))) * IdSalt);
    }

    for (auto &className : element.ClassNames())
    {
      identifierHashes.push_back(IdentifierHash(className) * ClassSalt);
    }

    for (auto &attribute : element.AttributeList())
    {
      auto &attributeName = Is<html::HTMLElement>(element) ? attribute->GetQualifiedName().LocalName()
                                                           : attribute->GetQualifiedName().LocalNameLower();
      if (IsExcludedAttribute(attributeName))
      {
        continue;
      }
      identifierHashes.push_back(IdentifierHash(attributeName) * AttributeSalt);
    }
  }

  bool SelectorFilter::ParentStackIsConsistent(const dom::ContainerNode *parentNode) const noexcept
  {
    if (!parentNode || Is<dom::Document>(*parentNode) || Is<dom::ShadowRoot>(*parentNode))
    {
      return _parentStack.empty();
    }
    return !_parentStack.empty() && _parentStack.back().Element == parentNode;
  }

  void SelectorFilter::InitializeParentStack(const dom::Element &parent) noexcept
  {
    SmallList<const dom::Element *, 20> ancestors;
    for (const auto *ancestor = &parent; ancestor; ancestor = ancestor->ParentElement())
    {
      ancestors.push_back(ancestor);
    }
    _parentStack.reserve(_parentStack.capacity() + ancestors.size());
    for (size_t i = ancestors.size(); i--;)
    {
      PushParent(ancestors[i]);
    }
  }

  void SelectorFilter::PushParent(const dom::Element *parent) noexcept
  {
    krys_debug_assert(_parentStack.empty() || _parentStack.back().Element == parent->ParentElement());
    krys_debug_assert(!_parentStack.empty() || !parent->ParentElement());

    _parentStack.push_back(ParentStackFrame {.Element = parent});
    ParentStackFrame &parentFrame = _parentStack.back();
    // Mix tags, class names and ids into some sort of weird bouillabaisse.
    // The filter is used for fast rejection of child and descendant selectors.
    CollectElementIdentifierHashes(*parent, parentFrame.IdentifierHashes);
    for (auto hash : parentFrame.IdentifierHashes)
    {
      _ancestorIdentifierFilter.Add(hash);
    }
  }

  void SelectorFilter::PushParentInitializingIfNeeded(const dom::Element &parent) noexcept
  {
    if (_parentStack.empty()) KRYS_UNLIKELY
    {
      InitializeParentStack(parent);
      return;
    }
    PushParent(&parent);
  }

  void SelectorFilter::PopParent() noexcept
  {
    krys_debug_assert(!_parentStack.empty());
    const ParentStackFrame &parentFrame = _parentStack.back();
    for (auto hash : parentFrame.IdentifierHashes)
    {
      _ancestorIdentifierFilter.Remove(hash);
    }
    _parentStack.pop_back();
    if (_parentStack.empty())
    {
      krys_debug_assert(_ancestorIdentifierFilter.LikelyEmpty());
      _ancestorIdentifierFilter.Clear();
    }
  }

  void SelectorFilter::PopParentsUntil(const dom::Element *parent) noexcept
  {
    while (!_parentStack.empty())
    {
      if (parent && _parentStack.back().Element == parent)
      {
        return;
      }
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
      {
        auto &tagLowercaseLocalName = selector.TagLocalNameLower();
        if (tagLowercaseLocalName != dom::StarAtom())
        {
          collectedHashes.Tags.push_back(IdentifierHash(tagLowercaseLocalName) * TagNameSalt);
        }
        break;
      }
      case SelectorMatch::AttributeEquals:
      case SelectorMatch::AttributeExists:
      case SelectorMatch::AttributeIncludes:
      case SelectorMatch::AttributeDash:
      case SelectorMatch::AttributeSubstring:
      case SelectorMatch::AttributePrefix:
      case SelectorMatch::AttributeSuffix:
      {
        auto &attributeName = selector.Attribute().LocalNameLower();
        if (!IsExcludedAttribute(attributeName))
        {
          collectedHashes.Attributes.push_back(IdentifierHash(attributeName) * AttributeSalt);
        }
        break;
      }
      case SelectorMatch::PseudoClass:
        switch (selector.PseudoClass())
        {
          case SelectorPseudoClass::Is:
          case SelectorPseudoClass::Where:
            // We can use the filter in the trivial case of single argument :is()/:where().
            // Supporting the multiargument case would require more than one hash.
            if (selector.SubSelectors()->SelectorCount() == 1)
            {
              CollectSelectorHashes(collectedHashes, *selector.SubSelectors()->FirstSelectorComponent(),
                                    IncludeRightmost::Yes);
            }
            break;
          default: break;
        }
        break;
      default: break;
    }
  }

  void SelectorFilter::CollectSelectorHashes(CollectedSelectorHashes &collectedHashes,
                                             const Selector &rightmostSelector,
                                             IncludeRightmost includeRightmost) noexcept
  {
    auto [selector, relation, skipOverSubselectors] = [&]
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
      // Only collect identifiers that match ancestors.
      switch (relation)
      {
        case SelectorRelation::Compounding:
          if (!skipOverSubselectors)
          {
            CollectSimpleSelectorHash(collectedHashes, *selector);
          }
          break;
        case SelectorRelation::NextSibling:
        case SelectorRelation::SubsequentSibling:
        case SelectorRelation::ShadowDescendant:
        case SelectorRelation::ShadowPartDescendant:
        case SelectorRelation::ShadowSlotted:        skipOverSubselectors = true; break;
        case SelectorRelation::Descendant:
        case SelectorRelation::Child:
          skipOverSubselectors = false;
          CollectSimpleSelectorHash(collectedHashes, *selector);
          break;
      }
      relation = selector->Relation();
    }
  }

  SelectorFilter::Hashes SelectorFilter::ChooseSelectorHashesForFilter(
    const CollectedSelectorHashes &collectedSelectorHashes) noexcept
  {
    Hashes resultHashes {};
    uint32 index = 0;
    auto addIfNew = [&](uint32 hash)
    {
      for (uint32 i = 0; i < index; ++i)
      {
        if (resultHashes[i] == hash)
        {
          return;
        }
      }
      resultHashes[index++] = hash;
    };

    auto copyHashes = [&](auto &hashes)
    {
      for (auto hash : hashes)
      {
        addIfNew(hash);
        if (index == resultHashes.size())
        {
          return true;
        }
      }
      return false;
    };

    // There is a limited number of slots. Prefer more specific selector types.
    if (copyHashes(collectedSelectorHashes.Ids))
    {
      return resultHashes;
    }
    if (copyHashes(collectedSelectorHashes.Attributes))
    {
      return resultHashes;
    }
    if (copyHashes(collectedSelectorHashes.Classes))
    {
      return resultHashes;
    }
    if (copyHashes(collectedSelectorHashes.Tags))
    {
      return resultHashes;
    }

    // Null-terminate if not full.
    resultHashes[index] = 0;
    return resultHashes;
  }

  SelectorFilter::Hashes SelectorFilter::CollectHashes(const Selector &selector) noexcept
  {
    CollectedSelectorHashes collectedHashes;
    CollectSelectorHashes(collectedHashes, selector, IncludeRightmost::No);
    return ChooseSelectorHashesForFilter(collectedHashes);
  }
}
